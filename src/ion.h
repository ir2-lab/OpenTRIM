#ifndef _ION_H_
#define _ION_H_

#include <cmath>
#include <queue>

#include "geometry.h"

class atom;

/**
 * \defgroup Ions Ion generation & transport
 *
 * @brief Classes for performing ion transport and injecting new ions into the simulation.
 *
 * @{
 *
 * @ingroup MC
 *
 * @}
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
    None,       /**< No boundary crossing occured. */
    Internal,   /**< The ion crossed an internal cell boundary. The ion changes cell */
    External,    /**< The ion crossed an external boundary of the simulation volume. The ion exits the simulation */
    InternalPBC   /**< Special case: internal boundary crossing due to periodic boundary conditions */
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
    vector3 pos_; // position = x,y,z in nm
    vector3 pos0_; // initial position (start of track)
    vector3 dir_; // direction cosines
    double erg_; // energy in eV
    double erg0_; // initial energy
    ivector3 icell_;
    int cellid_, // current cell id
        prev_cellid_, // previous cell id
        cellid0_; // initial cell id (start of track)
    size_t ion_id_; // history id
    int recoil_id_; // recoil id (generation), 0=ion, 1=PKA, ...
    const atom* atom_;
    const grid3D* grid_;

    // counters
    // they are reset when ion changes cell, stops or exits
    size_t ncoll_; // # of collisions
    double path_, // total path length
        ioniz_, // total E loss to ionization
        phonon_, // total E loss to phonons
        recoil_; // total E loss to recoils

public:

    /// Default constructor
    ion() :
        pos_(0.f,0.f,0.f),
        dir_(0.f,0.f,1.f),
        erg_(1.f), erg0_(1.f),
        icell_(), cellid_(-1), prev_cellid_(-1),
        ion_id_(0), recoil_id_(0),
        atom_(nullptr), grid_(nullptr),
        ncoll_(0),
        path_(0),ioniz_(0),phonon_(0), recoil_(0)
    {}

    /// Returns the ion's position vector [nm]
    const vector3& pos() const { return pos_; }

    /// Returns the vector of the ion's direction cosines
    const vector3& dir() const { return dir_; }

    /// Returns the ion's kinetic energy
    const double& erg() const { return erg_; }

    /// Returns the ion's initial kinetic energy
    const double& erg0() const { return erg0_; }

    /// Returns the index vector of the cell the ion is currently in
    const ivector3& icell() const { return icell_; }

    /// Returns the id of the cell the ion is currently in
    int cellid() const { return cellid_; }

    /// Returns the id of the cell the ion started in
    int cellid0() const { return cellid0_; }

    /// Returns the id of the cell the ion was previously in
    int prev_cellid() const { return prev_cellid_; }

    /// Returns the history id that the current ion belongs to
    int ion_id() const { return ion_id_; }

    /**
     * @brief Returns the recoil generation id of the current ion
     *
     * Source ions have a recoil_id equal to 0, PKAs have recoil_id==1, etc.
     *
     * @return The current ion's recoil_id
     */
    int recoil_id() const { return recoil_id_; }

    /// Returns a pointer to the \ref atom class describing the atomic species of the current ion
    const atom* myAtom() const { return atom_; }

    void de_phonon(double de) {
        erg_ -= de;
        phonon_ += de;
        assert(erg_>=0);
        assert(finite(erg_));
    }
    void de_ioniz(double de) {
        erg_ -= de;
        ioniz_ += de;
        assert(erg_>0);
        assert(finite(erg_));
    }
    void de_recoil(double T) {
        // This is needed because recoil T is calculated in single precision
        // and it can happen that T > erg by a small amount, e.g. 1e-6
        if (T >= erg_ ) T = erg_;
        erg_ -= T;
        recoil_ += T;
        assert(erg_>=0);
        assert(finite(erg_));
    }
    const double& phonon() const { return phonon_; }
    const double& ioniz() const { return ioniz_; }
    const double& recoil() const { return recoil_; }
    const double& path() const { return path_; }
    size_t ncoll() const { return ncoll_; }

    void add_coll() { ncoll_++; }

    /// Return reference to the vector of direction cosines
    vector3& dir() { return dir_; }   

    /// Set initial direction
    void setDir(const vector3 d) {
        dir_ = d;
        dir_.normalize();
        assert(dir_.allFinite());
    }
    /// Set initial position
    int setPos(const vector3& x);
    /// Set the atomic species of the ion
    void setAtom(const atom* a) {
        atom_ = a;
    }
    /// Set the initial energy of the ion
    void setErg(double e) {
        erg_ = erg0_ = e;
        assert(finite(erg_));
        assert(erg_>0);
    }
    /// increase recoil id
    void incRecoilId() {
        recoil_id_++;
    }
    /// set history id
    void setId(size_t id) {
        ion_id_ = id;
    }
    /// reset recoil id to 0
    void resetRecoilId() {
        recoil_id_ = 0;
    }
    /// set a grid3D for the ion
    void setGrid(const grid3D* g) {
        grid_ = g;
    }

    void init_recoil(const atom* a, double T);

    /**
     * @brief Deflect the ion after scattering.
     * 
     * Changes the direction after scattering at angles \f$ (\theta,\phi) \f$.
     * 
     * @param n the vector \f$ \mathbf{n} = (\cos\phi\,\sin\theta, \sin\phi\,\sin\theta,\cos\theta) \f$
     * @see  \ref deflect_vector()
     */
    void deflect(const vector3& n)
    {
        deflect_vector(dir_,n);
    }

    /// reset all accumulators (path, energy etc)
    void reset_counters()
    {
        ncoll_=0;
        path_=ioniz_=phonon_=recoil_=0.0;
    }

    BoundaryCrossing propagate(float& s);
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
class ion_queue {

    // FIFO ion buffer
    typedef std::queue< ion* > ion_queue_t;

    ion_queue_t ion_buffer_; // buffer of allocated ion objects
    ion_queue_t recoil_queue_; // queue of generated recoils
    ion_queue_t pka_queue_; // queue of generated PKAs

    // pop an ion from the respective queue
    static ion* pop_one_(ion_queue_t& Q)
    {
        if (Q.empty()) return nullptr;
        ion* i = Q.front();
        Q.pop();
        return i;
    }

    size_t sz_;

public:
    explicit ion_queue() : sz_(0) {}

    /// Returns a new ion object, initialized with data copied from non-null p
    ion* new_ion(const ion* p = nullptr) {
        ion* i;
        if (ion_buffer_.empty()) {
            i = p ? new ion(*p) : new ion;
            sz_++;
        } else {
            i = ion_buffer_.front();
            ion_buffer_.pop();
            if (p) *i = *p;
        }        
        return i;
    }

    /// Push an ion object to the PKA queue
    void push_pka(ion* i) {
        pka_queue_.push(i);
    }
    /// Push an ion object to the recoil queue
    void push_recoil(ion* i) {
        recoil_queue_.push(i);
    }
    /// Pop a PKA ion object from the queue. If the queue is empty, a nullptr is returned.
    ion* pop_pka() {
        return pop_one_(pka_queue_);
    }
    /// Pop a recoil ion object from the queue. If the queue is empty, a nullptr is returned.
    ion* pop_recoil() {
        return pop_one_(recoil_queue_);
    }

    /// Release a used ion object
    void free_ion(ion* i) {
        ion_buffer_.push(i);
    }

    /// Clear all allocated ion objects from memory
    void clear() {
        while (!ion_buffer_.empty()) {
            ion* i = ion_buffer_.front();
            ion_buffer_.pop();
            delete i;
            sz_--;
        }
        assert(sz_==0);
    }

    size_t size() const {
        return sz_;
//        return ion_buffer_.size() +
//               pka_queue_.size() +
//               recoil_queue_.size();
    }
};

#endif // ION_H
