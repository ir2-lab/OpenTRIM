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
 * Finally, the \ref IPP algorithm does the following:
 * - \f$\ell = -\ell_0(E) \log{u_1}\f$
 * - \f$p = p_{max}(E)\sqrt{u_2}\f$
 * - Reject collision if \f$\ell > \ell_{max}(E)\f$
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

    /// Initialize object
    int init(const mccore &s);

    /// Init object for a specific ion/material combination
    int init(const ion *i, const material *m);

    /**
     * @brief Select the ion's flight path and impact parameter
     *
     * The selection is performed according to the algorithm specified by
     * transport_options::flight_path_type, which can be any of the types defined by
     * the mccore::flight_path_type_t enum.
     *
     * If transport_options::flight_path_type is equal to \ref AtomicSpacing or \ref Constant
     * then the flight path \f$\ell\f$ is precalculated and equal to either the material's atomic
     * radius \f$ R_{at} = \left(\frac{3}{4\pi}N\right)^{1/3} \f$ or to \ref
     * parameters::flight_path_const, respectively.
     *
     * In both of these cases the impact parameter \f$p\f$ is calculated as
     * $$
     * p = (\pi \ell N)^{-1/2}\sqrt{u},
     * $$
     * where \f$u \in (0,1)\f$ is a random number.
     *
     * For the other algorithms there are precomputed tables as a function of energy
     * of mean free path \$\ell_0\f$, max. impact parameter \f$p_{max}\f$ and
     * maximum fligth path \$\ell_{max}\f$. For more information see \ref flightpath.
     *
     * The \ref MendenhallWeller algorithm is as follows:
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
     * Finally, the \ref IPP algorithm does the following:
     * - \f$\ell = -\ell_0(E) \log{u_1}\f$
     * - \f$p = p_{max}(E)\sqrt{u_2}\f$
     * - Reject collision if \f$\ell > \ell_{max}(E)\f$
     *
     * @param[in] i pointer to the moving ion object
     * @param[in] m pointer to the material the ion is in
     * @param[inout] d struct to store the selected flight path and impact parameter
     * @return true if the ion should collide at the end of its flight path
     *
     * @sa \ref flightpath
     */
    bool operator()(random_vars &rng, float E, float &fp, float &sqrtfp, float &ip) const
    {
        bool doCollision = true;
        int ie;

        switch (type_) {
        case AtomicSpacing:
            fp = fp_;
            sqrtfp = 1;
            ip = ip_ * std::sqrt(rng.u01d_lopen());
            break;
        case Constant:
            fp = fp_;
            sqrtfp = sqrtfp_;
            ip = ip_ * std::sqrt(rng.u01d_lopen());
            break;
        case MendenhallWeller:
            ie = dedx_index(E);
            ip = ipmax_tbl[ie];
            fp = mfp_tbl[ie];
            if (ip < ip_) {
                sqrtfp = std::sqrt(fp / fp_);
                ip *= std::sqrt(-std::log(rng.u01s_open()));
                doCollision = (ip <= ipmax_tbl[ie]);
            } else { // atomic spacing
                fp = fp_;
                sqrtfp = 1;
                ip = ip_ * std::sqrt(rng.u01d_lopen());
            }
            break;
        case FullMC:
            ie = dedx_index(E);
            fp = mfp_tbl[ie] * (-std::log(rng.u01s_open()));
            doCollision = fp <= fpmax_tbl[ie];
            if (doCollision)
                ip = ipmax_tbl[ie] * std::sqrt(rng.u01d_lopen());
            else
                fp = fpmax_tbl[ie];
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
    ArrayNDf mfp_, ipmax_, fp_max_, Tcutoff_;

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
};

#endif // FLIGHT_PATH_CALC_H
