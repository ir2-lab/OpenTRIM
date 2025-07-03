#ifndef USER_TALLY_H
#define USER_TALLY_H

#include "tally.h"
#include "ion.h"

class user_tally
{
public:
    static constexpr int nbins = 8;
    static constexpr double xmin = 0.0;
    static constexpr double xmax = 600.0;
    std::vector<double> bins; // count ions per bin


    struct parameters
    {
        std::string id{ "My tally" };
        Event event{ Event::IonStop };
    };

    user_tally(const parameters &p) : par_(p) { }
    user_tally(const user_tally &o) : par_(o.par_), data_(o.data_.copy()), bins(o.bins) { }

    const ArrayNDd &data() const { return data_; }

    /// @brief Initialize tally buffers for given # of atoms and cells
    void init();

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
    void addSquared(const user_tally &t) { data_.addSquared(t.data_); }

    /// @brief Copy contents from another tally
    /// @param t another tally object
    void copy(const user_tally &t) { data_ = t.data_.copy(); }

    /// @brief Copy contents to another tally
    /// @param t another tally object
    void copyTo(user_tally &t) const { data_.copyTo(t.data_); }

    /// @brief Copy contents to another tally
    /// @param t another tally object
    user_tally clone() const
    {
        user_tally t(*this);
        t.data_ = data_.copy();
        return t;
    }

    void print() const;

    /// @brief Score an event
    /// @param ev the event type
    /// @param i the ion causing the event
    /// @param pv pointer to additional data, if available
    void operator()(Event ev, const ion &i, const void *pv = 0);

private:
    parameters par_;
    ArrayNDd data_;
    int get_bin(double x) const; // helper
};

#endif // USER_TALLY_H
