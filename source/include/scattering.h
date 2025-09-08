#ifndef SCATTERING_H
#define SCATTERING_H

#include <screened_coulomb.h>
#include <ieee754_seq.h>

#include <Eigen/Dense>

/**
 * \defgroup XS Nuclear scattering
 *
 * \brief Scattering of ions by target nuclei.
 *
 * The interaction is described by the screened Coulomb potential.
 *
 * The scattering is considered elastic and it is approximated by classical kinematics.
 *
 * For fast calculation of scattering events in the Monte-Carlo simulation,
 * scattering quantities are tabulated on a 2-d grid of reduced energy and impact parameter, \f$
 * (\epsilon_i, s_j ) \f$.
 *
 * Both \f$ \epsilon_i \f$ and \f$ s_j  \f$ are quasi log-spaced sequences based on the IEEE754
 * floating point number representation as employed in the program Corteo (F. Schiettekatte,
 * http://www.lps.umontreal.ca/~schiette/index.php?n=Recherche.Corteo).
 *
 * The 2-d grid is defined in \ref scattering_tbl_grid.
 *
 * Bi-linear or bi-log
 * interpolation is used to obtain values of scattering quantities for arbitrary
 * \f$ (\epsilon, s ) \f$. This is done with \ref scattering_tbl_grid::bilinear_interp.
 *
 * The class \ref scattering_calc performs
 * scattering calculations for a specific projectile/target combination.
 *
 */

/**
 * @brief Defines a 2-d grid for scattering tables
 *
 * Quantities are tabulated as a function of reduced energy and impact parameter on a 2-d grid \f$
 * (\epsilon_i, s_j ) \f$.
 *
 * Both \f$ \epsilon_i \f$ and \f$ s_j  \f$ are quasi log-spaced sequences of type `ieee754_seq`
 * (https://github.com/ir2-lab/ieee754_seq) optimized for fast look-up and fewer calls to `log()`.
 *
 * The interpolation scheme is based on the program Corteo (F. Schiettekatte,
 * http://www.lps.umontreal.ca/~schiette/index.php?n=Recherche.Corteo).
 *
 * The range of the grid dimensions are as follows:
 *   - \ref e_range_t : \f$ \epsilon_{min} = 2^{-19}\approx 2\cdot 10^{-6}\f$,
 *   \f$\epsilon_{max}= \leq 2^{21} \approx 2\cdot 10^6\f$,
 *   \# of intervals \f$ N = 640 = 2^4\cdot (21-(-19))\f$
 *   - \ref s_range_t \f$ s_{min} = 2^{-26}\approx 1.5\cdot 10^{-8}\f$,
 *   \f$s_{max}=  2^{6}=64\f$,
 *   \# of intervals \f$ M = 512 = 2^4\cdot (6-(-26))\f$
 *
 * The class defines also an \ref iterator for iterating over the grid points and
 * the \ref bilinear_interp for perfoming bilinear interpolations.
 *
 * The amount of memory required for storing a scattering table using 32-bit floats is *1.25 MB*
 *
 * @sa
 * https://github.com/ir2-lab/ieee754_seq, \n
 * http://www.lps.umontreal.ca/~schiette/index.php?n=Recherche.Corteo
 *
 * @ingroup XS
 */
struct scattering_tbl_grid
{
    /// Reduced energy range
    typedef ieee754_seq<float, 4, -19, 21> e_range_t;

    /// Reduced energy range iterator
    typedef typename e_range_t::iterator e_iterator_t;

    /// Reduced impact parameter range
    typedef ieee754_seq<float, 4, -26, 6> s_range_t;

    /// Reduced impact parameter range
    typedef typename s_range_t::iterator s_iterator_t;

    /// number of rows (energy values)
    constexpr static const int rows = e_range_t::count;

    /// number of columns (impact parameter values)
    constexpr static const int cols = s_range_t::count;

    /// total number of elements
    constexpr static const int size = rows * cols;

    /**
     * @brief Iterator object for the 2-d scatterig table
     *
     * Stores 2 iterators, one for the reduced energy and one for the impact
     * parameter range.
     *
     */
    struct iterator
    {

        /**
         * @brief Construct an iterator a index `(ie,is)`
         */
        constexpr iterator(int ie = 0, int is = 0) : ie_(ie), is_(is) { }
        /**
         * @brief Construct an iterator pointing to the lower grid point of the vertex containing
         * the point `(e,s)`
         */
        constexpr iterator(float e, float s) : ie_(e), is_(s) { }

        /**
         * @brief Return a reference to the energy range iterator
         */
        e_iterator_t &e_iterator() { return ie_; }
        /**
         * @brief Return a reference to the impact parameter iterator
         */
        s_iterator_t &s_iterator() { return is_; }
        /**
         * @brief Return a constant reference to the energy range iterator
         */
        const e_iterator_t &e_iterator() const { return ie_; }
        /**
         * @brief Return a constant reference to the impact parameter iterator
         */
        const s_iterator_t &s_iterator() const { return is_; }
        /**
         * @brief Return the reduced energy at the vertex pointed to by the iterator
         */
        float e() const { return ie_.value(); }
        /**
         * @brief Return the reduced impact parameter at the vertex pointed to by the iterator
         */
        float s() const { return is_.value(); }

        /**
         * @brief Advance the iterator to the next grid vertex. The s iterator is increased first
         * @return A reference to the iterator
         */
        iterator &operator++()
        {
            if (is_ < s_range_t::count)
                is_++;
            if (is_ == s_range_t::count) {
                if (ie_ < e_range_t::count) {
                    ie_++;
                    if (ie_ != e_range_t::count)
                        is_ = 0;
                }
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
         * @brief Reduce the iterator to the previous grid vertex. The s iterator is reduced first
         * @return A reference to the iterator
         */
        iterator &operator--()
        {
            if (is_ >= 0)
                is_--;
            if (is_ == -1) {
                if (ie_ >= 0) {
                    ie_--;
                    if (ie_ != -1)
                        is_ = s_range_t::count - 1;
                }
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

    private:
        e_range_t::iterator ie_;
        s_range_t::iterator is_;

        // Return the index to the memory location where the value that corresponds
        // to given energy and impact parameter is stored.
        //
        // Tables are stored in C-style row-major order.
        int _tbl_idx_() const { return ie_ * cols + is_; }
    };

    /**
     * @brief Returns an iterator at the first grid point
     */
    constexpr static iterator begin() { return iterator(0, 0); }
    /**
     * @brief Returns an iterator at one past the last grid point
     */
    constexpr static iterator end() { return iterator(rows, cols); }

    /**
     * @brief Bilinear interpolator class for 2-d scattering tables
     *
     * Bilinear interpolation is used to obtain the values of scattering quantities
     * from 2-d tabulations on a grid \f$(\epsilon_i, s_j)\f$.
     *
     * For given reduced energy and impact parameter values, \f$(\epsilon, s)\f$,
     * the table index pair \f$(i,j)\f$ is found such that
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
     * t &= (\epsilon - \epsilon_i)/(\epsilon_{i+1} - \epsilon_i) \\
     * u &= (s - s_j)/(s_{j+1} - s_j)
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
     * In the present implementation, the idx() function returns a vector of
     * indices pointing to the memory locations within the tabulated matrix were the
     * values \f$(y_{i,i}, y_{i,j+1},\dots)\f$ are stored.
     *
     * lin_coef() returns the vector of corresponding interpolating coefficients,
     * \f$\left[ (1-t)(1-u), (1-t)u, \dots \right]\f$.
     *
     * log2_coef() returns a similar coefficient vector for base-2 log interpolation.
     * Base-2 log is used due to the underlying `ieee754_seq`-type ranges ability to
     * optimize the calculation of base-2 logarithms.
     *
     * Finally, the interpolated value can be found by taking the dot product of the coefficient
     * and tabulated function value vectors. Using the Eigen C++ library this can be done as
     * follows:
     *
     * @code
     *
     * Eigen::VectorXf y_table; // interpolation data
     * bilinear_interp I;
     * Eigen::Vector4i i = I.idx();
     * Eigen::Vector4f c = I.lin_coef();
     *
     * float y = c.dot(y_data(i)); // interpolated value
     *
     * @endcode
     *
     *
     */
    struct bilinear_interp
    {
        typedef scattering_tbl_grid::iterator iterator;
        constexpr const static int stride = cols;

        /// \brief Type of 4-element index vector
        typedef Eigen::Vector4i idx_vec_t;
        /// \brief Type of 4-element coefficient vector
        typedef Eigen::Vector4f coef_vec_t;

        /**
         * @brief Construct a bilinear_interp object at given \f$(\epsilon,s)\f$
         */
        bilinear_interp(float ae, float as)
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

        /**
         * @brief Return a 4-element vector of indices
         */
        idx_vec_t idx() const
        {
            int k0 = i0;
            int k1 = i1;
            return { k0, k0 + 1, k1 - 1, k1 };
        }

        /**
         * @brief Return a 4-element vector of linear interpolation coefficients
         */
        coef_vec_t lin_coef() const
        {
            float t = i0.e(), u = i0.s();
            t = (e - t) / (i1.e() - t);
            u = (s - u) / (i1.s() - u);
            return { (1 - t) * (1 - u), (1 - t) * u, t * (1 - u), t * u };
        }

        /**
         * @brief Return a 4-element vector of log2 interpolation coefficients
         */
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
};

// pointers to raw scattering tables
// generated by gen_scattering_tbl.cpp
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
struct cms_scattering_tbl : public scattering_tbl_grid
{
    cms_scattering_tbl()
    {
        switch (ScreeningType) {
        case Screening::Bohr:
            s2_ = xs_bohr_data();
            break;
        case Screening::KrC:
            s2_ = xs_krc_data();
            break;
        case Screening::Moliere:
            s2_ = xs_moliere_data();
            break;
        case Screening::ZBL:
            s2_ = xs_zbl_data();
            break;
        default:
            break;
        }

        if (!is_initialized_ && data()) {
            log2_s2_.resize(size);
            for (int i = 0; i < size; i++)
                log2_s2_[i] = std::log2(data()[i]);
            is_initialized_ = true;
        }
    }

    /// Returns the tabulated value of \f$ \sin^2\theta(\epsilon,s)/2 \f$
    static float sin2Thetaby2(float e, float s)
    {
        const float *p = data();
        int i = iterator(e, s);
        return p[i];
    }
    /// Returns the tabulated value of \f$ \sin^2\theta(i,j)/2 \f$
    static float sin2Thetaby2(int ie, int is)
    {
        const float *p = data();
        int i = iterator(ie, is);
        return p[i];
    }

    /// Returns pointer to raw tabulated data
    static const float *data() { return s2_; }
    /// Returns pointer to raw tabulated data for log2(sin^2(thetaCM/2))
    static const float *log2_data() { return log2_s2_.data(); }

private:
    inline static const float *s2_ = nullptr;
    inline static std::vector<float> log2_s2_;
    inline static bool is_initialized_ = false;
};

/**
 * @brief The abstract_xs_lab_tbl2d class defines the interface for lab system cross-section objects
 *
 * @ingroup XS
 */
class abstract_scattering_calc
{
public:
    abstract_scattering_calc() { }
    virtual ~abstract_scattering_calc() { }
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
 * @brief A scattering calculator class utilizing tabulated scattering quantities
 *
 * The class uses internally stored 2-dimensional tables of
 * - \f$ \sin^2(\theta/2), \f$
 * - \f$ \sin\Theta, \f$
 *
 * where \f$\theta \f$ and \f$\Theta \f$ are scattering angles in center-of-mass
 * and lab systems, respectively, on a log-spaced reduced energy and impact
 * parameter grid defined by \ref scattering_tbl_grid.
 *
 * Bilinear interpolation is used to obtain the values of these quantities
 * for the scattering of a projectile of energy \f$ E\f$ and impact parameter \f$P\f$,
 * utilizing the class \ref scattering_tbl_grid::bilinear_interp.
 *
 * @tparam ScreeningType the type of screening
 *
 * @ingroup XS
 */
template <Screening ScreeningType>
class scattering_calc : public abstract_scattering_calc,
                        public cms_scattering_tbl<ScreeningType>,
                        private xs_lab<ScreeningType>
{
public:
    typedef xs_cms<ScreeningType> _xs_cms_t;
    typedef xs_lab<ScreeningType> _xs_lab_t;
    typedef cms_scattering_tbl<ScreeningType> _cms_tbl_t;

    // explicitly shared arrays
    typedef Eigen::VectorXf xs_array_t;
    typedef Eigen::Map<const xs_array_t> xs_array_map_t;
    xs_array_t sinTable;

    /**
     * @brief Construct an object for scattering calculations on a specific projectile/target
     * combination
     * @param Z1 projectile atomic number
     * @param M1 projectile atomic mass
     * @param Z2 target atom atomic number
     * @param M2 target atom atomic mass
     */
    scattering_calc(int Z1, float M1, int Z2, float M2)
        : _xs_lab_t(Z1, M1, Z2, M2), sinTable(scattering_tbl_grid::size)
    {
        /* compute scattering angle components */
        double mr = mass_ratio();
        // fill the log_s2 and sinTable
        for (int k = 0; k < scattering_tbl_grid::size; k++) {
            float s2 = _cms_tbl_t::data()[k]; // s2 = sin^2(thetaCM/2)

            /* convert CM scattering angle to lab frame of reference: */
            double costhetaCM = 1.0 - 2.0 * s2;
            double sinthetaCM = std::sqrt(1.0 - costhetaCM * costhetaCM);
            double thetaLab = std::atan2(sinthetaCM, (costhetaCM + mr));

            // fill the tables
            sinTable[k] = std::sin(thetaLab);
        }
    }
    /**
     * @brief Copy constructor.
     */
    scattering_calc(const scattering_calc &x) : _xs_lab_t(x), sinTable(x.sinTable) { }

    /**
     * @brief Calculate a scattering event
     *
     * For the scattering of a projectile with given energy \f$E\f$ and impact parameter \f$P\f$,
     * calculate the recoil energy \f$T\f$ and the scattering angle sine and cosine.
     *
     * The energy and impact parameter are converted to reduced values, \f$(\epsilon, s)\f$.
     * These are used to obtain the values of \f$\sin^2(\theta/2)\f$ and \f$\sin\Theta\f$ by
     * log-log and log-lin bilinear interpolation, respectively, from the stored scattering tables.
     *
     * The recoil energy is given by \f$T = \gamma E \sin^2(\theta/2)\f$ and the scattering
     * angle cosine is
     * obtained by \f$ \cos\Theta = \sqrt{1-\sin^2\Theta}\f$
     *
     * @param E Projectile energy [eV]
     * @param S Impact parameter [nm]
     * @param T Recoil energy [eV]
     * @param sinTheta Scattering angle sine
     * @param cosTheta Scattering angle cosine
     */
    virtual void scatter(float E, float S, float &T, float &sinTheta,
                         float &cosTheta) const override;

    // test for calculating sintheta and costheta without interpolation
    // It is 30% slower than scatter
    virtual void scatter2(float e, float s, float &recoil_erg, float &sintheta,
                          float &costheta) const override;

    virtual float sin2Thetaby2(float e, float s) const override
    {
        // get coeffs for bilin & bilog interpolation
        scattering_tbl_grid::bilinear_interp interp(e, s);
        Eigen::Vector4i i = interp.idx();
        Eigen::Vector4f log2_coeff = interp.log2_coef();

        // bilog interpolation for sin^2(θ/2)
        xs_array_map_t log2_s2(_cms_tbl_t::log2_data(), scattering_tbl_grid::size);
        return std::exp2(log2_coeff.dot(log2_s2(i)));
    }

    /**
     * @brief Return the impact parameter [nm] for given incoming (E) and recoil (T) energy [eV]
     */
    virtual float find_p(float E, float T) const override { return _xs_lab_t::find_p(E, T); }

    /**
     * @brief Return the screening length [nm]
     */
    virtual float screening_length() const override { return _xs_lab_t::screening_length(); }

    /**
     * @brief Return the projectile to target mass ratio
     */
    virtual float mass_ratio() const override { return _xs_lab_t::mass_ratio(); }

    /**
     * @brief Return the square root of the mass ratio
     */
    virtual float sqrt_mass_ratio() const override { return _xs_lab_t::sqrt_mass_ratio(); }

    /**
     * @brief Return the constant \f$\gamma = 4 m_1 m_2 / (m_1+m_2)^2\f$
     */
    virtual float gamma() const override { return _xs_lab_t::gamma(); }

    /**
     * @brief Return the factor \f$f\f$ [eV^-1] to convert energy to reduced energy, \f$\epsilon =
     * f\cdot E\f$
     */
    virtual float red_E_conv() const override { return _xs_lab_t::red_E_conv(); }

    /**
     * @brief Return the name of the screening function
     */
    virtual const char *screeningName() const override { return _xs_lab_t::screeningName(); }
};

template <>
class scattering_calc<Screening::None> : public abstract_scattering_calc,
                                         private xs_lab<Screening::None>
{
public:
    typedef xs_cms<Screening::None> _xs_cms_t;
    typedef xs_lab<Screening::None> _xs_lab_t;

    scattering_calc(int Z1, float M1, int Z2, float M2) : _xs_lab_t(Z1, M1, Z2, M2) { }
    scattering_calc(const scattering_calc &x) : _xs_lab_t(x) { }

    virtual void scatter2(float e, float s, float &recoil_erg, float &sintheta,
                          float &costheta) const override
    {
        scatter(e, s, recoil_erg, sintheta, costheta);
    }

    virtual void scatter(float e, float s, float &recoil_erg, float &sintheta,
                         float &costheta) const override
    {
        double th, T;
        _xs_lab_t::scatter(e, s, T, th);
        recoil_erg = T;
        costheta = std::cos(th);
        sintheta = std::sin(th);
    }

    virtual float sin2Thetaby2(float e, float s) const override
    {
        return _xs_cms_t::sin2Thetaby2(e, s);
    }

    virtual float find_p(float E, float T) const override { return _xs_lab_t::find_p(E, T); }
    virtual float screening_length() const override { return _xs_lab_t::screening_length(); }
    virtual float mass_ratio() const override { return _xs_lab_t::mass_ratio(); }
    virtual float sqrt_mass_ratio() const override { return _xs_lab_t::sqrt_mass_ratio(); }
    virtual float gamma() const override { return _xs_lab_t::gamma(); }
    virtual float red_E_conv() const override { return _xs_lab_t::red_E_conv(); }
    virtual const char *screeningName() const override { return _xs_lab_t::screeningName(); }
};

template <Screening ScreeningType>
void scattering_calc<ScreeningType>::scatter(float e, float s, float &recoil_erg, float &sintheta,
                                             float &costheta) const
{
    recoil_erg = e * _xs_lab_t::gamma();
    e *= _xs_lab_t::red_E_conv();
    s /= _xs_lab_t::screening_length();

    // get coeffs for bilin & bilog interpolation
    scattering_tbl_grid::bilinear_interp interp(e, s);
    Eigen::Vector4i i = interp.idx();
    Eigen::Vector4f lin_coeff = interp.lin_coef();
    Eigen::Vector4f log2_coeff = interp.log2_coef();

    // bilinear interpolation for lab sinTh
    sintheta = lin_coeff.dot(sinTable(i));
    costheta = std::sqrt(1.f - sintheta * sintheta);

    // bilog interpolation for sin^2(θ/2)
    xs_array_map_t log2_s2(_cms_tbl_t::log2_data(), scattering_tbl_grid::size);
    recoil_erg *= std::exp2(log2_coeff.dot(log2_s2(i)));
}

template <Screening ScreeningType>
void scattering_calc<ScreeningType>::scatter2(float e, float s, float &recoil_erg, float &sintheta,
                                              float &costheta) const
{
    recoil_erg = e * _xs_lab_t::gamma();
    e *= _xs_lab_t::red_E_conv();
    s /= _xs_lab_t::screening_length();

    // get coeffs for bilin & bilog interpolation
    scattering_tbl_grid::bilinear_interp interp(e, s);
    Eigen::Vector4i i = interp.idx();
    Eigen::Vector4f log2_coeff = interp.log2_coef();

    // bilog interpolation for sin^2(θ/2)
    xs_array_map_t log2_s2(_cms_tbl_t::log2_data(), scattering_tbl_grid::size);
    float s2 = std::exp2(log2_coeff.dot(log2_s2(i)));
    recoil_erg *= s2;

    /* convert CM scattering angle to lab frame of reference: */
    costheta = 1.0f - 2.0f * s2;
    sintheta = std::sqrt(1.0f - costheta * costheta);
    float th = std::atan2(sintheta, (costheta + mass_ratio()));
    costheta = std::cos(th);
    sintheta = std::sin(th);
}

typedef scattering_calc<Screening::ZBL> zbl_scattering_calc;
typedef scattering_calc<Screening::Bohr> bohr_scattering_calc;
typedef scattering_calc<Screening::KrC> krc_scattering_calc;
typedef scattering_calc<Screening::Moliere> moliere_scattering_calc;
typedef scattering_calc<Screening::None> unscreened_scattering_calc;

#endif // SC_TBL_H
