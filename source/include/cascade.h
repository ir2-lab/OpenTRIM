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

// base class of all cascade objects
// defines public interface of cascade objects
class abstract_cascade
{
protected:
public:
    explicit abstract_cascade(const grid3D &ag) : g(ag) { }
    virtual ~abstract_cascade() { }

    virtual void intra_cascade_recombination(ion_queue &q) = 0;

    // count I-V recombinations & correlated recombinations
    // s, sc are corresponding buffers to hold counts per atomic id
    void count_riv(float *s, float *sc) const;

    // return all I-V ion buffers to the ion queue
    void clear(ion_queue &q);

protected:
    // ref to the grid, for calculating defect distance
    const grid3D &g;

    // frenkel pair type
    // first: interstitial
    // second: vacancy
    typedef std::pair<ion *, ion *> frenkel_pair_t;

    // container of frenkel pairs
    typedef std::vector<frenkel_pair_t> pair_buff_t;

    // defect container
    // std::list is used for optimized removal of items regardless of their position
    // within the container
    typedef std::list<ion *> defect_list_t;

    defect_list_t v_; // not recombined vacancies
    defect_list_t i_; // not recombined interstitials
    pair_buff_t riv_; // recombined pairs

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
     * d1: a defect (vacancy or interstitial)
     * adcont: anti-defect container
     *
     * returns (an iterator to) the closest anti-defect that can recombine with d1
     *
     */
    defect_list_t::iterator find_rc_partner(const ion *d1, defect_list_t &adcont);

    // recombine a vacancy with the closest vacancy
    bool recombine_vacancy(ion *d1);

    // recombine an interstitial with the closest vacancy
    bool recombine_interstitial(ion *d1);
};

/**
 * @brief Order defects according to creation time and then recombine
 */
class time_ordered_cascade : public abstract_cascade
{
public:
    explicit time_ordered_cascade(const grid3D &g) : abstract_cascade(g) { }

    void intra_cascade_recombination(ion_queue &q) override;

protected:
    // defect time comparison operator
    // returns true if the time of lhs is greater than rhs
    struct defect_time_cmp
    {
        bool operator()(const ion *lhs, const ion *rhs) { return lhs->t() > rhs->t(); }
    };

    // time ordered defect queue
    typedef std::priority_queue<ion *, std::vector<ion *>, defect_time_cmp> defect_queue_t;
};

/**
 * @brief recombine defects without caring for time of creation
 */
class unordered_cascade : public abstract_cascade
{
public:
    explicit unordered_cascade(const grid3D &g) : abstract_cascade(g) { }

    void intra_cascade_recombination(ion_queue &q) override;
};

#endif // CASCADE_QUEUE_H
