#ifndef USER_TALLY_H
#define USER_TALLY_H

#include "tally.h"
#include "ion.h"

class user_tally
{
public:
    struct parameters
    {
        std::string id{ "MyTally" };
        std::string description;
        Event event{ Event::IonStop };
        coord_sys coordinate_system;
        /* clang-format off */
        std::vector<float> x, y, z,
                           r, rho, cosTheta,
                           nx, ny, nz,
                           E, Tdam, V,
                           atom_id, recoil_id;
        /* clang-format on */
    };

    user_tally(const parameters &p) : par_(p) { }

    user_tally(const user_tally &o)
        : par_(o.par_),
          data_(o.data_.copy()),
          bin_codes(o.bin_codes),
          bins(o.bins),
          bin_sizes(o.bin_sizes),
          idx(o.idx),
          vac_id(o.vac_id)
    {
    }

    /// @brief Return the id of the user tally
    const std::string &id() const { return par_.id; }

    /// @brief Return the description for the user tally
    const std::string &description() const { return par_.description; }

    /// @brief Return a constant reference to the tally data
    const ArrayNDd &data() const { return data_; }

    /// @brief Return a reference to the tally data
    ArrayNDd &data() { return data_; }

    /// @brief Return coordinate system values
    const coord_sys &cs() const { return par_.coordinate_system; }

    /// @brief Return Event
    const Event &event() const { return par_.event; }

    /// @brief Initialize tally buffers for given # of atoms and cells
    void init(size_t natoms);

    /// @brief Zero-out all tally scores
    void clear() { data_.clear(); }

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

    /// @brief Score an event
    /// @param ev the event type
    /// @param i the ion causing the event
    /// @param pv pointer to additional data, if available
    void operator()(Event ev, const ion &i, const void *pv = 0)
    {
        if (ev == par_.event && get_bin(i, pv))
            data_(idx)++;
    }

    const char *bin_name(int i) const;
    const char *bin_desc(int i) const;
    std::vector<std::string> bin_names() const;
    std::vector<std::string> bin_descriptions() const;

    /// @brief Return the vector of bin edges for the given bin index
    /// @param bin_id zero-based bin index
    /// @return
    const std::vector<float> &bin_edges(int bin_id) const
    {
        assert(bin_id >= 0 && bin_id < (int)bins.size());
        return bins[bin_id];
    }

private:
    parameters par_;
    ArrayNDd data_;

    /* clang-format off */
    enum variable_code {
        cX, cY, cZ,
        cR, cRho, cCosTheta,
        cNx, cNy, cNz,
        cE, cTdam, cV,
        cAtom_id, cRecoil_id,
        cNumCodes
    };
    /* clang-format on */

    // codes of binning variables defined by user
    std::vector<variable_code> bin_codes;
    // array of bin edges for each variable
    std::vector<std::vector<float>> bins;
    // # of bins for each binning variable
    std::vector<size_t> bin_sizes;
    // bin indexes for current event
    std::vector<size_t> idx; // will be same size as bin_sizes
    //
    std::vector<size_t> vac_id; // will be same size as bin_sizes

    bool get_bin(const ion &i, const void *pv = 0); // const;
    bool push_bins(variable_code c, const std::vector<float> &edges, size_t natoms = 0);
};

#endif // USER_TALLY_H
