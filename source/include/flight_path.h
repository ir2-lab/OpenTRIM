#ifndef FLIGHT_PATH_H
#define FLIGHT_PATH_H

#include <ieee754_seq.h>
#include "arrays.h"
#include "random_vars.h"

#define SAMPLE_P_AND_N 1

#define SQRT_4over3 1.1547005f

class ion;
class material;
class mccore;

/**
 * @brief The flight_path_calc class samples the ion's flight path and impact parameter
 *
 * The sampling is performed according to the algorithm specified by the
 * \ref flight_path_type_t enum.
 *
 * If \ref flight_path_type_t is \ref Constant
 * then the flight path \f$\ell\f$ is equal to the user specified
 * parameter mccore::transport_options::flight_path_const.
 *
 * The value is in units of the current
 * material's atomic radius
 * \f$ R_{at} = \left(\frac{3}{4\pi}N\right)^{-1/3} \f$.
 *
 * The impact parameter \f$p\f$ is calculated as
 * $$
 * p = (\pi \ell N)^{-1/2}\sqrt{u},
 * $$
 * where \f$u\f$ is a random number sampled uniformly in \f$(0,1)\f$.
 *
 * For the Variable sampling algorithm there are precomputed tables as a function of energy
 * of the mean free path \f$\ell\f$ , the maximum impact parameter \f$p_{max}\f$ and
 * the maximum fligth path \f$\Delta x_{max}\f$. For more information see \ref flightpath.
 *
 * Sampling is performed as follows:
 * - \f$\Delta x = -\ell(E) \log{u_1}\f$
 * - \f$p = p_{max}(E)\sqrt{u_2}\f$
 * - Reject collision if \f$\Delta x > \Delta x_{max}(E)\f$
 *
 * The flight_path_calc class is first initialized for a given simulation by calling
 * flight_path_calc::init(). This pre-computes the data tables that will be used
 * during the Monte-Carlo simulation.
 *
 * During the simulation, preload() is called to select the pre-computed tables for a specific
 * ion/material combination.
 *
 * Multiple calls to operator() can then be made to sample the ion's
 * flight path and impact parameter.
 *
 * \ingroup Core
 *
 * \sa \ref flightpath
 *
 */
class flight_path_calc
{
public:
    /**
     * @brief Flight path sampling algorithm
     *
     * Determines the algorithm to select the
     * free flight path between collisions.
     */
    enum flight_path_type_t {
        Constant = 0, /**< Constant flight path (no sampling)*/
        Variable = 1, /**< Variable flight path (sampling with an energy dependent mean free path)*/
        InvalidPath = -1
    };

    /**
     * @brief Energy grid for flight path selection tables
     *
     * This is a 4-bit ieee754_seq, providing a fast access
     * log-spaced energy grid.
     *
     * The energy range in [eV], \f$ 2^4 = 16 \leq E \leq 2^{30} \sim 10^9 \f$, is
     * divided in (30-4)*2^4 = 416 approx. log-spaced intervals.
     *
     */
    typedef ieee754_seq<float, 4, 4, 30> fp_tbl_erange;
    /// @brief An iterator for accesing the energy grid
    typedef fp_tbl_erange::iterator fp_tbl_iterator;

    /**
     * @brief Default constructor
     *
     */
    flight_path_calc();

    /**
     * @brief Copy constructor
     *
     * Internal data tables are explicitly shared
     *
     */
    flight_path_calc(const flight_path_calc &other);

    /// @brief Return the type of flight path sampling
    flight_path_type_t type() const { return type_; }

    /// @brief Return the table of mfp values vs energy
    ArrayNDf mfp() const { return mfp_; }
    /// @brief Return the table of p_max values vs energy
    ArrayNDf ipmax() const { return ipmax_; }
    /// @brief Return the table of max flight path values vs energy
    ArrayNDf fpmax() const { return fp_max_; }

    /**
     * @brief Initialize the object for a given simulation
     *
     * This function pre-computes all the data required for flight-path
     * selection during a simulation.
     *
     * This includes
     *   - the ion mean free path
     *   - the max impact parameter
     *   - the max flight path, set by the limit of rel. electronic energy loss
     *
     * The above quantities are calculated for all ion/material combinations
     * in the given simulation and as a function of ion energy on an energy grid defined by @ref
     * dedx_erange.
     *
     * @param s The parent simulation object
     * @return
     */
    int init(const mccore &s);

    /// @brief Preload internal data tables for the given ion/material combination
    int preload(const ion *i, const material *m);

    /**
     * @brief Generate samples for the ion's flight path and impact parameter
     *
     * The selection is performed for the ion/material combination that has
     * been set by the last call to preload() and according to the algorithm specified by
     * transport_options::flight_path_type.
     *
     * @param[in] rng a random number generator object
     * @param[in] E the ion's energy [eV]
     * @param[out] fp the ion flight path [nm]
     * @param[out] ip impact parameter [nm]

     * @return true if the ion should collide at the end of its flight path
     *
     * @sa \ref flightpath
     */
    bool operator()(random_vars &rng, float E, float &fp, float &ip)
    {
        bool doCollision = true;
        int ie;
        float u;

#if SAMPLE_P_AND_N == 1

        rng.random_azimuth_dir_norm(nx_, ny_, u);

#else

        u = rng.u01s_open();

#endif

        switch (type_) {
        case Constant:
            fp = fp_;
            ip = ip_ * std::sqrt(u);
            break;
        case Variable:
            ie = fp_tbl_iterator(E);
            doCollision = u >= umin_tbl[ie];
            if (doCollision) {
                fp = mfp_tbl[ie] * (-std::log(u));
                ip = ipmax_tbl[ie] * std::sqrt(rng.u01s_lopen());
            } else {
                fp = fpmax_tbl[ie];
            }
            break;
        default:
            assert(false); // never get here
        }
        assert(fp > 0);
        assert(finite(fp));
        return doCollision;
    }

    /// @brief Return nx = cos of azimuthal scattering angle
    float nx() const { return nx_; }
    /// @brief Return ny = sin of azimuthal scattering angle
    float ny() const { return ny_; }

protected:
    flight_path_type_t type_;

    // helper variables for flight path calc
    std::vector<float> fp_const;
    std::vector<float> ip0;

    // optional random 2d dir
    float nx_, ny_;

    // flight path selection tables
    ArrayNDf mfp_, ipmax_, fp_max_, umin_;

    // Flight path [nm]
    float fp_;
    // Impact parameter [nm]
    float ip_;
    // Tabulated max impact parameter as a function of ion energy
    const float *ipmax_tbl;
    // Tabulated mean free path as a function of ion energy
    const float *mfp_tbl;
    // Tabulated max flight path as a function of ion energy
    const float *fpmax_tbl;
    // Tabulated min random var to reject collisions
    const float *umin_tbl;
};

#endif // FLIGHT_PATH_H
