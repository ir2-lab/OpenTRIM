#ifndef CORTEO_XS_H
#define CORTEO_XS_H

#include <screened_coulomb.h>
#include <ieee754_seq.h>

#include <Eigen/Dense>

/**
 * \defgroup XS Nuclear scattering
 *
 * \brief Scattering of ions by target atoms described by the screened Coulomb interaction.
 *
 * The scattering is considered elastic and it is approximated by classical kinematics.
 *
 * For fast calculation of scattering in the Monte-Carlo simulation, we employ interpolation
 * on 2-d tables of scattering quantities as a function of reduced
 * center-of-mass energy and impact parameter \f$ (\epsilon_i, s_j ) \f$.
 *
 * Both \f$ \epsilon_i \f$ and \f$ s_j  \f$ are quasi log-spaced sequences based on the IEEE754
 * floating point number representation as employed in the program Corteo (F. Schiettekatte,
 * http://www.lps.umontreal.ca/~schiette/index.php?n=Recherche.Corteo).
 *
 * They 2-d grid is defined in the class `xs_tbl2d_iterator`.
 *
 * The class \ref corteo_xs_lab can be used
 * for scattering calculations on a specific projectile/target combination. It stores internally
 * 2-dimensional scattering tables and uses interpolation to calculate the scattering for
 * given energy and impact parameter.
 *
 */

/**
 * @brief The xs_tbl2d_iterator struct facilitates access to tabulated scattering data
 *
 * Quantities are tabulated as a function of reduced energy and impact parameter.
 *
 * This structure defines two 4-bit corteo indexes:
 *   - e_index for the reduced energy with [21-(-19)]*2^4 + 1=641 points (rows) from \f$ 2^{-19} \f$
 * to \f$ 2^{21} \f$
 *   - s_index for the reduced impact parameter with [6-(-26)]*2^4 + 1=513 points (columns) from \f$
 * 2^{-26} \f$ to \f$ 2^{6} \f$
 *
 * Required memory for the tabulated floating point data: 1.25 MB
 *
 * @ingroup XS
 */
struct xs_tbl2d_grid
{
    /// Reduced energy range
    typedef ieee754_seq<float, 4, -19, 21> e_range_t;
    typedef typename e_range_t::iterator e_iterator_t;
    /// Reduced impact parameter range
    typedef ieee754_seq<float, 4, -26, 6> s_range_t;
    typedef typename s_range_t::iterator s_iterator_t;
    /// number of rows (energy values)
    constexpr static const int rows = e_range_t::count;
    /// number of columns (impact parameter values)
    constexpr static const int cols = s_range_t::count;
    /// total number of elements
    constexpr static const int size = rows * cols;

    struct iterator
    {

        iterator(int ie = 0, int is = 0) : ie_(ie), is_(is) { }
        iterator(float e, float s) : ie_(e), is_(s) { }

        e_iterator_t &e_iterator() { return ie_; }
        s_iterator_t &s_iterator() { return is_; }
        const e_iterator_t &e_iterator() const { return ie_; }
        const s_iterator_t &s_iterator() const { return is_; }

        float e() const { return ie_.value(); }
        float s() const { return is_.value(); }

        /**
         * @brief Advance the iterator by one
         * @return A reference to the iterator
         */
        iterator &operator++()
        {
            if (is_ != s_range_t::count)
                is_++;
            if (is_ == s_range_t::count) {
                if (ie_ != e_range_t::count)
                    ie_++;
                if (ie_ != e_range_t::count)
                    is_ = 0;
            }
            return *this;
        }
        iterator operator++(int)
        {
            iterator retval = *this;
            ++(*this);
            return retval;
        }

        /**
         * @brief Reduce the iterator by one
         * @return A reference to the iterator
         */
        iterator &operator--()
        {
            if (is_ != -1)
                is_--;
            if (is_ == -1) {
                if (ie_ != -1)
                    ie_--;
                if (ie_ != -1)
                    is_ = s_range_t::count - 1;
            }
            return *this;
        }
        iterator operator--(int)
        {
            iterator retval = *this;
            --(*this);
            return retval;
        }

        /**
         * @brief Returns an iterator to the 1st point of the range
         */
        iterator begin() const { return iterator(); }

        /**
         * @brief Returns an iterator to one past the last point of the range
         */
        iterator end() const { return iterator(e_range_t::count, s_range_t::count); }

        /**
         * @brief Returns an iterator to the last point of the range
         */
        iterator rbegin() const { return iterator(e_range_t::count - 1, s_range_t::count - 1); }

        /**
         * @brief Returns an iterator to one before the first point
         */
        iterator rend() const { return iterator(-1, -1); }

        /**
         * @brief Implicit conversion to int
         */
        operator int() const { return _tbl_idx_(); }

        /**
         * @brief Calculate the table index for given energy and impact parameter
         *
         * Return the index to the memory location where the value that corresponds
         * to given energy and impact parameter is stored.
         *
         * Tables are stored in C-style row-major order.
         *
         * @param e the reduced energy
         * @param s the reduced impact parameter
         * @return the table index
         */
        static int table_index(const float &e, const float &s)
        {
            return e_range_t::iterator(e) * cols + s_range_t::iterator(s);
        }

    private:
        e_range_t::iterator ie_;
        s_range_t::iterator is_;
        int _tbl_idx_() const { return ie_ * cols + is_; }
    };
};

// pointers to raw scattering tables
// generated by gencorteo.cpp
const float *xs_zbl_data();
const float *xs_krc_data();
const float *xs_bohr_data();
const float *xs_moliere_data();

/*
 * Access to corteo-tabulated screened Coulomb scattering integrals
 *
 * Depending on the ScreeningType template parameter
 * data() returns a pointer to the raw scattering table
 *
 * Tables are 2-dimensional and store values of sin^2(θ(ε,s)/2) on
 * a xs_corteo_index grid of (ε,s) points.
 *
 * A 2-dimensional pre-caclulated table of \f$ \sin^2\theta/2 \f$ as a function of reduced energy
 * and reduced impact parameter, where \f$ \theta \f$ is
 * the center-of-mass scattering angle.
 *
 * The type of screening is defined by the template parameter \ref ScreeningType
 * of enum type \ref Screening.
 *
 * The table is generated by \ref xs_quad, which
 * employs Gauss–Chebyshev quadrature to compute the scattering integral.
 *
 * The tabulated values are calculated at log-spaced energy
 * and impact factor values as documented in \ref xs_corteo_index.
 *
 * @tparam ScreeningType the type of screening
 *
 * @ingroup XS
 */
template <Screening ScreeningType>
struct xs_tbl2d
{
    /// The 2D corteo index type
    typedef xs_tbl2d_grid::iterator iterator_t;
    /// Number of table rows (energy values)
    constexpr const static int rows = xs_tbl2d_grid::rows;
    /// Number of table columns (impact parameter values)
    constexpr const static int cols = xs_tbl2d_grid::cols;

    /// Returns the tabulated value of \f$ \sin^2\theta(\epsilon,s)/2 \f$
    static float sin2Thetaby2(float e, float s)
    {
        const float *p = data();
        int i = iterator_t(e, s);
        return p[i];
    }
    /// Returns the tabulated value of \f$ \sin^2\theta(i,j)/2 \f$
    static float sin2Thetaby2(int ie, int is)
    {
        const float *p = data();
        return p[ie * cols + is];
    }
    /// Returns pointer to raw tabulated data
    static const float *data() { return nullptr; }
};
template <>
inline const float *xs_tbl2d<Screening::ZBL>::data()
{
    return xs_zbl_data();
}
template <>
inline const float *xs_tbl2d<Screening::Bohr>::data()
{
    return xs_bohr_data();
}
template <>
inline const float *xs_tbl2d<Screening::KrC>::data()
{
    return xs_krc_data();
}
template <>
inline const float *xs_tbl2d<Screening::Moliere>::data()
{
    return xs_moliere_data();
}

// helper object for doing bilinear interpolation
// on the xs_tbl2d grid
struct xs_bilinear_interp
{
    typedef xs_tbl2d_grid::iterator iterator;
    constexpr const static int stride = xs_tbl2d_grid::cols;
    typedef typename xs_tbl2d_grid::e_range_t e_range_t;
    typedef typename e_range_t::iterator e_iterator_t;
    typedef typename xs_tbl2d_grid::s_range_t s_range_t;
    typedef typename s_range_t::iterator s_iterator_t;
    typedef Eigen::Vector4i idx_vec_t;
    typedef Eigen::Vector4f coef_vec_t;

    void set(float ae, float as)
    {
        e = ae;
        s = as;
        i0 = iterator(e, s);
        i1 = i0;
        if (e >= e_range_t::max) {
            i0.e_iterator()--;
            e = e_range_t::max;
        } else {
            i1.e_iterator()++;
            if (e < e_range_t::min)
                e = e_range_t::min;
        }
        if (s >= s_range_t::max) {
            i0.s_iterator()--;
            s = s_range_t::max;
        } else {
            i1.s_iterator()++;
            if (s < s_range_t::min)
                s = s_range_t::min;
        }
    }

    idx_vec_t idx() const
    {
        int k0 = i0;
        int k1 = i1;
        return { k0, k0 + 1, k1 - 1, k1 };
    }

    xs_bilinear_interp(float ae, float as) { set(ae, as); }

    coef_vec_t lin_coef() const
    {
        float t = i0.e(), u = i0.s();
        t = (e - t) / (i1.e() - t);
        u = (s - u) / (i1.s() - u);
        return { (1 - t) * (1 - u), (1 - t) * u, t * (1 - u), t * u };
    }

    coef_vec_t log2_coef() const
    {
        float t = i0.e_iterator().log2v();
        float u = i0.s_iterator().log2v();
        t = (std::log2(e) - t) / (i1.e_iterator().log2v() - t);
        u = (std::log2(s) - u) / (i1.s_iterator().log2v() - u);
        return { (1 - t) * (1 - u), (1 - t) * u, t * (1 - u), t * u };
    }

private:
    float e, s;
    iterator i0;
    iterator i1;
};

/**
 * @brief The abstract_xs_lab_tbl2d class defines the interface for lab system cross-section objects
 *
 * @ingroup XS
 */
class abstract_xs_lab_tbl2d
{
public:
    abstract_xs_lab_tbl2d() { }
    virtual ~abstract_xs_lab_tbl2d() { }
    virtual void scatter(float E, float P, float &recoil_erg, float &sintheta,
                         float &costheta) const = 0;
    virtual void scatter2(float E, float P, float &recoil_erg, float &sintheta,
                          float &costheta) const = 0;
    virtual float sin2Thetaby2(float e, float s) const = 0;
    virtual float find_p(float E, float T) const = 0;
    virtual float screening_length() const = 0;
    virtual float mass_ratio() const = 0;
    virtual float sqrt_mass_ratio() const = 0;
    virtual float gamma() const = 0;
    virtual float red_E_conv() const = 0;
    virtual const char *screeningName() const = 0;
};

/**
 * @brief A lab system cross-section class utilizing tabulated scattering integrals
 *
 * The class stores internally 2-dimensional tables of
 * - \f$ \sin^2(\theta/2), \f$
 * - \f$ \sin( \Theta ), \f$
 *
 * where \f$\theta \f$ and \f$\Theta \f$ are scattering angles in center-of-mass
 * and lab systems, on a log-spaced reduced energy and impact
 * parameter grid.
 *
 * Bilinear interpolation is used to obtain the values of these quantities
 * for the scattering of a projectile of energy \f$ E\f$ and impact parameter \f$P\f$.
 *
 * First, the reduced energy and impact parameter \f$(\epsilon, s)\f$ are found and the
 * interpolation ranges are search to find the index \f$(i,j)\f$ such that
 * \f[
 * \begin{align*}
 * \epsilon_i & \leq \epsilon < \epsilon_{i+1} \\
 * s_j & \leq s < s_{j+1}
 * \end{align*}
 * \f]
 *
 * Then, define
 * \f[
 * \begin{align*}
 * t &= (\log\epsilon - \log\epsilon_i)/(\log\epsilon_{i+1} - \log\epsilon_i) \\
 * u &= (\log s - \log s_j)/(\log s_{j+1} - \log s_j)
 * \end{align*}
 * \f]
 *
 * For a tabulated function \f$y_{i,j} = y(\epsilon_i, s_j)\f$ the bilinear interpolation
 * reads:
 * \f[
 * y(\epsilon, s) \approx y_{i,j} (1 - t)(1 - u) +
 * y_{i,j+1} (1 - t)u + y_{i+1,j} t(1 - u) + y_{i+1,j+1} tu
 * \f]
 *
 * For \f$ y = \sin^2(\theta_{CM}/2) \f$ log-log interpolation is used, while for the
 * sine and cosine of lab scattering angle the interpolation is log-lin.
 *
 * Finally, the energy transfer is obtained by \f$T=\gamma E \sin^2(\theta/2)\f$
 * and the sine and cosine of the lab scattering angle are obtained directly from
 * the interpolation.
 *
 * @tparam ScreeningType the type of screening
 *
 * @ingroup XS
 */
template <Screening ScreeningType>
class xs_lab_tbl2d : public abstract_xs_lab_tbl2d, private xs_lab<ScreeningType>
{
public:
    typedef xs_cms<ScreeningType> _xs_cms_t;
    typedef xs_lab<ScreeningType> _xs_lab_t;
    typedef xs_tbl2d<ScreeningType> _xs_tbl2d_t;
    typedef typename xs_tbl2d_grid::e_iterator_t e_iterator_t;
    typedef typename xs_tbl2d_grid::s_iterator_t s_iterator_t;
    constexpr const static int stride = xs_tbl2d_grid::cols;
    constexpr const static int array_size = xs_tbl2d_grid::size;

    // explicitly shared arrays
    typedef Eigen::VectorXf xs_array_t;
    xs_array_t sinTable;
    xs_array_t log_s2;

    xs_lab_tbl2d(int Z1, float M1, int Z2, float M2)
        : _xs_lab_t(Z1, M1, Z2, M2), sinTable(array_size), log_s2(array_size)
    {
        /* compute scattering angle components */
        double mr = mass_ratio();
        for (e_iterator_t ie; ie < ie.end(); ie++)
            for (s_iterator_t is; is < is.end(); is++) {
                float s2 = _xs_tbl2d_t::sin2Thetaby2(ie, is);

                /* convert CM scattering angle to lab frame of reference: */
                double costhetaCM = 1.0 - 2.0 * s2;
                double sinthetaCM = std::sqrt(1.0 - costhetaCM * costhetaCM);
                double thetaLab = std::atan2(sinthetaCM, (costhetaCM + mr));

                // fill the tables
                int k = ie * stride + is;
                sinTable[k] = std::sin(thetaLab);
                log_s2[k] = std::log2(s2);
            }
    }
    xs_lab_tbl2d(const xs_lab_tbl2d &x) : _xs_lab_t(x), sinTable(x.sinTable), log_s2(x.log_xs_) { }

    virtual void scatter2(float e, float s, float &recoil_erg, float &sintheta,
                          float &costheta) const override
    {
        recoil_erg = e * gamma();
        e *= red_E_conv();
        s /= screening_length();

        // get coeffs for bilin & bilog interpolation
        xs_bilinear_interp interp(e, s);
        Eigen::Vector4i i = interp.idx();
        Eigen::Vector4f log2_coeff = interp.log2_coef();

        // bilog interpolation for sin^2(θ/2)
        float s2 = std::exp2(log2_coeff.dot(log_s2(i)));
        recoil_erg *= s2;

        /* convert CM scattering angle to lab frame of reference: */
        costheta = 1.0f - 2.0f * s2;
        sintheta = std::sqrt(1.0f - costheta * costheta);
        float th = std::atan2(sintheta, (costheta + mass_ratio()));
        costheta = std::cos(th);
        sintheta = std::sin(th);
    }

    virtual void scatter(float e, float s, float &recoil_erg, float &sintheta,
                         float &costheta) const override
    {
        recoil_erg = e * gamma();
        e *= red_E_conv();
        s /= screening_length();

        // get coeffs for bilin & bilog interpolation
        xs_bilinear_interp interp(e, s);
        Eigen::Vector4i i = interp.idx();
        Eigen::Vector4f lin_coeff = interp.lin_coef();
        Eigen::Vector4f log2_coeff = interp.log2_coef();

        // bilinear interpolation for lab sinTh
        sintheta = lin_coeff.dot(sinTable(i));
        costheta = std::sqrt(1.f - sintheta * sintheta);

        // bilog interpolation for sin^2(θ/2)
        recoil_erg *= std::exp2(log2_coeff.dot(log_s2(i)));
    }

    virtual float sin2Thetaby2(float e, float s) const override
    {
        // get coeffs for bilin & bilog interpolation
        xs_bilinear_interp interp(e, s);
        Eigen::Vector4i i = interp.idx();
        Eigen::Vector4f log2_coeff = interp.log2_coef();

        // bilog interpolation for sin^2(θ/2)
        return std::exp2(log2_coeff.dot(log_s2(i)));
    }

    virtual float find_p(float E, float T) const { return _xs_lab_t::find_p(E, T); }
    virtual float screening_length() const { return _xs_lab_t::screening_length(); }
    virtual float mass_ratio() const { return _xs_lab_t::mass_ratio(); }
    virtual float sqrt_mass_ratio() const { return _xs_lab_t::sqrt_mass_ratio(); }
    virtual float gamma() const { return _xs_lab_t::gamma(); }
    virtual float red_E_conv() const { return _xs_lab_t::red_E_conv(); }
    virtual const char *screeningName() const { return _xs_lab_t::screeningName(); }
};

/*
 * @brief xs_lab implementation with ZBL potential and 4-bit corteo tabulated scattering integrals
 *
 * @ingroup XS
 */
typedef xs_lab_tbl2d<Screening::ZBL> xs_lab_zbl;
/*
 * @brief xs_lab implementation with Bohr potential and 4-bit corteo tabulated scattering
 * integrals
 *
 * @ingroup XS
 */
typedef xs_lab_tbl2d<Screening::Bohr> xs_lab_bohr;
/*
 * @brief xs_lab implementation with Kr-C potential and 4-bit corteo tabulated scattering integrals
 *
 * @ingroup XS
 */
typedef xs_lab_tbl2d<Screening::KrC> xs_lab_krc;
/*
 * @brief xs_lab implementation with Moliere potential and 4-bit corteo tabulated scattering
 * integrals
 *
 * @ingroup XS
 */
typedef xs_lab_tbl2d<Screening::Moliere> xs_lab_moliere;

#endif // CORTEO_XS_H
