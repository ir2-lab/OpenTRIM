#ifndef _MCCORE_H_
#define _MCCORE_H_

#include "target.h"
#include "random_vars.h"
#include "ion_beam.h"
#include "ion.h"
#include "xs.h"
#include "tally.h"
#include "event_stream.h"
#include "dedx.h"
#include "flight_path_calc.h"
#include "dedx_calc.h"

// for thread sync
#include <atomic>
#include <mutex>

class cascade_queue;

/**
 * \defgroup MC libopentrim shared library
 *
 * \brief A library containing the main ion transport simulation code.
 *
 * The building blocks for the Monte-Carlo ion transport simulation are: \ref Geometry,
 * \ref Ions "Ion transport", \ref RNG and \ref target "Target definition".
 *
 * All these are combined into the simulation \ref Core.
 *
 * The \ref Driver part facilitates the process of loading the configuration,
 * setting up the simulation, executing on multiple threads and adding up and storing the results.
 *
 *
 */

/**
 * \defgroup Core Core
 *
 * \brief The core of the Monte-Carlo ion transport simulation
 *
 * @{
 *
 * It comprises of 3 classes
 *
 *
 * @ingroup MC
 *
 * @}
 *
 *
 */

/**
 * @brief The mccore class defines the core Monte-Carlo ion transport simulation algorithms.
 *
 * @ingroup Core
 */
class mccore
{
public:
    /**
     * @brief Type of simulation
     */
    enum simulation_type_t {
        FullCascade = 0, /**< Full Damage Cascade, follow recoils */
        IonsOnly = 1, /**< Ions only. Recoils are not followed. Damage estimate by NRT */
        CascadesOnly = 2, /**< Cascades generated and followed */
        InvalidSimulationType = -1
    };

    /**
     * @brief Detail of NRT implementation in multielement materials
     */
    enum nrt_calculation_t {
        NRT_element = 0, /**< NRT calculated using Ed of struck atom (similar to SRIM) */
        NRT_average = 1, /**< NRT calculated with average Ed (J.-P. Crocombette 2019) */
        NRT_InvalidOption = -1
    };

    /**
     * @brief Simulation parameters/options
     */
    struct parameters
    {
        /// Type of the simulation
        simulation_type_t simulation_type{ FullCascade };
        /// screeninig
        Screening screening_type{ Screening::ZBL };
        /// Electronic energy loss calculation option
        dedx_calc::eloss_calculation_t eloss_calculation{ dedx_calc::EnergyLoss };
        /// Model to use for calculating electronic straggling (if calculated)
        StragglingModel straggling_model{ StragglingModel::Bohr };
        /// Way to calculate NRT vacancies in multielement materials
        nrt_calculation_t nrt_calculation{ NRT_element };
        /// Allow intra cascade Frenkel pair recombination
        bool intra_cascade_recombination{ false };
        /****  Experimental stuff ****/
        // Allow same Frenkel pair recombination
        bool correlated_recombination{ true };
        // Move recoil atom to Rc
        bool move_recoil{ false };
        // Subtract Ed from the recoil atom
        bool recoil_sub_ed{ false };
    };

    /**
     * @brief Ion transport options
     */
    struct transport_options
    {
        /// Free flight path selection algorithm
        flight_path_calc::flight_path_type_t flight_path_type{ flight_path_calc::AtomicSpacing };
        /// The constant flight path (for algorithm flight_path_type_t=Constant) [nm]
        float flight_path_const{ 0.1f };
        /// Minimum energy cutoff for ion transport [eV]
        float min_energy{ 1.f };
        /// Minimum recoil energy
        float min_recoil_energy{ 1.f };
        /// Scattering at shorter distances than interatomic distance
        bool allow_sub_ml_scattering{ false };
        /// Max mfp
        float max_mfp{ 1.0e30f };
        /// Max dE/E per mfp, dE = dEdx*mfp
        float max_rel_eloss{ 0.05f };
    };

protected:
    ion_queue q_;

    // simulation paramenters
    parameters par_;
    // transport options
    transport_options tr_opt_;

    // components
    ion_beam *source_;
    target *target_;
    tally tally_, dtally_, tion_;
    random_vars rng;
    event_stream pka_stream_, exit_stream_;
    pka_event pka;
    exit_event exit_ev;

    // ref counter
    std::shared_ptr<int> ref_count_;

    // max # of ion histories to run
    size_t max_no_ions_;

    // shared ion counter
    std::shared_ptr<std::atomic_size_t> ion_counter_; // shared ion counter
    std::atomic_size_t thread_ion_counter_; // per thread ion counter

    // shared abort flag
    std::shared_ptr<std::atomic_bool> abort_flag_; // thread safe abort simulation flag

    // shared mutex for tally data
    std::shared_ptr<std::mutex> tally_mutex_;

    // electronic dedx & straggling calculator
    dedx_calc dedx_calc_;

    // flight path calculator
    flight_path_calc flight_path_calc_;

    // Scattering cross-section array for all
    // projectile/target combinations
    ArrayND<abstract_xs_lab *> scattering_matrix_;

public:
    mccore();
    mccore(const parameters &p, const transport_options &t);
    mccore(const mccore &S);
    ~mccore();

    /**
     * @brief Return the max number of ions to run
     * This is the total number of histories to be run by all threads
     * @return the max number of ions to run
     */
    size_t max_no_ions() const { return max_no_ions_; }
    /**
     * @brief Set the max number of ions to run
     * This is the total number number of histories to be run by all threads
     * @param n the number of histories to run
     */
    void setMaxIons(size_t n) { max_no_ions_ = n; }

    // Number of simulated ion histories
    // This number refers to all threads
    size_t ion_count() const { return *ion_counter_; }
    void setIonCount(size_t n) { *ion_counter_ = n; }
    // Number of ions run in the current tread
    size_t thread_ion_count() const { return thread_ion_counter_; }

    /// Returns the core simulation parameters
    const parameters &getSimulationParameters() const { return par_; }
    /// Returns the transport parameters
    const transport_options &getTransportOptions() const { return tr_opt_; }

    /// Return a reference to the \ref target object
    target &getTarget() { return *target_; }
    /// Return a const reference to the target object
    const target &getTarget() const { return *target_; }
    /// Return a reference to the source (ion_beam) object
    ion_beam &getSource() { return *source_; }
    /// Return a const reference to the source (ion_beam) object
    const ion_beam &getSource() const { return *source_; }
    /// Return a const reference to the tally object
    const tally &getTally() const { return tally_; }
    /// Return a const reference to the tally variance object
    const tally &getTallyVar() const { return dtally_; }
    /// Return a reference to the tally object
    tally &getTally() { return tally_; }
    /// Return a reference to the tally variance object
    tally &getTallyVar() { return dtally_; }

    ArrayNDd getTallyTable(int i) const;
    ArrayNDd getTallyTableVar(int i) const;
    void copyTallyTable(int i, ArrayNDd &A) const;
    void copyTallyTableVar(int i, ArrayNDd &dA) const;

    /// Open file stream to store \ref pka_event data
    int open_pka_stream() { return pka_stream_.open(pka); }
    /// Return reference to the pka stream
    event_stream &pka_stream() { return pka_stream_; }
    /// Open file stream to store \ref exit_event data
    int open_exit_stream() { return exit_stream_.open(exit_ev); }
    /// Return reference to the exit stream
    event_stream &exit_stream() { return exit_stream_; }

    // scattering matrix (atoms x materials)
    ArrayND<abstract_xs_lab *> scattering_matrix() const { return scattering_matrix_; }

    /// Return a reference to the electronic energy loss calculator object
    const dedx_calc &get_dedx_calc() const { return dedx_calc_; }

    /// Return a reference to the flight path calculator object
    const flight_path_calc &get_fp_calc() const { return flight_path_calc_; }

    /**
     * @brief Initialize internal variables of the mccore object
     *
     * The init() function performs the following:
     *  - calls the init() function of the underlying target and ion_beam objects
     *  - creates electronic stopping and straggling interpolation tables for all atom/material
     * combinations
     *  - creates an array of lab frame scattering cross-section objects (\ref xs_lab) for all ion
     * combinations
     *  - creates tables of mean free path and max impact parameter for flight path selection
     *  - allocates tally memory
     *
     * @return 0 if succesfull
     */
    int init();

    /**
     * @brief Seed the random number generator
     * @param s the seed value
     */
    void seed(unsigned int s) { rng.seed(s); }

    void rngJump() { rng.longJump(); }
    random_vars::state_type rngState() const { return rng.state(); }
    void setRngState(const random_vars::state_type &s) { return rng.state(s); }

    /**
     * @brief Merge the results from another simulation object
     *
     * The tallies from @p other are
     * added to this object's tallies.
     *
     * The
     * tallies of @p other are cleared.
     *
     * @param other a simulation object
     */
    void mergeTallies(mccore &other);

    /**
     * @brief Merge the events from another simulation object
     *
     * The streamed events from @p other are
     * added to this object's event streams.
     *
     * The events of @p other are cleared.
     *
     * @param other a simulation object
     */
    void mergeEvents(mccore &other);

    /**
     * @brief Run the ion transport simulation
     *
     * This function runs the actual Monte-Carlo loop, repeating
     * the following steps:
     *  - Call the source object to generate a new ion
     *  - Call transport() to propagate the ion through the target
     *  - Call transport() for all recoils generated by the ion and all their secondary recoils
     *
     * The loop is repeated until the specified max_no_ions() has been completed
     * or the simulation is aborted.
     *
     * @return 0 if succesfull
     *
     */
    int run();

    void clear_abort_flag() { *abort_flag_ = false; }
    void abort() { *abort_flag_ = true; }
    bool abort_flag() const { return *abort_flag_; }

    int reset();

protected:
    /**
     * @brief Transport an ion through the target
     *
     * This function propagates an ion by repeating the following steps in a loop:
     *   - Call flight_path_calc::operator() to sample flight path and impact parameter
     *   - Call ion::propagate() to advance the ion position
     *   - Call dedx_calc::operator() to calculate stopping and straggling and subtract it from the
     * ion energy
     *   - Select collision partner, select the cross-section object @ref xs_lab for
     *   the projectile/target pair and call xs_lab::scatter() to obtain scattering angle and recoil
     * energy \f$T\f$
     *   - If \f$T>E_d\f$, where \f$E_d\f$ is the \ref atom::parameters::Ed "displacement energy",
     * call new_recoil() to create a recoil ion and put it the \ref ion_queue "simulation queue"
     *   - Otherwise \f$T\f$ is deposited as lattice vibrations
     *
     * The loop is repeated until either the ion exits the target or
     * its energy becomes less than parameters::min_energy whereupon the ion
     * stops and remains implanted in the target.
     *
     * Tally scoring is performed at specific simulation events
     * (cell change, ion exit, implantation)
     * using the \ref tally object \p t.
     *
     * @param i pointer to the ion object
     * @param t reference to the tally object
     * @return 0 if succesfull
     */
    int transport(ion *i, tally &t, cascade_queue *q = nullptr);

    /**
     * @brief Generate a new recoil ion
     *
     * This function creates a new recoil ion of atomic species \p target
     * with energy \p recoil_erg and direction \p nt.
     *
     * The newly created recoil is put on the respective \ref ion_queue in order to
     * be transported later.
     *
     * @param proj the scattered projectile ion
     * @param target pointer to the atomic species of the target atom
     * @param recoil_erg recoil energy of the target atom
     * @param nt recoil direction
     * @return a pointer to the newly created moving ion
     */
    ion *new_recoil(const ion *proj, const atom *target, const float &recoil_erg, const vector3 &nt)
    {
        // init the recoil by cloning the projectile ion
        // this gets the correct position and cell id
        ion *j = q_.new_ion(proj);

        // set atomic species,
        // recoil kinetic energy & direction
        // Energy partition
        //   kinetic energy = T-Ed
        //   Ed-Efp goes to the lattice (phonos)
        //   Efp is stored energy (will be deposited when the ion finishes)
        j->init_recoil(target, recoil_erg - target->El());
        j->reset_counters();
        // j->de_phonon(target->Ed()); //(target->Ed()-target->El());
        j->setNormalizedDir(nt);

        // store recoil in respective queue
        if (j->recoil_id() == 1)
            q_.push_pka(j);
        else
            q_.push_recoil(j);

        return j;
    }
};

#endif // SIMULATION_H
