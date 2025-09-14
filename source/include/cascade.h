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
#include "event_stream.h"

// object describing a vacancy or interstitial
struct defect
{
    damage_event::defect_type_t type;
    size_t hid; // history id
    int rid; // recoil id
    int cid; // cell id
    double t; // time
    vector3 pos; // position
    vector3 dir; // direction
    const atom *a; // atom species
    size_t pair_id; // unique id

    // copy data from an ion object
    void from_ion(damage_event::defect_type_t atype, const ion &i, bool is_init = false)
    {
        type = atype;
        t = i.t();
        pos = is_init ? i.pos0() : i.pos();
        cid = is_init ? i.cellid0() : i.cellid();
        dir = i.dir();
        a = i.myAtom();
        pair_id = i.uid();
        hid = i.ion_id();
        rid = i.recoil_id();
    }
};

inline std::ostream &operator<<(std::ostream &os, const defect &d)
{
    os << d.t << '\t' << d.a->id() << '\t' << ((d.type == damage_event::Interstitial) ? 'I' : 'V')
       << '\t' << d.pos.x() << '\t' << d.pos.y() << '\t' << d.pos.z() << '\t' << d.cid << '\t'
       << d.pair_id;
    return os;
}

// these are defined only to differentiate the 2 defect types
struct interstitial : public defect
{
};
struct vacancy : public defect
{
};

// frenkel pair
typedef std::pair<const interstitial *, const vacancy *> frenkel_pair_t;

// container of frenkel pairs
typedef std::vector<frenkel_pair_t> pair_buff_t;

// types of defect containers
// std::list is used for optimized removal of items regardless of their position
// within the container
typedef std::list<const interstitial *> interstitial_list_t;
typedef std::list<const vacancy *> vacancy_list_t;

/*
 * Class to handle allocated defect objects
 *
 * Defect structs are allocated as the cascade expands
 *
 * When the cascade finishes, call reset().
 * Then the allocated are objects are available for
 * the next cascade.
 *
 */
class defect_buffer
{
    // type for defect object buffers
    // std::vector selected for optimized allocation & pop_back/push_back at the back
    typedef std::vector<defect *> defect_buff_t;
    defect_buff_t all_; // all allocated defect objects
    defect_buff_t free_; // defects available for use

public:
    defect_buffer() = default;
    ~defect_buffer()
    {
        // delete all
        for (defect *d : all_)
            delete d;
    }

    // return a new defect object
    // either allocate it or return a free one
    defect *new_defect()
    {
        defect *d;
        if (free_.empty()) {
            d = new defect;
            all_.push_back(d);
        } else {
            d = free_.back();
            free_.pop_back();
        }
        return d;
    }

    // make all allocated objects free to use
    void reset()
    {
        free_.clear();

        // if there are allocated defect objects copy them to the "free" buffer
        if (!all_.empty()) {
            free_.resize(all_.size());
            std::copy(all_.begin(), all_.end(), free_.begin());
        }
    }
};

// base class of all cascade objects
// defines public interface of cascade objects
class abstract_cascade
{
protected:
    defect_buffer buff_; // allocated defect objects
    vacancy_list_t v_; // not recombined vacancies
    interstitial_list_t i_; // not recombined interstitials
    pair_buff_t riv_; // recombined pairs
    size_t nv{ 0 }, ni{ 0 }; // # of v & i
    const grid3D &g; // ref to the grid, for calculating defect distance

public:
    explicit abstract_cascade(const grid3D &ag) : g(ag) { }
    virtual ~abstract_cascade() { }

    virtual void init(const ion &i) = 0;
    virtual void push_interstitial(const ion &i) = 0;
    virtual void push_vacancy(const ion &i) = 0;
    virtual void intra_cascade_recombination() = 0;

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
            int cid = i->cid;
            AI(iid, cid)--;
            As(iid, cid) -= halfEl;
            Al(iid, cid) += halfEl;
            Ariv(iid, cid)++;
            // vacancy
            auto &v = fp.second;
            cid = v->cid;
            AV(iid, cid)--;
            As(iid, cid) -= halfEl;
            Al(iid, cid) += halfEl;
        }
    }

    // send remaining defects to stream
    void stream_update(event_stream &t) const
    {
        damage_event ev;

        for (const auto &d : i_) {
            ev.interstitial(d->hid, d->rid, d->a->id(), d->cid, d->pos);
            t.write(&ev);
        }
        for (const auto &d : v_) {
            ev.vacancy(d->hid, d->rid, d->a->id(), d->cid, d->pos);
            t.write(&ev);
        }
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

    virtual void clear()
    {
        // clear all buffers
        i_.clear();
        v_.clear();
        riv_.clear();
        ni = nv = 0;
        buff_.reset();
    }

protected:
    defect *new_defect(damage_event::defect_type_t atype, const ion &i, bool is_init = false)
    {
        defect *d = buff_.new_defect();
        d->from_ion(atype, i, is_init);
        return d;
    }

    // Comparator for pair<iterator, distance> objects
    // returns true if distance of lhs is greater than rhs
    template <typename _T>
    struct defect_distance_cmp
    {
        bool operator()(const _T &lhs, _T &rhs) { return lhs.second > rhs.second; }
    };

    /*
     * find recombination partner
     *
     * _D: a defect class (vacancy or interstitial)
     * _ADcont: container of anti-defects
     *
     * returns (an iterator to) the closest anti-defect that can recombine with d1
     *
     */
    template <class _D, class _ADcont>
    typename _ADcont::iterator find_rc_partner(const _D *d1, _ADcont &adcont)
    {
        float rc = d1->a->Rc(); // recombination radius

        // distance ordered vacancy queue
        typedef std::pair<typename _ADcont::iterator, float> _T;
        std::priority_queue<_T, std::vector<_T>, defect_distance_cmp<_T>> fpq;

        // go through all vacancies
        auto i = adcont.begin();
        for (; i != adcont.end(); ++i) {
            auto d2 = *i;
            if (d1->a == d2->a) {
                float d = g.distance(d1->pos, d2->pos);
                if (d < rc)
                    fpq.push(_T(i, d));
            }
        }

        return fpq.empty() ? adcont.end() : fpq.top().first;
    }
};

/**
 * @brief Order defects according to creation time and then recombine
 */
class time_ordered_cascade : public abstract_cascade
{
protected:
    // defect time comparison operator
    // returns true if the time of lhs is greater than rhs
    struct defect_time_cmp
    {
        bool operator()(const defect *lhs, const defect *rhs) { return lhs->t > rhs->t; }
    };

    // time ordered defect queue
    typedef std::priority_queue<defect *, std::vector<defect *>, defect_time_cmp> defect_queue_t;

    defect_queue_t defect_queue;

    ion pka;

    bool recombine(const vacancy *d1)
    {
        auto it = find_rc_partner(d1, i_);

        if (it == i_.end()) {
            // defect does not recombine, put it on the list
            v_.push_back(d1);
            return false;
        } else {
            // defect recombines with the closest anti-defect
            riv_.emplace_back(*it, d1);
            i_.erase(it);
            return true;
        }
    }

    bool recombine(const interstitial *d1)
    {
        auto it = find_rc_partner(d1, v_);

        if (it == v_.end()) {
            // defect does not recombine, put it on the list
            i_.push_back(d1);
            return false;
        } else {
            // defect recombines with the closest anti-defect
            riv_.emplace_back(d1, *it);
            v_.erase(it);
            return true;
        }
    }

public:
    explicit time_ordered_cascade(const grid3D &g) : abstract_cascade(g) { }

    void push_interstitial(const ion &i) override
    {
        defect_queue.push(new_defect(damage_event::Interstitial, i));
        ni++;
    }
    void push_vacancy(const ion &i) override
    {
        defect_queue.push(new_defect(damage_event::Vacancy, i));
        nv++;
    }

    // #define CASCADE_DEBUG_PRINT

    void intra_cascade_recombination() override
    {

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

            if (d->type == damage_event::Interstitial)
                recombine((interstitial *)d);
            else
                recombine((vacancy *)d);
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

    virtual void clear() override
    {
        // clear all buffers
        if (!defect_queue.empty()) {
            defect_queue_t empty;
            std::swap(defect_queue, empty);
        }
        abstract_cascade::clear();
    }

    void init(const ion &i) override
    {
        clear();
        pka = i;
        defect_queue.push(new_defect(damage_event::Vacancy, i, true));
        nv++;
    }
};

/**
 * @brief recombine defects without caring for time of creation
 */
class unordered_cascade : public abstract_cascade
{
protected:
    vacancy_list_t v0_; // initial vacancies
    interstitial_list_t i0_; // initial interstitials

    bool recombine(const vacancy *d1)
    {
        auto it = find_rc_partner(d1, i_);

        if (it == i_.end()) {
            // defect does not recombine, put it on the list
            v_.push_back(d1);
            return false;
        } else {
            // defect recombines with the closest anti-defect
            riv_.emplace_back(*it, d1);
            i_.erase(it);
            return true;
        }
    }

    bool recombine(const interstitial *d1)
    {
        auto it = find_rc_partner(d1, v_);

        if (it == v_.end()) {
            // defect does not recombine, put it on the list
            i_.push_back(d1);
            return false;
        } else {
            // defect recombines with the closest anti-defect
            riv_.emplace_back(d1, *it);
            v_.erase(it);
            return true;
        }
    }

public:
    explicit unordered_cascade(const grid3D &g) : abstract_cascade(g) { }

    void push_interstitial(const ion &i) override
    {
        i0_.push_back((interstitial *)new_defect(damage_event::Interstitial, i));
        ni++;
    }
    void push_vacancy(const ion &i) override
    {
        v0_.push_back((vacancy *)new_defect(damage_event::Vacancy, i));
        nv++;
    }

    void init(const ion &i) override
    {
        clear();
        v0_.push_back((vacancy *)new_defect(damage_event::Vacancy, i, true));
        nv++;
    }

    void intra_cascade_recombination() override
    {
        // loop over vacancies and find recombination pairs
        for (; !v0_.empty(); v0_.pop_back()) {
            const vacancy *v = v0_.back();

            auto it = find_rc_partner(v, i0_);

            if (it == i0_.end()) {
                // defect does not recombine, put it on the list
                v_.push_back(v);
            } else {
                // defect recombines with the closest anti-defect
                riv_.emplace_back(*it, v);
                i0_.erase(it);
            }
        }

        // put the remaining interstitials in the buffer
        i_.splice(i_.begin(), i0_);
    }
};

#endif // CASCADE_QUEUE_H
