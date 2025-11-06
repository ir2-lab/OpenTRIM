#ifndef _ION_H_
#define _ION_H_

#include <cmath>
#include <queue>

#include "geometry.h"

#define S_ERG_TO_TIME_CONST 7.198712007850257e-02 // ps/nm-eV^(1/2)

class atom;

/**
 * @brief An basic atomic element definition struct
 */
struct element_t
{
    /// Atomic element symbol, H(Z=1) to U(Z=92)
    std::string symbol;
    /// Atomic number, 1<=Z<=92
    int atomic_number{ 0 };
    /// Atomic mass
    float atomic_mass{ 0.f };
};

/**
 * \defgroup Ions Ion generation & transport
 *
 * @brief Classes for performing ion transport and injecting new ions into the simulation.
 *
 *
 *
 */

/**
 * @brief Enum characterizing the type of boundary crossing for an ion
 *
 * @ingroup Ions
 */
enum class BoundaryCrossing {
    None, /**< No boundary crossing occured. */
    Internal, /**< The ion crossed an internal cell boundary. The ion changes cell */
    External, /**< The ion crossed an external boundary of the simulation volume. The ion exits the
                 simulation */
    InternalPBC /**< Special case: internal boundary crossing due to periodic boundary conditions */
};

/**
 * @brief The ion class represents a moving ion in the simulation.
 *
 * The class stores the ion data, such as the position and direction
 * vectors, the energy, the atomic species,
 * the id of the geometric cell the ion is currently in, the id of the
 * parent source ion, and the recoil generation id of the current ion.
 *
 * Furthermore, here are defined 2 very important simulation functions:
 * \ref propagate(), which advances the position of the moving ion and
 * \ref deflect(), which changes the ion's direction after scattering.
 *
 * @ingroup Ions
 */
class ion
{
public:
    enum ion_type { moving, vacancy, interstitial };

private:
    ion_type type_;
    vector3 pos_; // position = x,y,z in nm
    vector3 pos0_; // initial position (start of track)
    vector3 dir_; // direction cosines
    double erg_; // energy in eV
    double erg0_; // initial energy
    double t_; // time in ns
    double t0_; // start time in ns (relative to ??? TODO)
    double s_erg_to_t_;
    ivector3 icell_;
    int cellid_, // current cell id
            prev_cellid_, // previous cell id
            cellid0_; // initial cell id (start of track)
    size_t ion_id_; // ion history id
    int recoil_id_; // recoil id (generation), 0=ion, 1=PKA, ...
    size_t uid_; // unique recoil id
    const atom *atom_;
    const grid3D *grid_;

    // counters
    // they are reset when ion changes cell, stops or exits
    size_t ncoll_; // # of collisions
    double path_, // total path length
            ioniz_, // total E loss to ionization
            phonon_, // total E loss to phonons
            recoil_; // total E loss to recoils

    friend class ion_queue;

    void sub_erg(double de, double *s)
    {
        // This is needed because de may be calculated in single precision
        // and it can happen that de > erg by a small amount, e.g. 1e-6
        // This can happen, e.g., for recoil energy T in head-on collisions
        if (de > erg_)
            de = erg_;
        erg_ -= de;
        if (s)
            *s += de;
        assert(finite(erg_));
    }

public:
    /// Default constructor
    ion();

    ion_type type() const { return type_; }
    void set_type(ion_type t) { type_ = t; }

    /// Returns the ion's position vector [nm]
    const vector3 &pos() const { return pos_; }

    /// Returns the ion's position vector [nm]
    const vector3 &pos0() const { return pos0_; }

    /// Returns the vector of the ion's direction cosines
    const vector3 &dir() const { return dir_; }

    /// Returns the ion's kinetic energy
    const double &erg() const { return erg_; }

    /// Returns the ion's initial kinetic energy
    const double &erg0() const { return erg0_; }

    /// Returns the ion's current time
    const double &t() const { return t_; }

    /// Returns the ion's start time
    const double &t0() const { return t0_; }

    /// Returns the index vector of the cell the ion is currently in
    const ivector3 &icell() const { return icell_; }

    /// Returns the id of the cell the ion is currently in
    int cellid() const { return cellid_; }

    /// Returns the id of the cell the ion started in
    int cellid0() const { return cellid0_; }

    /// Returns the id of the cell the ion was previously in
    int prev_cellid() const { return prev_cellid_; }

    /// Returns the history id that the current ion belongs to
    int ion_id() const { return ion_id_; }

    /// set history id
    void setId(size_t id) { ion_id_ = id; }

    /**
     * @brief Returns the recoil generation id of the current ion
     *
     * Source ions have a recoil_id equal to 0, PKAs have recoil_id==1, etc.
     *
     * @return The current ion's recoil_id
     */
    int recoil_id() const { return recoil_id_; }

    /// increase recoil id
    void incRecoilId() { recoil_id_++; }

    /// Set the recoil id
    void setRecoilId(int id) { recoil_id_ = id; }

    /// Returns a universal id for this ion
    size_t uid() const { return uid_; }

    /// Set this ion's universal id
    void setUid(size_t id) { uid_ = id; }

    /// Returns a pointer to the \ref atom class describing the atomic species of the current ion
    const atom *myAtom() const { return atom_; }

    /// Subtract energy \a de due to phonon excitation
    void de_phonon(double de) { sub_erg(de, &phonon_); }

    /// Subtract energy \a de due to ionization
    void de_ioniz(double de) { sub_erg(de, &ioniz_); }

    /// Subtract energy \a de due to recoil generation
    void de_recoil(double de) { sub_erg(de, &recoil_); }

    /// Subtract energy \a de due to other proccesses
    void de_other(double de) { sub_erg(de, nullptr); }

    /// Returns the amount of ion energy lost to phonons
    const double &phonon() const { return phonon_; }

    /// Returns the amount of ion energy lost to ionization
    const double &ioniz() const { return ioniz_; }

    /// Returns the amount of ion energy lost to recoils
    const double &recoil() const { return recoil_; }

    /// Returns the total path (nm) traveled by the ion
    const double &path() const { return path_; }

    /// Returns the total number of ion collisions
    size_t ncoll() const { return ncoll_; }

    /// Increase the collisions counter
    void add_coll() { ncoll_++; }

    // #pragma GCC push_options
    // #pragma GCC optimize("O0")

    /// Set the ion's direction. \a d is assumed to be normalized.
    void setNormalizedDir(const vector3 &d)
    {
        assert(d.allFinite());
        assert(std::abs(d.norm() - 1.0f) <= 2 * std::numeric_limits<float>::epsilon());
        dir_ = d;
    }

    /// Set the ion's direction parallel to a vector \a d.
    void setDirParallelTo(const vector3 &d)
    {
        assert(d.allFinite());
        dir_ = d;
        dir_.normalize();
    }

    /**
     * @brief Deflect the ion after scattering.
     *
     * Changes the direction after scattering at angles \f$ (\theta,\phi) \f$.
     *
     * @param n the vector \f$ \mathbf{n} = (\cos\phi\,\sin\theta, \sin\phi\,\sin\theta,\cos\theta)
     * \f$
     * @see  \ref deflect_vector()
     */
    void deflect(const vector3 &n) { deflect_vector(dir_, n); }

    // #pragma GCC pop_options

    /// Set initial position
    int setPos(const vector3 &x);

    /// Set the atomic species of the ion
    void setAtom(const atom *a);

    /// Set the initial energy of the ion
    void setErg(double e)
    {
        erg_ = erg0_ = e;
        assert(finite(erg_));
        assert(erg_ > 0);
    }

    /// Set the initial energy of the ion
    void setTime(double t)
    {
        t_ = t0_ = t;
        assert(finite(t_));
        assert(t_ >= 0.0);
    }

    /// set a grid3D for the ion
    void setGrid(const grid3D *g) { grid_ = g; }

    /**
     * @brief Initialize this object as a recoil
     * @param a pointer to the recoil atom type
     * @param T the recoil energy
     */
    void init_recoil(const atom *a, double T);

    /// reset all accumulators (path, energy etc)
    void reset_counters()
    {
        ncoll_ = 0;
        path_ = ioniz_ = phonon_ = recoil_ = 0.0;
    }

    BoundaryCrossing propagate(float &s);

    float move(float s);
};

/**
 * @brief The ion_queue class handles queues of ion objects waiting to be simulated.
 *
 * There are 2 queues, one for the primary recoils (PKAs) and one for
 * all higher recoil generations.
 *
 * When a simulated ion generates primary recoils, these are put on
 * the PKA queue.
 *
 * After the program finishes the ion history, it runs the PKA recoils from the
 * queue. These may produce new recoils, which are put on the secondary recoil queue.
 * The program proceeds to process all recoils in both queues until there is no new recoil.
 *
 * Then the program moves to the next ion.
 *
 * Objects of the ion class are allocated by ion_queue and provided to the simulation
 * by the \ref new_ion() functions.
 *
 * When the simulation finishes an ion history, it returns the ion object to the
 * ion_queue by calling \ref free_ion(). The object buffer can then be used for
 * another ion history.
 *
 * @ingroup Ions
 */
class ion_queue
{

    // FIFO ion buffer
    typedef std::queue<ion *> ion_queue_t;

    ion_queue_t ion_buffer_; // buffer of allocated ion objects
    ion_queue_t recoil_queue_; // queue of generated recoils
    ion_queue_t pka_queue_; // queue of generated PKAs
    ion_queue_t v_queue_; // queue of vacancies
    ion_queue_t i_queue_; // queue of interstitials

    // pop an ion from the respective queue
    static ion *pop_one_(ion_queue_t &Q)
    {
        if (Q.empty())
            return nullptr;
        ion *i = Q.front();
        Q.pop();
        return i;
    }

    size_t sz_;
    size_t uctr_;

    // Returns a new ion object, optionally initialized with data copied from p
    ion *__new_ion__(const ion *p = nullptr)
    {
        ion *i;
        if (ion_buffer_.empty()) {
            i = p ? new ion(*p) : new ion;
            sz_++;
        } else {
            i = ion_buffer_.front();
            ion_buffer_.pop();
            if (p)
                *i = *p;
        }
        i->uid_ = uctr_++;
        return i;
    }

public:
    explicit ion_queue() : sz_(0), uctr_(0) { }

    /// Returns a clone of p
    ion *clone_ion(const ion &p) { return __new_ion__(&p); }

    /// Create an ion
    ion *create_ion() { return __new_ion__(); }

    /// Push an ion object to the PKA queue
    void push_pka(ion *i) { pka_queue_.push(i); }
    /// Pop a PKA ion object from the queue. If the queue is empty, a nullptr is returned.
    ion *pop_pka() { return pop_one_(pka_queue_); }

    /// Push an ion object to the recoil queue
    void push_recoil(ion *i) { recoil_queue_.push(i); }
    /// Pop a recoil ion object from the queue. If the queue is empty, a nullptr is returned.
    ion *pop_recoil() { return pop_one_(recoil_queue_); }

    /// Push an ion object to the recoil queue
    void push_vacancy(ion *i)
    {
        i->set_type(ion::vacancy);
        v_queue_.push(i);
    }
    /// Pop a recoil ion object from the queue. If the queue is empty, a nullptr is returned.
    ion *pop_vacancy() { return pop_one_(v_queue_); }

    /// Push an ion object to the recoil queue
    void push_interstitial(ion *i)
    {
        i->set_type(ion::interstitial);
        i_queue_.push(i);
    }
    /// Pop a recoil ion object from the queue. If the queue is empty, a nullptr is returned.
    ion *pop_interstitial() { return pop_one_(i_queue_); }

    /// Release a used ion object
    void free_ion(ion *i) { ion_buffer_.push(i); }

    /// Clear all allocated ion objects from memory
    void clear()
    {
        while (!ion_buffer_.empty()) {
            ion *i = ion_buffer_.front();
            ion_buffer_.pop();
            delete i;
            sz_--;
        }
        assert(sz_ == 0);
    }

    size_t size() const { return sz_; }
};

#endif // ION_H
