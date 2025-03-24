#ifndef CASCADE_QUEUE_H
#define CASCADE_QUEUE_H

#include <queue>
#include <list>
#include <iostream>
#include <cassert>

#include "geometry.h"
#include "target.h"
#include "ion.h"
#include "tally.h"

class cascade_queue
{

    enum defect_type_t { Vacancy, Interstitial };

    struct defect
    {
        defect_type_t type;
        double t;
        vector3 pos;
        vector3 dir;
        int icell;
        const atom *a;
        size_t pair_id;
    };

    // these are defined only to differentiate the 2 defect types
    struct interstitial : public defect
    {
    };
    struct vacancy : public defect
    {
    };

    // defect time comparison operator
    // returns true if the time of lhs is greater than rhs
    struct defect_time_cmp
    {
        bool operator()(const defect *lhs, const defect *rhs) { return lhs->t > rhs->t; }
    };

    // time ordered defect queue
    typedef std::priority_queue<defect *, std::vector<defect *>, defect_time_cmp> defect_queue_t;

    // type for defect object buffers
    // std::vector selected for optimized allocation & pop_back/push_back at the back
    typedef std::vector<defect *> defect_buff_t;
    defect_buff_t all_buff_, free_buff_;

    // types of defect lists
    // std::list is used for optimized removal of items regardless of their position
    // within the container
    typedef std::list<const interstitial *> interstitial_list_t;
    typedef std::list<const vacancy *> vacancy_list_t;

    // type for the  pair
    // typedef std::pair<const interstitial *, const vacancy *> frenkel_pair_t;
    template <typename iter_t>
    struct anti_defect_t
    {
        iter_t it;
        float distance;
        anti_defect_t(const iter_t &i, float d) : it(i), distance(d) { }
    };

    // Frenkel pair comparison operator
    // returns true if distance of lhs is greater than rhs
    template <typename _T>
    struct anti_defect_distance_cmp
    {
        bool operator()(const _T &lhs, _T &rhs) { return lhs.distance > rhs.distance; }
    };

    // type for a container of recombined frenkel pairs
    typedef std::vector<std::pair<const interstitial *, const vacancy *>> pair_buff_t;

    defect *new_defect(defect_type_t atype, const double &at, const vector3 &p, const vector3 &n,
                       int ic, const atom *a, size_t pid)
    {
        defect *d;
        if (free_buff_.empty()) {
            d = new defect;
            all_buff_.push_back(d);
        } else {
            d = free_buff_.back();
            free_buff_.pop_back();
        }
        d->type = atype;
        d->t = at;
        d->pos = p;
        d->dir = n;
        d->icell = ic;
        d->a = a;
        d->pair_id = pid;
        return d;
    }

    defect_queue_t defect_queue;
    vacancy_list_t v_;
    interstitial_list_t i_;
    pair_buff_t riv_;
    size_t nv{ 0 }, ni{ 0 };
    ion pka;

    bool allow_correlated_recombination_;

    bool can_recombine(const defect *d1, const defect *d2)
    {
        return (d1->a == d2->a) && // same atom type
                ((!allow_correlated_recombination_ && (d1->pair_id != d2->pair_id))
                 || allow_correlated_recombination_);
    }

    bool recombine(const vacancy *d1, const grid3D &g)
    {
        float rc = d1->a->Rc(); // recombination radius

        // distance ordered vacancy queue
        typedef anti_defect_t<interstitial_list_t::const_iterator> _T;
        std::priority_queue<_T, std::vector<_T>, anti_defect_distance_cmp<_T>> fpq;

        // ref to anti-defects buffer
        auto &anti_defects = i_;

        // go through all vacancies
        auto i = anti_defects.begin();
        for (; i != anti_defects.end(); ++i) {
            auto d2 = *i;
            if (can_recombine(d1, d2)) {
                float d = g.distance(d1->pos, d2->pos);
                if (d < rc)
                    fpq.push(_T(i, d));
            }
        }

        if (fpq.empty()) {
            // defect does not recombine, put it on the list
            v_.push_back(d1);
            return false;
        } else {
            // defect recombines with the closest anti-defect
            auto &fp = fpq.top();
            riv_.emplace_back(*(fp.it), d1);
            anti_defects.erase(fp.it);
            return true;
        }
    }

    bool recombine(const interstitial *d1, const grid3D &g)
    {
        float rc = d1->a->Rc(); // recombination radius

        // distance ordered vacancy queue
        typedef anti_defect_t<vacancy_list_t::const_iterator> _T;
        std::priority_queue<_T, std::vector<_T>, anti_defect_distance_cmp<_T>> fpq;

        // ref to anti-defects buffer
        auto &anti_defects = v_;

        // go through all vacancies
        auto i = anti_defects.begin();
        for (; i != anti_defects.end(); ++i) {
            auto d2 = *i;
            if (can_recombine(d1, d2)) {
                float d = g.distance(d1->pos, d2->pos);
                if (d < rc)
                    fpq.push(_T(i, d));
            }
        }

        if (fpq.empty()) {
            // defect does not recombine, put it on the list
            i_.push_back(d1);
            return false;
        } else {
            // defect recombines with the closest anti-defect
            auto &fp = fpq.top();
            riv_.emplace_back(d1, *(fp.it));
            anti_defects.erase(fp.it);
            return true;
        }
    }

    static void print(std::ostream &os, const defect &d)
    {
        os << d.t << '\t' << d.a->id() << '\t' << ((d.type == Interstitial) ? '1' : '0') << '\t'
           << d.pos.x() << '\t' << d.pos.y() << '\t' << d.pos.z() << '\t' << d.icell << '\t'
           << d.pair_id << std::endl;
    }

public:
    explicit cascade_queue(bool allow_correlated_recombination)
        : allow_correlated_recombination_(allow_correlated_recombination)
    {
    }
    ~cascade_queue()
    {
        for (defect *d : all_buff_)
            delete d;
    }

    void push_i(const ion *i)
    {

        defect_queue.push(new_defect(Interstitial, i->t(), i->pos(), i->dir(), i->cellid(),
                                     i->myAtom(), i->uid()));
        ni++;
    }
    void push_v(const ion *i)
    {

        defect_queue.push(new_defect(Vacancy, i->t(), i->pos(), i->dir(), i->cellid(), i->myAtom(),
                                     i->uid()));
        nv++;
    }

    // #define CASCADE_DEBUG_PRINT

    void intra_cascade_recombination(const grid3D &g)
    {
        // if (defect_queue.size()<=2) return;

#ifdef CASCADE_DEBUG_PRINT

        bool dbg = defect_queue.size() > 10;
        if (dbg) {
            std::cout << "PKA E=" << pka.erg() << std::endl;
            std::cout << "R=" << pka.pos() << std::endl;
            std::cout << "defect_queue (N=" << defect_queue.size() << "):" << std::endl;
        }

#endif

        for (; !defect_queue.empty(); defect_queue.pop()) {
            defect *d = defect_queue.top();

#ifdef CASCADE_DEBUG_PRINT
            if (dbg)
                print(std::cout, *d);
#endif

            if (d->type == Interstitial)
                recombine((interstitial *)d, g);
            else
                recombine((vacancy *)d, g);
        }

#ifdef CASCADE_DEBUG_PRINT
        if (dbg) {
            std::cout << "END defect_queue\n\n";
            std::cout << "Recombinations (" << riv_.size() << "):\n";
            for (auto &p : riv_) {
                print(std::cout, *p.first);
                print(std::cout, *p.second);
            }
        }
#endif
    }

    // update tally scores due to I-V recombinations
    void tally_update(tally &t)
    {
        // get relevant tally arrays
        auto &AI = t.at(tally::cI);
        auto &AV = t.at(tally::cV);
        auto &As = t.at(tally::eStored);
        auto &Al = t.at(tally::eLattice);
        auto &Ariv = t.at(tally::cRecombinations);
        // update tally
        for (auto &fp : riv_) {
            // interstitial
            auto &i = fp.first;
            int iid = i->a->id();
            float halfEl = i->a->El() / 2;
            int cid = i->icell;
            AI(iid, cid)--;
            As(iid, cid) -= halfEl;
            Al(iid, cid) += halfEl;
            Ariv(iid, cid)++;
            // vacancy
            auto &v = fp.second;
            cid = v->icell;
            AV(iid, cid)--;
            As(iid, cid) -= halfEl;
            Al(iid, cid) += halfEl;
        }
    }

    void clear()
    {
        // clear all buffers
        if (!defect_queue.empty()) {
            defect_queue_t empty;
            std::swap(defect_queue, empty);
        }
        i_.clear();
        v_.clear();
        riv_.clear();
        ni = nv = 0;
        free_buff_.clear();

        // if there are allocated defect objects copy them to the "free" buffer
        if (!all_buff_.empty()) {
            free_buff_.resize(all_buff_.size());
            std::copy(all_buff_.begin(), all_buff_.end(), free_buff_.begin());
        }
    }

    void init(const ion *i)
    {
        clear();
        pka = *i;
        defect_queue.push(new_defect(Vacancy, i->t(), i->pos0(), i->dir(), i->cellid0(),
                                     i->myAtom(), i->uid()));
        nv++;
    }

    // count I-V recombinations & correlated recombinations
    // s, sc are corresponding buffers to hold counts per atomic id
    void count_riv(float *s, float *sc) const
    {
        for (auto &p : riv_) {
            auto &d1 = p.first; // interstitial
            auto &d2 = p.second; // vacancy
            int i = d1->a->id() - 1; // atom id
            s[i]++;
            if (d1->pair_id == d2->pair_id)
                sc[i]++;
        }
    }
};

#endif // CASCADE_QUEUE_H
