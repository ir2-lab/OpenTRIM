#ifndef _TARGET_H_
#define _TARGET_H_

#include "geometry.h"
#include "arrays.h"
#include "elements.h"

#include <string>
#include <vector>

class target;
class material;
class random_vars;

/**
 * \defgroup TargetG Target
 *
 * @brief A set of classes for defining the properties, composition and
 * spatial configuration of the target.
 *
 * @{
 *
 * The simulation volume is a rectangular 3D box.
 *
 * A 3D grid divides the volume into rectangular cells. The grid is described by
 * a \ref grid3D object, which can be accesed with the target::grid() function.
 *
 * Each cell can be filled with a material, which is random mixture of atoms
 * at specific density. A cell can also be empty.
 *
 * Materials and atoms are described by the corresponding classes \ref material
 * and \ref atom, respectively.
 *
 * In order to create a specific target configuration, we fill
 * rectangular regions of the simulation volume with a specific material.
 * Such regions are described with the \ref target::region structure.
 *
 *
 * The \ref target class keeps a track of all regions, materials and atoms in the
 * simulation target.
 *
 * @ingroup MC
 *
 * @}
 *
 */

class target;

/**
 * @brief target_item is the base class for all objects related to the target
 * @ingroup TargetG
 */
class target_item {
protected:
    target* target_;
public:
    target_item(target* t) : target_(t)
    {}
};

/**
 * @brief The atom class represents an atomic species in the target or in the ion beam.
 * @ingroup TargetG
 */
class atom : public target_item
{

public:

    /**
     * @brief The atom class parameters
     */
    struct parameters {
        /// atomic number
        int Z;
        /// mass of the atom
        float M;
        /// Displacement threshold energy (eV) of target atoms
        float Ed;
        /// Lattice energy (eV) of target atoms
        float El;
        /// Surface binding energy (eV) of target atoms
        float Es;
        /// Replacement threshold energy (eV) of target atoms
        float Er;
    };

private:

    int id_;
    material* mat_;
    parameters p_;
    float X_; // proxy concentration

    // LSS & NRT
    float lss_Kd_;
    float lss_Efact_;
    float nrt_L_;

public:
    explicit atom(const parameters& p);

    /**
     * @brief Return the unique id of this atom
     *
     * As atoms are added to the target by calling material::addAtom()
     * they are registered on a table and given a unique, sequential id number.
     * Thus, we can distinguish
     * between atoms of the same element but in different materials.
     *
     * E.g., a target may be composed of Al2O3 and ZrO layers. Oxygen atoms in
     * Al2O3 have a different id from O's in ZrO. This allows, e.g., to assign
     * a different displacement threshold for O in these two materials.
     *
     * Furthermore, most tallies have separate columns for each atom id.
     * This way one can distinguish, e.g.,
     * in the previous example,
     * how much ionization energy comes by O recoils specifically from ZrO.
     *
     * The special id value of 0 is reserved for the atomic species of the ion beam.
     *
     * @return the atom's unique id number
     */
    int id() const { return id_; }
    /// A pointer to the target material this atom belongs to. For the beam atom nullptr is returned.
    const material* mat() const { return mat_; }
    /// Returns the chemical name of the atom
    const char* name() const { return elements::name(p_.Z); }
    /// Returns the atomic number
    int Z() const { return p_.Z; }
    /// Returns the atomic mass
    float M() const { return p_.M; }
    /// Returns the concentration of this atom in the target material
    float X() const { return X_; }
    /// Returns the displacement threshold energy (eV) of target atoms
    float Ed() const { return p_.Ed; }
    /// Returns the lattice binding energy (eV) of target atoms
    float El() const { return p_.El; }
    /// Returns the surface binding energy (eV) of target atoms
    float Es() const { return p_.Es; }
    /// Returns the replacement threshold energy (eV) of target atoms
    float Er() const { return p_.Er; }
    /// Returns true if this atom's Z and M are equal to Z and M of other
    bool operator==(const atom& other) const
    { return (p_.Z == other.p_.Z) && (p_.M == other.p_.M); }
    /// Returns the damage energy [eV] corresponding to recoil energy T according to LSS approx.
    float LSS_Tdam(float T) const;
    /// Returns the number of vacancies created by a recoil of damage energy Tdam according to Norgett-Roninson-Torrens model
    float NRT(float Tdam) const;

private:
    atom();
    atom(target* t, material* m, int id);

    friend class target;
    friend class material;
};

/**
 * @brief The material class represents a material in the target
 * @ingroup TargetG
 */
class material : public target_item
{

    int id_;
    std::string name_;
    float massDensity_; // gr/cm^3

    std::vector<atom*> atoms_;
    std::vector<float> X_;
    std::vector<float> cumX_;

    float atomicRadius_; // nm
    float sqrtAtomicDistance_;
    float layerDistance_;

    // float atomicDensity_; // at/cm^3 ??
    float atomicDensityNM_; // at/nm^3
    float sqrtRecFlDensity_;
    float meanZ_;
    float meanM_;

    // SRIM like stuff
    float meanF_;
    float meanA_;
    float meanMinRedTransfer_;
    float meanImpactPar_;

    // LSS & NRT
    float lss_Kd_;
    float lss_Efact_;
    float nrt_Ed_;
    float nrt_L_;

public:

    struct material_desc_t {
        std::string id;
        float density{1.f};
        bool isMassDensity{true};
        std::vector<int> Z;
        std::vector<float> M;
        std::vector<float> X;
        std::vector<float> Ed;
        std::vector<float> El;
        std::vector<float> Es;
        std::vector<float> Er;
    };

    explicit material(const char* name);

    /**
     * @brief Set atomic density of the material
     *
     * Invalidates other previous density setting
     *
     * @param v density in [at/nm^3]
     */
    void setAtomicDensity(float v) { atomicDensityNM_ = v; massDensity_ = -1; }
    /**
     * @brief Set mass density of the material
     *
     * Invalidates other previous density setting
     *
     * @param v density in [g/cm^3]
     */
    void setMassDensity(float v) { massDensity_ = v; atomicDensityNM_ = -1; }
    /// Returns the name of the material
    const std::string& name() const { return name_; }
    /// Returns the atomic density \f$ N \f$ [nm^-3]
    float atomicDensity() const { return atomicDensityNM_; }
    /// Returns the mass density [g/cm^3]
    float massDensity() const { return massDensity_; }
    /// Returns the atomic radius \f$ (4\pi N/3)^{-1/3} \f$ [nm]
    float atomicRadius() const { return atomicRadius_; }
    /// Returns the monolayer distance \f$ N^{-1/3} \f$ [nm]
    float layerDistance() const { return layerDistance_; }

    // the following are used for SRIM ffp algorithm
    float meanF() const { return meanF_; }
    float meanA() const { return meanA_; }
    float meanMinRedTransfer() const { return meanMinRedTransfer_; }
    float meanImpactPar() const { return meanImpactPar_; }
    float sqrtRecFlDensity() const { return sqrtRecFlDensity_; }

    // this is used for generating options structure
    material_desc_t getDescription() const;

    /**
     * @brief addAtom adds an atom to the material
     * @param p atomic species parameters
     * @param x concentration
     * @return a pointer to the created atom object
     */
    atom* addAtom(const atom::parameters& p, float x);

    /**
     * @brief Perform initialization of internal material parameters.
     *
     * This function performs necessary initialization such as calculation
     * of average atomic number and mass, normalization of
     * atomic species concentrations, etc.
     *
     */
    void init();

    /**
     * @brief Randomly select an atom from the material
     *
     * The selection is based on the relative concentrations of the
     * costituents.
     *
     * @param g a random number generator
     * @return a pointer to the selected atom object
     */
    const atom* selectAtom(random_vars& g) const;

    /**
     * @brief Returns the damage energy according to the LSS approximation
     *
     * This function implements the LSS approximation using the average
     * atomic number and mass.
     *
     * @param recoilE the recoil energy [eV]
     * @return the damage energy [eV]
     */
    float LSS_Tdam(float recoilE) const;

    /**
     * @brief Returns the number of vacancies created by a recoil with damage energy Tdam according to the Norgett-Roninson-Torrens model
     *
     * The effective displacement threshold \f$ \bar{E}_d \f$ of the material defined by
     * Ghoniem and Chou JNM1988 is used:
     * \f[
     * \bar{E}_d^{-1} = \sum_i {X_i E_{d,i}^{-1}}
     * \f]
     *
     * @param Tdam the damage energy [eV]
     * @return the number of generated vacancies
     */
    float NRT(float Tdam) const;

    int id() const { return id_; }

    /// Returns a vector of atomic concentration of elements in the material
    const std::vector<float>& X();
    /// Returns a vector of pointers to atom objects
    const std::vector<atom*>& atoms() const { return atoms_; }

private:
    material(target* t, const char* name, int id);

    friend class target;
};

/**
 * @brief The target class keeps a list of atoms, materials and regions as well as the geometrical grid.
 * @ingroup TargetG
 */
class target
{
public:

    /**
     * @brief The region stucture descrines a rectangular volume within the target filled with a specific material
     * @ingroup TargetG
     */
    struct region {
        /// The id of this region
        std::string id;
        /// The id of the material that fills this region
        std::string material_id;
        /// Position of the region's lower left corner
        vector3 min;
        /// Position of the region's upper right corner
        vector3 max;
    };

    /**
     * @brief The target_desc_t class contains all information for the target
     */
    struct target_desc_t {
        std::vector<material::material_desc_t> materials{};
        std::vector<region> regions{};
        ivector3 cell_count{1, 1, 1};
        vector3 cell_size{100.f, 100.f, 100.f};
        ivector3 periodic_bc{0, 1, 1};
    };

protected:

    std::vector<atom*> atoms_;
    std::vector<material*> materials_;   
    std::vector< region > regions_;

    // grid points
    grid3D grid_;

    // cells
    ArrayND<const material*> cells_;

    friend class material;

    /**
     * @brief Fills a rectangular volume with a specific material
     *
     * This creates also a \ref target::region.
     *
     * @param box the rectangular volume
     * @param m a pointer to the material filling the volume
     */
    void fill(const box3D& box, const material* m);

public:
    /// Default constructor creates an empty target
    target();
    /// Destructor deletes all resources
    ~target();

    /// Returns a reference to the geometric 3D grid
    grid3D& grid() { return grid_; }
    /// Returns a constant reference to the geometric 3D grid
    const grid3D& grid() const { return grid_; }

    /// Return a vector of the defined \ref target::region objects
    const std::vector< region >& regions() const { return regions_; }
    /// Returns a vector of references to all atoms defined in the target
    const std::vector<atom*>& atoms() const { return atoms_; }
    /// Returns a vector of references to all materials defined in the target
    const std::vector<material*>& materials() const { return materials_; }

    /// Return a target_desc_t with all info on the target
    target_desc_t getDescription() const;

    /// Returns a pointer to the material in cell at index vector \p i
    const material* cell(const ivector3& i) const
    { return cells_(i.x(),i.y(),i.z()); }
    /// Returns a pointer to the material in cell index \p i
    const material* cell(int i) const
    { return cells_.data()[i]; }

    /// Set the atomic number and mass of the projectile
    void setProjectile(int Z, float M);
    /// Returns a pointer to the projectile's atomic species description
    const atom* projectile() const { return atoms_.front(); }

    /// Add a material with given name and return a pointer to the \ref material class
    material* addMaterial(const char* name);

    /// Add a material from a material descriptor \ref material::material_desc_t
    material* addMaterial(const material::material_desc_t& md);

    /// Return vector of atom labels, e.g. Fe in Fe2O3
    std::vector<std::string> atom_labels() const;

    /// Add a rectangular \ref target::region filled with a specific material
    void addRegion(const region& r);

    /**
     * @brief Perform necessary initialization of all target objects.
     *
     * Calls material::init() on all materials in the target.
     *
     * Should be called before starting a simulation.
     */
    void init();
};

#endif // TARGET_H
