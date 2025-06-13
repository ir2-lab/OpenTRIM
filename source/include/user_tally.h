#ifndef USER_TALLY_H
#define USER_TALLY_H

#include "tally.h"

class user_tally
{
    ArrayNDd data_;
public:
    user_tally();

    const ArrayNDd& data() const { return data_; }

    /// @brief Initialize tally buffers for given # of atoms and cells
    void init()
    {
    }

    /// @brief Zero-out all tally scores
    void clear()
    {
        data_.clear();
    }

    /// @brief Add the scores from another tally
    /// @param t another tally object
    /// @return this object
    user_tally &operator+=(const user_tally &t)
    {
        data_ += t.data_;
        return *this;
    }

    /// @brief Add the squared scores from another tally, i.e., x[i] += x'[i]*x'[i]
    /// @param t another tally object
    void addSquared(const user_tally &t)
    {
        data_.addSquared(t.data_);
    }

    /// @brief Copy contents from another tally
    /// @param t another tally object
    void copy(const tally &t)
    {
        data_ = t.data_.copy();
    }

    /// @brief Copy contents to another tally
    /// @param t another tally object
    void copyTo(tally &t) const
    {
        data_.copyTo(t.data_);
    }

    /// @brief Copy contents to another tally
    /// @param t another tally object
    user_tally clone() const
    {
        tally t;
        t.data_ = data_.copy();
        return t;
    }

    /// @brief Score an event
    /// @param ev the event type
    /// @param i the ion causing the event
    /// @param pv pointer to additional data, if available
    void operator()(Event ev, const ion &i, const void *pv = 0);

};

#endif // USER_TALLY_H
