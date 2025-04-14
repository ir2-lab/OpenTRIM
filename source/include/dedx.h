#ifndef _DEDX_H_
#define _DEDX_H_

#include "corteo.h"

#include <vector>

/**
 * \defgroup dedx libdedx shared library
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
 * @brief A 4-bit corteo::index for tables of ion stopping, \f$dE/dx\f$
 *
 * This index provides fast access to a log-spaced table of ion energy values.
 * They are intended for indexing interpolation tables of ion stopping.
 *
 * The number of tabulated values is (30-4)*2^4 + 1 = 417,
 * in the range \f$ 2^4 = 16 \leq E \leq 2^{30} \sim 10^9 \f$ in [eV].
 *
 * @ingroup dedx
 *
 */
typedef corteo::index<float, int, 4, 4, 30> dedx_index;

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

constexpr int dedx_max_Z{ 92 };

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
 * power in eV/nm at a given projectile energy in eV. The value is obtained by log-log interpolation
 * on the tabulated data stored internally.
 *
 * data() returns a pointer to the first element in the internal
 * energy loss data table.
 *
 * Raw stopping data is obtained internally by calling raw_dedx().
 *
 * @ingroup dedx
 */
class dedx_interp : public corteo::log_interp<dedx_index>
{
    int init(int Z1, float M1, const std::vector<int> &Z2, const std::vector<float> &X2,
             float atomicDensity);

public:
    /**
     * @brief Construct an interpolator for monoatomic targets
     * @param Z1 projectile atomic number
     * @param M1 projectile atomic mass
     * @param Z2 target atom atomic number
     * @param N target atomic density in [at/nm3]
     */
    dedx_interp(int Z1, float M1, int Z2, float N = 1.f);
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
    dedx_interp(int Z1, float M1, const std::vector<int> &Z2, const std::vector<float> &X2,
                float N = 1);
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
class straggling_interp : public corteo::log_interp<dedx_index>
{
    int init(StragglingModel model, int Z1, float M1, const std::vector<int> &Z2,
             const std::vector<float> &X2, float atomicDensity);

public:
    /**
     * @brief Construct an interpolator for monoatomic targets
     * @param model the \ref StragglingModel to apply
     * @param Z1 projectile atomic number
     * @param M1 projectile atomic mass
     * @param Z2 target atom atomic number
     * @param N target atomic density in [at/nm3]
     */
    straggling_interp(StragglingModel model, int Z1, float M1, int Z2, float N = 1.f);
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
    straggling_interp(StragglingModel model, int Z1, float M1, const std::vector<int> &Z2,
                      const std::vector<float> &X2, float N = 1);
};

#endif // DEDX_H
