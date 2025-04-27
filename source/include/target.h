#ifndef _TARGET_H_
#define _TARGET_H_

#include "geometry.h"
#include "arrays.h"
#include "ion.h"

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
class target_item
{
protected:
    target *target_;

public:
    target_item(target *t) : target_(t) { }
};

/**
 * @brief The atom class represents an atomic species in the target.
 * @ingroup TargetG
 */
class atom : public target_item
{

public:
    /**
     * @brief A target material's atom definition parameters
     */
    struct parameters
    {
        /// Element specification
        element_t element;
        /// Atomic fraction
        float X;
        /// Displacement threshold energy (eV) of target atoms
        float Ed;
        /// Lattice energy (eV) of target atoms
        float El;
        /// Surface binding energy (eV) of target atoms
        float Es;
        /// Replacement threshold energy (eV) of target atoms
        float Er;
        /// Frenkel pair recombination radius
        float Rc{ 0.946 };
    };

private:
    int id_;
    material *mat_;
    parameters p_;
    float X_; // proxy concentration

    // LSS & NRT
    float lss_Kd_;
    float lss_Efact_;
    float nrt_L_;

public:
    explicit atom(const parameters &p);

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
    /// A pointer to the target material this atom belongs to. For the beam atom nullptr is
    /// returned.
    const material *mat() const { return mat_; }
    /// Returns the chemical name of the atom
    const std::string &symbol() const { return p_.element.symbol; }
    /// Returns the atomic number
    int Z() const { return p_.element.atomic_number; }
    /// Returns the atomic mass
    float M() const { return p_.element.atomic_mass; }
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
    /// Returns the recombination radius (nm) of target atoms
    float Rc() const { return p_.Rc; }
    /// Returns true if this atom's Z and M are equal to Z and M of other
    bool operator==(const atom &other) const
    {
        return (p_.element.atomic_number == other.p_.element.atomic_number)
                && (p_.element.atomic_mass == other.p_.element.atomic_mass);
    }
    /// Returns the damage energy [eV] corresponding to recoil energy T according to LSS approx.
    float LSS_Tdam(float T) const;
    /// Returns the number of vacancies created by a recoil of damage energy Tdam according to
    /// Norgett-Roninson-Torrens model
    float NRT(float Tdam) const;

private:
    atom();
    atom(target *t, material *m, int id);

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

    std::vector<atom *> atoms_;
    std::vector<float> X_;
    std::vector<int> Z_;
    std::vector<float> cumX_;

    float atomicRadius_; // nm
    float sqrtAtomicDistance_;

    // float atomicDensity_; // at/cm^3 ??
    float atomicDensityNM_; // at/nm^3
    float meanZ_;
    float meanM_;

    // LSS & NRT
    float lss_Kd_;
    float lss_Efact_;
    float nrt_Ed_;
    float nrt_L_;

public:
    struct material_desc_t
    {
        std::string id;
        float density{ 1.f };
        std::vector<atom::parameters> composition;
    };

    explicit material(const char *name);

    /**
     * @brief Set atomic density of the material
     *
     * Invalidates other previous density setting
     *
     * @param v density in [at/nm^3]
     */
    void setAtomicDensity(float v)
    {
        atomicDensityNM_ = v;
        massDensity_ = -1;
    }
    /**
     * @brief Set mass density of the material
     *
     * Invalidates other previous density setting
     *
     * @param v density in [g/cm^3]
     */
    void setMassDensity(float v)
    {
        massDensity_ = v;
        atomicDensityNM_ = -1;
    }
    /// Returns the name of the material
    const std::string &name() const { return name_; }
    /// Returns the atomic density \f$ N \f$ [nm^-3]
    float atomicDensity() const { return atomicDensityNM_; }
    /// Returns the mass density [g/cm^3]
    float massDensity() const { return massDensity_; }
    /// Returns the atomic radius \f$ (4\pi N/3)^{-1/3} \f$ [nm]
    float atomicRadius() const { return atomicRadius_; }

    // this is used for generating options structure
    material_desc_t getDescription() const;

    /**
     * @brief addAtom adds an atom to the material
     * @param p atomic species parameters
     * @param x concentration
     * @return a pointer to the created atom object
     */
    atom *addAtom(const atom::parameters &p);

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
    const atom *selectAtom(random_vars &g) const;

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
     * @brief Returns the number of vacancies created by a recoil with damage energy Tdam according
     * to the Norgett-Roninson-Torrens model
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

    /// Returns a vector of atomic numbers of elements in the material
    const std::vector<int> &Z() const { return Z_; }
    /// Returns a vector of atomic concentration of elements in the material
    const std::vector<float> &X() const { return X_; }
    /// Returns a vector of pointers to atom objects
    const std::vector<atom *> &atoms() const { return atoms_; }

private:
    material(target *t, const char *name, int id);

    friend class target;
};

/**
 * @brief The target class keeps a list of atoms, materials and regions as well as the geometrical
 * grid.
 * @ingroup TargetG
 */
class target
{
public:
    /**
     * @brief The region stucture descrines a rectangular volume within the target filled with a
     * specific material
     *
     * A region is defined by its origin [x0, y0, z0], size [Lx, Ly, Lz],
     * and material.
     *
     * A region may contain many cells. All these cells are filled with
     * the region's material.
     *
     * A cell belongs to a region when the cell center position is within
     * the region volume.
     *
     * The order by which the regions are declared in the configuration options
     * is important. When the simulation box is constructed, the regions are
     * filled with material one after the other in the order that they appear in
     * the configuration. If there is overlap between regions, then the material
     * in a given cell will be defined by the last region that it belongs to.
     * This provides some flexibility for implementing certain target configurations.
     *
     *
     * @ingroup TargetG
     */
    struct region
    {
        /// The id of this region
        std::string id;
        /// The id of the material that fills this region
        std::string material_id;
        /// Position of the region's lower left corner, [x0, y0, z0] [nm]
        vector3 origin{ 0.f, 0.f, 0.f };
        /// Size of the region, [Lx, Ly, Lz] [nm]
        vector3 size{ 100.f, 100.f, 100.f };
    };

    /**
     * @brief The target_desc_t class contains all information for the target
     */
    struct target_desc_t
    {
        vector3 origin{ 0.f, 0.f, 0.f };
        vector3 size{ 100.f, 100.f, 100.f };
        ivector3 cell_count{ 1, 1, 1 };
        ivector3 periodic_bc{ 0, 1, 1 };
        std::vector<material::material_desc_t> materials{};
        std::vector<region> regions{};
    };

protected:
    std::vector<atom *> atoms_;
    std::vector<material *> materials_;
    std::vector<region> regions_;

    // grid points
    grid3D grid_;

    // cells
    ArrayND<const material *> cells_;

    friend class material;

    /**
     * @brief Fills a rectangular volume with a specific material
     *
     * This creates also a \ref target::region.
     *
     * @param box the rectangular volume
     * @param m a pointer to the material filling the volume
     */
    void fill(const box3D &box, const material *m);

public:
    /// Default constructor creates an empty target
    target();
    /// Destructor deletes all resources
    ~target();

    /// Returns a reference to the geometric 3D grid
    // grid3D& grid() { return grid_; }
    /// Returns a constant reference to the geometric 3D grid
    const grid3D &grid() const { return grid_; }

    /// Return a vector of the defined \ref target::region objects
    const std::vector<region> &regions() const { return regions_; }
    /// Returns a vector of references to all atoms defined in the target
    const std::vector<atom *> &atoms() const { return atoms_; }
    /// Returns a vector of references to all materials defined in the target
    const std::vector<material *> &materials() const { return materials_; }

    /// Return a target_desc_t with all info on the target
    target_desc_t getDescription() const;

    /// Returns a pointer to the material in cell at index vector \p i
    const material *cell(const ivector3 &i) const { return cells_(i.x(), i.y(), i.z()); }
    /// Returns a pointer to the material in cell index \p i
    const material *cell(int i) const { return cells_.data()[i]; }

    /// Set the atomic number and mass of the projectile
    void setProjectile(const element_t &e);
    /// Returns a pointer to the projectile's atomic species description
    const atom *projectile() const { return atoms_.front(); }

    /// Add a material with given name and return a pointer to the \ref material class
    material *addMaterial(const char *name);

    /// Add a material from a material descriptor \ref material::material_desc_t
    material *addMaterial(const material::material_desc_t &md);

    /// Return vector of atom labels, e.g. Fe in Fe2O3
    std::vector<std::string> atom_labels() const;

    /// Add a rectangular \ref target::region filled with a specific material
    void addRegion(const region &r);

    /**
     * @brief Perform necessary initialization of all target objects.
     *
     * Calls material::init() on all materials in the target.
     *
     * Should be called before starting a simulation.
     */
    void init();

    /**
     * @brief Create the target grid.
     *
     * Creates the internal grid data structure.
     *
     * Should be called before adding materials and regions.
     *
     * @param sz The size of the simulation box
     * @param n Cell count in x, y, z dimensions
     * @param pbc 3d int vector. A 1 means periodic boundary conditions
     */
    void createGrid(const vector3 &sz, const ivector3 &n, const ivector3 &pbc);
};

#endif // TARGET_H
