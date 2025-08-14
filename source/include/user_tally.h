#ifndef USER_TALLY_H
#define USER_TALLY_H

#include "tally.h"
#include "ion.h"

class user_tally
{
public:
    enum coordinate_t { Invalid, xyz, cyl, sph };
    struct parameters
    {
        std::string id{ "My tally" };
        Event event{ Event::IonStop };
        std::vector<float> x, y, z, rho, phi, r, theta, vx, vy, vz, vrho, vphi, vr, vtheta, atom_id;
        coordinate_t coordinates = xyz;
        vector3 zaxis{0, 0, 1}; // z-axis direction
        vector3 xzvec{1, 1, 0}; // vector in xz plane
        vector3 org{0.5, 0.5, 0.5}; // system center
    };

    user_tally(const parameters &p) : par_(p) { }
    user_tally(const user_tally &o)
        : par_(o.par_),
          data_(o.data_.copy()),
          bins(o.bins),
          bin_codes(o.bin_codes),
          bin_sizes(o.bin_sizes),
          idx(o.idx),
          t(o.t)
    {
    }

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

    /// @brief Score an event
    /// @param ev the event type
    /// @param i the ion causing the event
    /// @param pv pointer to additional data, if available
    void operator()(Event ev, const ion &i, const void *pv = 0);

private:
    parameters par_;
    ArrayNDd data_;
    bool get_bin(const ion& i); //const;
    enum variable_code {cX, cY, cZ, cRho, cPhi, cR, cTheta, cVX, cVY, cVZ, cVRho, cVPhi, cVR, cVTheta,cAtom_id};
    std::vector<variable_code> bin_codes;
    std::vector<std::vector<float>> bins;
    std::vector<size_t> bin_sizes;
    std::vector<size_t> idx; // will be same size as bin_sizes
    xyz_frame_change t; // trasform to tally reference frame
};

#endif // USER_TALLY_H
