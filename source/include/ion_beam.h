#ifndef ION_BEAM_H
#define ION_BEAM_H

#include "geometry.h"
#include "ion.h"

class target;
class atom;
class material;
class random_vars;

/**
 * @brief The ion_beam class is used for ejecting ions into the simulation
 *
 * The ions are generated according to the spatial, direction and energy distributions
 * specified by the class \ref parameters.
 *
 * @ingroup Ions
 */
class ion_beam
{
public:
    /**
     * @brief Types of source parameter distributions
     */
    enum distribution_t {
        SingleValue = 0, /**< Single valued (=delta) distribution */
        Uniform, /**< Uniform distribution */
        Gaussian, /**< Gaussian (Normal) distribution */
        InvalidDistribution = -1
    };

    /**
     * @brief Ion energy distribution
     */
    struct energy_distribution_t
    {
        /// Energy distribution type. Default: SingleValue
        distribution_t type{ SingleValue };
        /// Center position of generated ions [eV]. Default: 1 MeV.
        float center{ 1e6f };
        /// Full-width at half-maximum of ion energy distribution [nm]. Default: 1 eV.
        float fwhm{ 1.0f };
        /// Draw a random energy sample from the distribution
        float sample(random_vars &r) const;
        void init();
        float a, b;
    };

    /**
     * @brief Type of ion beam source geometry
     *
     */
    enum geometry_t {
        Surface = 0, /**< Surface source */
        Volume = 1, /**< Volume source */
        InvalidGeometry = -1
    };

    /**
     * @brief Ion spatial distribution
     */
    struct spatial_distribution_t
    {
        /// Source geometry type
        geometry_t geometry{ Surface };
        /// Spatial distribution type
        distribution_t type{ SingleValue };
        /// Full-width at half-maximum of ion spatial distribution [nm]
        float fwhm{ 1.0f };
        /// Center position of generated ions [nm]. Default (0,0,0)
        vector3 center{ 0, 0, 0 };

        void sample(random_vars &g, const target &t, vector3 &pos) const;
        void init(const target &t);

    private:
        vector3 a, b;
        float sig;
    };

    /**
     * @brief Ion angular distribution
     */
    struct angular_distribution_t
    {
        /// Angular distribution type. Default: SingleValue
        distribution_t type{ SingleValue };
        /// Cental direction of generated ions. Unnormalized vector, default: (1,0,0).
        vector3 center{ 1, 0, 0 };
        /// Full-width at half-maximum of ion angular distribution [srad]. Default: 0.1.
        float fwhm{ 1.0f };
        void sample(random_vars &g, const target &t, vector3 &dir) const;
        void init(const target &t);
        float mu;
        vector3 norm_center;
    };

    /**
     * @brief The parameters of the ion_beam class
     */
    struct parameters
    {
        /// Atomic type of the generated ions
        element_t ion{ "H", 1, 1.007825f }; // initialize to proton
        /// Energy distribution of the generated ions
        energy_distribution_t energy_distribution;
        /// Spatial distribution of the generated ions
        spatial_distribution_t spatial_distribution;
        /// Angular distribution of the generated ions
        angular_distribution_t angular_distribution;
    };

protected:
    parameters par_;

    bool is_pka_source_;
    const material *pka_source_material_;

public:
    /// Default constructor
    ion_beam();
    /// Copy constructor
    ion_beam(const ion_beam &i);

    /// Set ion_beam class parameter values
    void setParameters(const parameters &p);
    /// Returns the ion_beam parameters
    const parameters &getParameters() const { return par_; }
    /// Initialize the ion_beam class
    void init(target &t, bool is_cascade_sim);

    /**
     * @brief Generate an ion in the simulation
     * @param g the random number engine
     * @param t the simulation target
     * @param i the generated ion
     */
    void source_ion(random_vars &g, const target &t, ion &i);
};

inline void shift_left(vector3 &v)
{
    float d = v.x();
    v.x() = v.y();
    v.y() = v.z();
    v.z() = d;
}

inline void shift_right(vector3 &v)
{
    float d = v.z();
    v.z() = v.y();
    v.y() = v.x();
    v.x() = d;
}

#endif // ION_BEAM_H
