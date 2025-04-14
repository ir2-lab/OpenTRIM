#ifndef FLIGHT_PATH_CALC_H
#define FLIGHT_PATH_CALC_H

#include "dedx.h"
#include "arrays.h"
#include "random_vars.h"

class ion;
class material;
class mccore;

/**
 * @brief The flight_path_calc class estimates the ion's flight path
 *
 * The calculation is performed according to the algorithm specified by the
 * \ref flight_path_type_t enum.
 *
 * If \ref flight_path_type_t is equal to \ref AtomicSpacing or \ref Constant
 * then the flight path \f$\ell\f$ is precalculated and equal either to the current
 * material's atomic radius
 * \f$ R_{at} = \left(\frac{3}{4\pi}N\right)^{1/3} \f$ or
 * to user specified constant value, respectively.
 *
 * In both of these cases the impact parameter \f$p\f$ is calculated as
 * $$
 * p = (\pi \ell N)^{-1/2}\sqrt{u},
 * $$
 * where \f$u\f$ is a random number sampled uniformly in \f$(0,1)\f$.
 *
 * For the other algorithms there are precomputed tables as a function of energy
 * of the mean free path \f$\ell_0\f$ , the maximum impact parameter \f$p_{max}\f$ and
 * the maximum fligth path \f$\ell_{max}\f$. For more information see \ref flightpath.
 *
 * In the \ref MendenhallWeller algorithm \f$\ell\f$ and \f$p\f$
 * are computed  as follows:
 * - If \f$p_{max}(E)<(\pi R_{at} N)^{-1/2}\f$ set
 * $$
 * p = p_{max}\sqrt{-\log{u}} \quad \mbox{and} \quad \ell=\ell_0(E).
 * $$
 * Reject collision if \f$p>p_{max}\f$.
 * - otherwise:
 * $$
 * \ell = R_{at} \quad \mbox{and} \quad p = (\pi \ell N)^{-1/2}\sqrt{u}.
 * $$
 *
 * Finally, the \ref FullMC algorithm does the following:
 * - \f$\ell = -\ell_0(E) \log{u_1}\f$
 * - \f$p = p_{max}(E)\sqrt{u_2}\f$
 * - Reject collision if \f$\ell > \ell_{max}(E)\f$
 *
 * The flight_path_calc class is first initialized for a given simulation by a calling
 * flight_path_calc::init(). This pre-computes a number of data tables that will be used
 * during the Monte-Carlo simulation.
 *
 * During the simulation, preload() is called to select pre-computed tables for a specific
 * ion/material combination. Then, multiple calls to operator() can be made to sample the ion's
 * flight path.
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
        AtomicSpacing = 0, /**< Constant, equal to interatomic distance */
        Constant = 1, /**< Constant, equal to user supplied value */
        MendenhallWeller = 2, /**< Algorithm from Mendenhall-Weller NIMB2005*/
        FullMC = 3, /**< Full Monte-Carlo flight path algorithm */
        InvalidPath = -1
    };

    flight_path_calc();
    flight_path_calc(const flight_path_calc &other);

    flight_path_type_t type() const { return type_; }

    // Tables of flight path selection parameters
    ArrayNDf mfp() const { return mfp_; }
    ArrayNDf ipmax() const { return ipmax_; }
    ArrayNDf fpmax() const { return fp_max_; }
    ArrayNDf Tcutoff() const { return Tcutoff_; }

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
    bool operator()(random_vars &rng, float E, float &fp, float &sqrtfp, float &ip) const
    {
        constexpr const float mhw_umin = 1. / M_E;

        bool doCollision = true;
        int ie;
        float u = rng.u01s_open();

        switch (type_) {
        case AtomicSpacing:
            fp = fp_;
            sqrtfp = 1;
            ip = ip_ * std::sqrt(u);
            break;
        case Constant:
            fp = fp_;
            sqrtfp = sqrtfp_;
            ip = ip_ * std::sqrt(u);
            break;
        case MendenhallWeller:
            ie = dedx_index(E);
            ip = ipmax_tbl[ie];
            fp = mfp_tbl[ie];
            if (ip < ip_) {
                sqrtfp = std::sqrt(fp / fp_);
                doCollision = u >= mhw_umin;
                if (doCollision)
                    ip *= std::sqrt(-std::log(u));
            } else { // atomic spacing
                fp = fp_;
                sqrtfp = 1;
                ip = ip_ * std::sqrt(u);
            }
            break;
        case FullMC:
            ie = dedx_index(E);
            doCollision = u >= umin_tbl[ie];
            if (doCollision) {
                fp = mfp_tbl[ie] * (-std::log(u));
                ip = ipmax_tbl[ie] * std::sqrt(rng.u01s_lopen());
            } else {
                fp = fpmax_tbl[ie];
            }
            sqrtfp = std::sqrt(fp / fp_);
            break;
        default:
            assert(false); // never get here
        }
        assert(fp > 0);
        assert(finite(fp));
        return doCollision;
    }

protected:
    flight_path_type_t type_;

    // helper variables for flight path calc
    std::vector<float> fp_const;
    std::vector<float> sqrtfp_const;
    std::vector<float> ip0;

    // flight path selection par
    ArrayNDf mfp_, ipmax_, fp_max_, Tcutoff_, umin_;

    // Flight path [nm]
    float fp_;
    // Square root of ratio (flight path)/(atomic radius) (used for straggling)
    float sqrtfp_;
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

#endif // FLIGHT_PATH_CALC_H
