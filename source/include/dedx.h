#ifndef _DEDX_H_
#define _DEDX_H_

#include <libdedx.h>

#include "corteo.h"
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
 * @{
 *
 * The stopping tables have been compiled with the program
 * `SRmodule.exe`
 * distributed with SRIM-2013 (http://www.srim.org).
 *
 * \f$dE/dx\f$ is given as a function of ion energy on a log-spaced corteo range
 * defined by \ref dedx_index.
 *
 * Tables are provided for all projectile (\f$Z=Z_1\f$) / target (\f$Z=Z_2\f$) compinations with \f$
 * 1 \leq Z_1, Z_2 \leq 92\f$
 *
 * The data are compiled into a dynamic library (libdedx.so or .dll).
 *
 * The \ref dedx_interp interpolator object can be used for stopping
 * calculations in mono- and polyatomic materials. The stopping at a given
 * ion energy is obtained by log-log interpolation.
 *
 * \ref straggling_interp is a similar interpolator class for calculating
 * energy straggling.
 *
 * Direct access to the stopping tables is provided by the function \ref raw_dedx().
 *
 * @}
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
 * The range is implemented as a 4-bit corteo::index, providing a
 * log-spaced energy table with fast access.
 *
 * The energy range in [eV], \f$ 2^4 = 16 \leq E \leq 2^{30} \sim 10^9 \f$, is
 * divided in (30-4)*2^4 = 416 approx. log-spaced intervals.
 *
 * @ingroup dedx
 *
 */
typedef corteo::index<float, int, 4, 4, 30> dedx_erange;

/**
 * @brief Return a table of electronic stopping for a projectile (atomic number Z1) moving inside a
 * target (atomic number Z2)
 *
 * Stopping data is calculated at log-spaced ion energy values given by \ref dedx_index.
 *
 * The tables have been generated for all \f$ (Z_1,Z_2) \f$ combinations,
 * with \f$  1 \leq Z_{i=1,2} \leq 92 \f$,
 * using the free program `SRModule.exe` provided by SRIM-2013.
 *
 * The energy loss is in units of eV / [10^15 at/cm^2] = 10^-15 eV-cm^2.
 *
 * Multiply by the target atomic density to obtain the stopping power dE/dx in eV/cm.
 *
 * @param Z1 projectile atomic number
 * @param Z2 target atomic number
 * @return pointer to the first data point in the table
 *
 * @ingroup dedx
 */
const float *raw_dedx(int Z1, int Z2);

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
class dedx_interp : public corteo::log_interp<dedx_erange>
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
 * Call the base class log_interp::operator() to obtain the straggling coefficient
 * \f$\Omega(E)\f$ in eV/nm^(1/2) at a given
 * projectile energy \f$E\f$ in eV. The value is obtained by log-log interpolation
 * on the tabulated data stored internally.
 *
 * data() returns a pointer to the first element in the internal
 * straggling data table.
 *
 * @ingroup dedx
 */
class straggling_interp : public corteo::log_interp<dedx_erange>
{
    int init(StoppingModel mstop, StragglingModel mstrag, int Z1, float M1,
             const std::vector<int> &Z2, const std::vector<float> &X2, float atomicDensity);

public:
    /**
     * @brief Construct an interpolator for monoatomic targets
     * @param model the \ref StragglingModel to apply
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
     * @param model The \ref StragglingModel to apply
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
 * @brief The dedx_calc class is used for electronic energy loss and straggling calculations
 *
 * The class stores pre-computed interpolation tables for all ion/material combinations
 * present in a given simulation. These tables are generated initially with a call to init(const
 * mccore& s).
 *
 * During the simulation, init(const ion* i, const material* m) should be
 * called each time a new ion/material combination arises, to preload the
 * required tables for max efficiency.
 *
 * Then, the energy loss of the ion for a given flight path is effected by calling
 * operator() on the dedx_calc object.
 *
 *
 * \ingroup dedx
 *
 */
class dedx_calc
{
public:
    /**
     * @brief Determines electronic energy loss calculation mode
     */
    enum eloss_calculation_t {
        EnergyLossOff = 0, /**< Electronic energy loss disabled */
        EnergyLoss = 1, /**< Only electronic energy loss is calculated */
        EnergyLossAndStraggling = 2, /**< Both energy loss & straggling are calculated */
        InvalidEnergyLoss = -1
    };

    dedx_calc();
    dedx_calc(const dedx_calc &other);
    ~dedx_calc();

    eloss_calculation_t type() const { return type_; }

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
     * call ton init(const ion* i, const material* m).
     *
     * @param i pointer to an \ref ion object
     * @param fp the ion's flight path [nm]
     * @param rng random number generator (used for straggling)
     * @return  the total energy loss [eV]
     */
    void operator()(ion *i, float fp, random_vars &rng) const
    {
        if (type_ == EnergyLossOff)
            return;

        float E = i->erg();
        float de_stopping = fp * (*stopping_interp_)(E);

        if (type_ == EnergyLossAndStraggling) {
            dedx_erange ie(E);
            float de_straggling = straggling_interp_->data()[ie] * rng.normal() * std::sqrt(fp);

            /* IRADINA
             * Due to gaussian distribution, the straggling can in some cases
             * get so large that the projectile gains energy or suddenly looses
             * a huge amount of energy. Is this realistic? This might actually
             * happen. However, in the simulation, ions may have higher energy
             * than the initial energy.
             * We will therefore limit the |straggling| to |stopping|.
             * Furthermore, with hydrogen, the straggling is often so big,
             * that ions gain huge amount of energy, and the phononic system
             * would actually gain energy.
             */
            if (std::abs(de_straggling) > de_stopping)
                de_straggling = (de_straggling < 0) ? -de_stopping : de_stopping;

            de_stopping += de_straggling;
        }

        /* IRADINA
         * The stopping tables have no values below minVal = 16 eV.
         * Therefore, we do sqrt downscaling of electronic
         * stopping below 16 eV.
         */
        if (E < dedx_erange::minVal)
            de_stopping *= std::sqrt(E / dedx_erange::minVal);

        /*
         * This is due to some rare low-energy events (ion E<100 eV)
         * with IPP flight selection were
         * a long flight path + large straggling can cause the
         * stopping + straggling > E
         *
         * The code below changes that to almost stopping the ion,
         * E - stopping ~ 0
         */
        if (de_stopping > E)
            de_stopping = 0.99 * E;

        i->de_ioniz(de_stopping);
    }

    void operator()(ion *i, float fp) const
    {
        if (type_ == EnergyLossOff)
            return;

        float E = i->erg();
        float de_stopping = fp * (*stopping_interp_)(E);

        /* IRADINA
         * The stopping tables have no values below minVal = 16 eV.
         * Therefore, we do sqrt downscaling of electronic
         * stopping below 16 eV.
         */
        if (E < dedx_erange::minVal)
            de_stopping *= std::sqrt(E / dedx_erange::minVal);

        /*
         * This is due to some rare low-energy events (ion E<100 eV)
         * with IPP flight selection were
         * a long flight path + large straggling can cause the
         * stopping + straggling > E
         *
         * The code below changes that to almost stopping the ion,
         * E - stopping ~ 0
         */
        if (de_stopping > E)
            de_stopping = 0.99 * E;

        i->de_ioniz(de_stopping);
    }

protected:
    eloss_calculation_t type_;

    // Electronic Stopping & Straggling Tables
    ArrayND<dedx_interp *> dedx_; // stopping data (atoms x materials)
    ArrayND<straggling_interp *> de_strag_; // straggling data (atoms x materials)

    const dedx_interp *stopping_interp_;
    const straggling_interp *straggling_interp_;
};

#endif // DEDX_H
