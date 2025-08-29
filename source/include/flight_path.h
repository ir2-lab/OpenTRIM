#ifndef FLIGHT_PATH_H
#define FLIGHT_PATH_H

#include <corteo.h>
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
 * For the other algorithms there are precomputed tables as a function of energy
 * of the mean free path \f$\ell_0\f$ , the maximum impact parameter \f$p_{max}\f$ and
 * the maximum fligth path \f$\ell_{max}\f$. For more information see \ref flightpath.
 *
 * In the \ref MHW algorithm, \f$\ell\f$ and \f$p\f$
 * are computed  as follows:
 * - If \f$p_{max}(E)< R_{at} \f$ set
 * $$
 * p = p_{max}\sqrt{-\log{u}} \quad \mbox{and} \quad \ell=\ell_0(E).
 * $$
 * Reject collision if \f$p>p_{max}\f$.
 * - otherwise:
 * $$
 * \ell = (4/3)R_{at} \quad \mbox{and} \quad p = R_{at} \sqrt{u}.
 * $$
 *
 * Finally, the \ref FullMC algorithm does the following:
 * - \f$\ell = -\ell_0(E) \log{u_1}\f$
 * - \f$p = p_{max}(E)\sqrt{u_2}\f$
 * - Reject collision if \f$\ell > \ell_{max}(E)\f$
 *
 * The flight_path_calc class is first initialized for a given simulation by calling
 * flight_path_calc::init(). This pre-computes a number of data tables that will be used
 * during the Monte-Carlo simulation.
 *
 * During the simulation, preload() is called to select pre-computed tables for a specific
 * ion/material combination. Then, multiple calls to operator() can be made to sample the ion's
 * flight path and impact parameter.
 *
 * \ingroup Core
 * \sa \ref flightpath
 *
 */
class flight_path_calc
{
public:
    /**
     * @brief Flight path selection algorithm
     *
     * Determines the algorithm to select the
     * free flight path \f$\ell\f$ between collisions.
     */
    enum flight_path_type_t {
        Constant = 0, /**< Constant flight path */
        MHW = 1, /**< Algorithm from Mendenhall-Weller NIMB2005, SRIM-like*/
        FullMC = 2, /**< Full Monte-Carlo flight path selection */
        InvalidPath = -1
    };

    /**
     * @brief Iterator for flight path selection tables
     *
     * This is a 4-bit corteo::iterator, providing a fast access
     * log-spaced energy grid.
     *
     * The energy range in [eV], \f$ 2^4 = 16 \leq E \leq 2^{30} \sim 10^9 \f$, is
     * divided in (30-4)*2^4 = 416 approx. log-spaced intervals.
     *
     */
    typedef corteo::iterator<float, int, 4, 4, 30> fp_tbl_iterator;

    flight_path_calc();
    flight_path_calc(const flight_path_calc &other);

    flight_path_type_t type() const { return type_; }

    // Tables of flight path selection parameters
    ArrayNDf mfp() const { return mfp_; }
    ArrayNDf ipmax() const { return ipmax_; }
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
     * dedx_index.
     *
     * @param s The parent simulation object
     * @return
     */
    int init(const mccore &s);

    /// Preload internal data tables for the given ion/material combination
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
     * @param[out] sqrtfp square root of ratio (flight path)/(atomic radius)
     * @param[out] ip impact parameter [nm]

     * @return true if the ion should collide at the end of its flight path
     *
     * @sa \ref flightpath
     */
    bool operator()(random_vars &rng, float E, float &fp, float &ip)
    {
        constexpr const float mhw_umin = 1. / M_E;

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
        case MHW:
            ie = fp_tbl_iterator(E);
            ip = ipmax_tbl[ie];
            fp = mfp_tbl[ie];
            if (ip < ip_) { // : ipmax < Rat
                doCollision = u >= mhw_umin;
                if (doCollision) {
                    ip *= std::sqrt(-std::log(u));
                    fp *= rng.u01s();
                }
            } else { // ipmax = Rat
                fp = fp_;
                ip = ip_ * std::sqrt(u);
            }
            break;
        case FullMC:
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

    float nx() const { return nx_; }
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
