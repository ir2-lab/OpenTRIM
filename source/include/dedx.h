#ifndef _DEDX_H_
#define _DEDX_H_

#include <libdedx.h>
#include <ieee754_seq.h>
#include "arrays.h"
#include "random_vars.h"
#include "ion.h"

class ion;
class material;
class mccore;

/**
 * \defgroup dedx Electronic stopping
 *
 * @brief Classes for calculating ion electronic stopping and straggling
 *
 *
 *
 * Electronic stopping cross-section data are taken from the `libdedx` project
 * (https://github.com/ir2-lab/libdedx), which is a compilation of the following parametrizations:
 * - SRIM version 1996
 * - SRIM version 2013
 * - DPASS version 21.06
 *
 * Tables are provided for all projectile (\f$Z=Z_1\f$) / target (\f$Z=Z_2\f$) combinations with \f$
 * 1 \leq Z_1, Z_2 \leq 92\f$.
 *
 * The parametrization to be used by OpenTRIM is specified by the \ref StoppingModel enum.
 *
 * Stopping data are interpolated from a log-spaced energy range defined by \ref dedx_erange.
 *
 * The \ref dedx_interp interpolator object can be used for stopping
 * calculations in mono- and polyatomic materials. The stopping at a given
 * ion energy is obtained by log-log interpolation.
 *
 * \ref straggling_interp is a similar interpolator class for calculating
 * energy straggling.
 *
 *
 *
 */

/**
 * @brief The parametrization/model used to calculate electronic stopping of ions
 *
 *  @ingroup dedx
 */
enum class StoppingModel {
    SRIM96 = SM_SRIM96, /**< SRIM version 1996  */
    SRIM13 = SM_SRIM13, /**< SRIM version 2013 */
    DPASS22 = SM_DPASS22, /**< DPASS version 21.06 */
    Invalid = -1
};

/**
 * @brief Energy range for interpolation tables of electronic ion stopping
 *
 * It is a 4-bit `ieee754_seq`, providing a fast access
 * log-spaced energy grid.
 *
 * The energy range in [eV], \f$ 2^4 = 16 \leq E \leq 2^{30} \sim 10^9 \f$, is
 * divided in 416 log-spaced intervals.
 *
 * @ingroup dedx
 *
 * @sa https://github.com/ir2-lab/ieee754_seq
 *
 */
typedef ieee754_seq<float, 4, 4, 30> dedx_erange;

typedef dedx_erange::iterator dedx_iterator;

/**
 * @brief Interpolator class for ion electronic stopping calculations
 *
 * Initialize the class with the projectile's atomic number and mass
 * and the composition and atomic density of the target.
 *
 * Monoatomic and polyatomic targets are covered by
 * the two different constructors. In polyatomic materials the Bragg
 * mixing rule is applied.
 *
 * Call the base class log_interp::operator() to obtain the stopping
 * power in eV/nm at a given projectile energy in eV.
 *
 * The value is obtained by
 * log-log interpolation
 * on tabulated data from libdedx (https://github.com/ir2-lab/libdedx).
 *
 * data() returns a pointer to the first element in the internal
 * energy loss data table.
 *
 * @ingroup dedx
 */
class dedx_interp : public dedx_erange::log_log_interp
{
    int init(StoppingModel m, int Z1, float M1, const std::vector<int> &Z2,
             const std::vector<float> &X2, float atomicDensity);

public:
    constexpr static const int Zmax = ZMAX;

    /**
     * @brief Construct an interpolator for monoatomic targets
     *
     * The average atomic mass is used for both projectile and target.
     *
     * @param m Electronic stopping model
     * @param Z1 projectile atomic number
     * @param Z2 target atom atomic number
     * @param N target atomic density in [at/nm3]
     */
    dedx_interp(StoppingModel m, int Z1, int Z2, float N = 1.f);
    /**
     * @brief Construct an interpolator for monoatomic targets
     *
     * The average atomic mass is used for the target atom.
     *
     * @param m Electronic stopping model
     * @param Z1 projectile atomic number
     * @param M1 projectile atomic mass
     * @param Z2 target atom atomic number
     * @param N target atomic density in [at/nm3]
     */
    dedx_interp(StoppingModel m, int Z1, float M1, int Z2, float N = 1.f);
    /**
     * @brief Construct an interpolator for polyatomic targets
     *
     * The total stopping power is given by the Bragg mixing rule:
     * \f[
     * dE/dx = N\sum_i{X_i (dE/dx)_i}
     * \f]
     * where the sum is over all atomic species in the target.
     *
     * The average atomic mass is used for both projectile and target.
     *
     * @param m Electronic stopping model
     * @param Z1 projectile atomic number
     * @param Z2 vector of target atom atomic numbers
     * @param X2 vector of target atomic fractions (sum of X2 assumed equal to 1.0)
     * @param N target atomic density in [at/nm3]
     */
    dedx_interp(StoppingModel m, int Z1, const std::vector<int> &Z2, const std::vector<float> &X2,
                float N = 1);
    /**
     * @brief Construct an interpolator for polyatomic targets
     *
     * The total stopping power is given by the Bragg mixing rule:
     * \f[
     * dE/dx = N\sum_i{X_i (dE/dx)_i}
     * \f]
     * where the sum is over all atomic species in the target.
     *
     * @param m Electronic stopping model
     * @param Z1 projectile atomic number
     * @param M1 projectile atomic mass
     * @param Z2 vector of target atom atomic numbers
     * @param X2 vector of target atomic fractions (sum of X2 assumed equal to 1.0)
     * @param N target atomic density in [at/nm3]
     */
    dedx_interp(StoppingModel m, int Z1, float M1, const std::vector<int> &Z2,
                const std::vector<float> &X2, float N = 1);
};

/**
 * @brief The model used to calculate electronic energy straggling of ions
 *
 * The implementation of the different models is from Yang et al (1991) NIMB
 *
 *  @ingroup dedx
 */
enum class StragglingModel {
    Bohr = 0, /**< Bohr straggling model */
    Chu = 1, /**< Chu straggling model */
    Yang = 2, /**< Yang straggling model */
    Invalid = -1
};

/**
 * @brief Interpolator class for electronic energy straggling calculations
 *
 * Initialize the class with the projectile's atomic number and mass
 * and the target composition and atomic density.
 *
 * Monoatomic and polyatomic targets are covered by
 * the two different constructors. In polyatomic materials the Bragg
 * mixing rule is applied.
 *
 * Call the base class \ref log_interp::operator() to obtain the straggling coefficient
 * \f$\Omega(E)\f$ in eV/nm^(1/2) at a given
 * projectile energy \f$E\f$ in eV. The value is obtained by log-log interpolation
 * on the tabulated data stored internally.
 *
 * data() returns a pointer to the first element in the internal
 * straggling data table.
 *
 * @ingroup dedx
 */
class straggling_interp : public dedx_erange::log_log_interp
{
    int init(StoppingModel mstop, StragglingModel mstrag, int Z1, float M1,
             const std::vector<int> &Z2, const std::vector<float> &X2, float atomicDensity);

public:
    /**
     * @brief Construct an interpolator for monoatomic targets
     * @param mstop the \ref StoppingModel to apply
     * @param mstrag the \ref StragglingModel to apply
     * @param Z1 projectile atomic number
     * @param M1 projectile atomic mass
     * @param Z2 target atom atomic number
     * @param N target atomic density in [at/nm3]
     */
    straggling_interp(StoppingModel mstop, StragglingModel mstrag, int Z1, float M1, int Z2,
                      float N = 1.f);
    /**
     * @brief Construct an interpolator for polyatomic targets
     *
     * The total straggling is given by the Bragg mixing rule:
     * \f[
     * \Omega^2 = N\sum_i{X_i \Omega_i^2}
     * \f]
     * where the sum is over all atomic species in the target.
     *
     * @param mstop the \ref StoppingModel to apply
     * @param mstrag the \ref StragglingModel to apply
     * @param Z1 projectile atomic number
     * @param M1 projectile atomic mass
     * @param Z2 vector of target atom atomic numbers
     * @param X2 vector of target atomic fractions (sum of X2 assumed equal to 1.0)
     * @param N target atomic density in [at/nm3]
     */
    straggling_interp(StoppingModel mstop, StragglingModel mstrag, int Z1, float M1,
                      const std::vector<int> &Z2, const std::vector<float> &X2, float N = 1);
};

/**
 * @brief Electronic energy loss and straggling calculator used in OpenTRIM simulations
 *
 * The class stores pre-computed interpolation tables for all ion/material combinations
 * present in a given simulation. These tables are generated initially with a call to init(const
 * mccore& s).
 *
 * During the simulation, preload(const ion* i, const material* m) should be
 * called each time a new ion/material combination arises, to preload the
 * required tables for max efficiency.
 *
 * dedx_calc::operator() implements energy loss & stragling for a given ion flight path.
 *
 * \ingroup dedx
 *
 */
class dedx_calc
{
public:
    enum class electronic_stopping_t {
        Off = 3,
        SRIM96 = int(StoppingModel::SRIM96),
        SRIM13 = int(StoppingModel::SRIM13),
        DPASS = int(StoppingModel::DPASS22),
        Invalid = -1
    };

    enum class electronic_straggling_t {
        Off = 3,
        Bohr = int(StragglingModel::Bohr),
        Chu = int(StragglingModel::Chu),
        Yang = int(StragglingModel::Yang),
        Invalid = -1
    };

    dedx_calc();
    dedx_calc(const dedx_calc &other);
    ~dedx_calc();

    electronic_stopping_t stopping() const { return stopping_; }
    electronic_straggling_t straggling() const { return straggling_; }

    // dedx interpolators
    ArrayND<dedx_interp *> dedx() const { return dedx_; }
    ArrayND<straggling_interp *> de_strag() const { return de_strag_; }

    /**
     * @brief Initialize the object for a given simulation \a s
     *
     * Computes the interpolation tables for all ion/material
     * combinations required by the given simulation.
     *
     * Should be called once after the projectile and all target materials
     * have been defined in \a s and before starting the transport
     * simulation.
     *
     * @param s a reference to an \ref mccore object
     * @return 0 if succesfull
     */
    int init(const mccore &s);

    /**
     * @brief Preload interpolation tables for a given ion/material combination
     *
     * This function should be called each time a new ion/material combination
     * is needed during the simulation. This could be, e.g., when a new recoil
     * is created, when an ion crosses the boundary between two materials.
     *
     * @param i pointer to an \ref ion object
     * @param m pointer to a \ref material object
     * @return 0 if succesfull
     */
    int preload(const ion *i, const material *m);

    /**
     * @brief Calculate and subtract electronic energy loss and straggling of a moving ion
     *
     * The calculation is based on interpolation tables preloaded for a specific
     * ion/material combination by a previous
     * call to preload()
     *
     * @param i pointer to an \ref ion object
     * @param fp the ion's flight path [nm]
     * @param rng random number generator (used for straggling)
     *
     */
    void operator()(ion &i, float fp, random_vars &rng) const
    {
        if (stopping_ == electronic_stopping_t::Off)
            return;

        float de = __get_de_stopping__(i, fp);

        if (straggling_ != electronic_straggling_t::Off) {
            float E = i.erg();
            dedx_iterator ie(E);
            de += straggling_interp_->data()[ie] * rng.normal() * std::sqrt(fp);
        }

        __impl_de__(i, de);
    }

    /**
     * @brief Calculate and subtract electronic energy loss for a moving ion
     *
     * The calculation is based on interpolation tables preloaded for a specific
     * ion/material combination by a previous
     * call to preload().
     *
     * @param i the \ref ion object
     * @param fp the ion's flight path [nm]
     *
     */
    void operator()(ion &i, float fp) const
    {
        if (stopping_ == electronic_stopping_t::Off)
            return;

        float de_stopping = __get_de_stopping__(i, fp);

        __impl_de__(i, de_stopping);
    }

protected:
    electronic_stopping_t stopping_;
    electronic_straggling_t straggling_;

    // Electronic Stopping & Straggling Tables
    ArrayND<dedx_interp *> dedx_; // stopping data (atoms x materials)
    ArrayND<straggling_interp *> de_strag_; // straggling data (atoms x materials)

    const dedx_interp *stopping_interp_;
    const straggling_interp *straggling_interp_;

    // implement ion energy reduction
    static void __impl_de__(ion &i, float de)
    {
        float E = i.erg();

        // For the rare events that ΔE > E, set ΔΕ slightly below E
        // so that the ion will stop
        if (de > E) {
            de = E;
            // avoid E-ΔE < 0 due to round-off
            constexpr static const float delta = 1e-3f;
            de -= (E > 2 * delta) ? delta : 0.5 * E;
        }

        i.de_ioniz(de);
    }

    // get stopping ΔΕ
    float __get_de_stopping__(ion &i, float fp) const
    {
        float E = i.erg();
        float de = fp * (*stopping_interp_)(E);

        // For E below the interpolation range scale with sqrt(E)
        if (E < dedx_erange::min)
            de *= std::sqrt(E / dedx_erange::min);

        return de;
    }
};

#endif // DEDX_H
