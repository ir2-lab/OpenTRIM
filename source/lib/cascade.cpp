#include "cascade.h"

inline std::ostream &operator<<(std::ostream &os, const ion *d)
{
    os << d->t() << '\t' << d->myAtom()->id() << '\t'
       << ((d->type() == ion::interstitial) ? 'I' : 'V') << '\t' << d->pos().x() << '\t'
       << d->pos().y() << '\t' << d->pos().z() << '\t' << d->cellid() << '\t' << d->uid();
    return os;
}

void abstract_cascade::count_riv(float *s, float *sc) const
{
    for (auto &p : riv_) {
        auto &d1 = p.first; // interstitial
        auto &d2 = p.second; // vacancy
        int i = d1->myAtom()->id() - 1; // atom id
        s[i]++;
        if (d1->uid() == d2->uid())
            sc[i]++;
    }
}

void abstract_cascade::clear(ion_queue &q)
{
    // for (auto &d : i_)
    //     q.free_ion(d);
    // i_.clear();

    // for (auto &d : v_)
    //     q.free_ion(d);
    // v_.clear();

    for (auto i = riv_.begin(); i != riv_.end(); ++i) {
        q.free_ion(i->first);
        q.free_ion(i->second);
    }
    riv_.clear();
}

bool abstract_cascade::recombine_vacancy(ion *d1)
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

bool abstract_cascade::recombine_interstitial(ion *d1)
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

abstract_cascade::defect_list_t::iterator abstract_cascade::find_rc_partner(const ion *d1,
                                                                            defect_list_t &adcont)
{
    float rc = d1->myAtom()->Rc(); // recombination radius

    // define a distance-ordered defect queue
    typedef std::pair<defect_list_t::iterator, float> _T;
    std::priority_queue<_T, std::vector<_T>, defect_distance_cmp<_T>> fpq;

    // go through all vacancies
    auto i = adcont.begin();
    for (; i != adcont.end(); ++i) {
        auto d2 = *i;
        if (d1->myAtom() == d2->myAtom()) {
            float d = g.distance(d1->pos(), d2->pos());
            if (d < rc)
                fpq.push(_T(i, d));
        }
    }

    return fpq.empty() ? adcont.end() : fpq.top().first;
}

// #define CASCADE_DEBUG_PRINT

void time_ordered_cascade::intra_cascade_recombination(ion_queue &q)
{
    assert(i_.empty());
    assert(v_.empty());
    assert(riv_.empty());

#ifdef CASCADE_DEBUG_PRINT

    bool dbg = defect_queue.size() > 10;
    if (dbg) {
        std::cout << "PKA E=" << pka.erg() << std::endl;
        std::cout << "R=" << pka.pos() << std::endl;
        std::cout << "defect_queue (N=" << defect_queue.size() << "):" << std::endl;
    }

#endif

    // get all cascade defects into a mixed I-V time-ordered queue
    defect_queue_t defect_queue;
    while (ion *d = q.pop_vacancy())
        defect_queue.push(d);
    while (ion *d = q.pop_interstitial())
        defect_queue.push(d);

    // process defects in time-ordered way
    for (; !defect_queue.empty(); defect_queue.pop()) {
        ion *d = defect_queue.top();

#ifdef CASCADE_DEBUG_PRINT
        if (dbg)
            print(std::cout, *d);
#endif

        if (d->type() == ion::interstitial)
            recombine_interstitial(d);
        else
            recombine_vacancy(d);
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

    // return all non-recombining Is and Vs back to the ion queue
    for (auto &d : i_)
        q.push_interstitial(d);
    i_.clear();
    for (auto &d : v_)
        q.push_vacancy(d);
    v_.clear();
}

void unordered_cascade::intra_cascade_recombination(ion_queue &q)
{
    assert(i_.empty());
    assert(v_.empty());
    assert(riv_.empty());

    // get all cascade interstitials
    while (ion *d = q.pop_interstitial())
        i_.push_back(d);

    // loop over vacancies and find recombination pairs
    while (ion *d = q.pop_vacancy())
        recombine_vacancy(d);

    // return all non-recombining Is and Vs back to the ion queue
    for (auto &d : i_)
        q.push_interstitial(d);
    i_.clear();
    for (auto &d : v_)
        q.push_vacancy(d);
    v_.clear();
}

// update tally scores due to I-V recombinations
// void tally_update(tally &t)
// {

//     // get relevant tally arrays
//     auto &AI = t.at(tally::cI);
//     auto &AV = t.at(tally::cV);
//     auto &As = t.at(tally::eStored);
//     auto &Al = t.at(tally::eLattice);
//     auto &Ariv = t.at(tally::cRecombinations);
//     size_t ncells = AI.size() / AI.dim()[0];

//            // update tally
//     for (auto &fp : riv_) {
//         // interstitial
//         auto &i = fp.first;
//         int iid = i->myAtom()->id();
//         float halfEl = i->myAtom()->El() / 2;
//         int cid = i->cellid();
//         size_t k = iid * ncells + cid;
//         AI(k)--;
//         As(k) -= halfEl;
//         Al(k) += halfEl;
//         Ariv(k)++;
//         // vacancy
//         auto &v = fp.second;
//         cid = v->cellid();
//         k = iid * ncells + cid;
//         AV(k)--;
//         As(k) -= halfEl;
//         Al(k) += halfEl;
//     }
// }

//        // send remaining defects to stream
// void stream_update(event_stream &t) const
// {
//     damage_event ev;

//     for (const auto &d : i_) {
//         ev.interstitial(*d);
//         t.write(&ev);
//     }
//     for (const auto &d : v_) {
//         ev.vacancy(*d);
//         t.write(&ev);
//     }
// }
