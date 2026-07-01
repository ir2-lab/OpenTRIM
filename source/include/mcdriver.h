#ifndef MCDRIVER_H
#define MCDRIVER_H

#include "mccore.h"

#include <ctime>
#include <thread>
#include "user_tally.h"

/**
 * @brief mcconfig contains all configuration options for seting up and running simulation.
 *
 * The various options are organized in groups
 * - mcconfig::Simulation - general simulation configuration options
 * - mcconfig::Transport - options related to ion transport
 * - mcconfig::IonBeam - ion beam source definition
 * - mcconfig::Target - target definition
 * - mcconfig::Run - options for running the simulation
 * - mcconfig::Output - options for data output.
 *
 * Each option group is a C++ struct and is a field of
 * mcconfig which itself is also a struct.
 *
 * The configuration info can be easily transfered to/from a \ref json_config "JSON string" with
 * the functions parseJSON(), printJSON(), toJSON().
 *
 * \ref json_config "The JSON config string" is used for passing configuration options
 * to the \ref cliapp "opentrim cli program" and can also be loaded in opentrim-gui.
 *
 * @note The mnemonic used in JSON for each option is the same as name of the option in C++.
 * E.g. the type of simulation is defined at JSON path "/Simulation/simulation_type" and in
 * the C++ value mcconfig::Simulation::simulation_type
 *
 * The functions get() and set() can read and write the options in the C++ structure
 * using JSON formatting.
 *
 * Finally, createSimulation() uses the info stored in the \a mcconfig struct to create a
 * \ref mccore object ready to run.
 *
 * @ingroup Core
 */
struct mcconfig
{
    enum option_type_t {
        tEnum,
        tFloat,
        tInt,
        tBool,
        tString,
        tVector,
        tIntVector,
        tStruct,
        tArray,
        tInvalid
    };

    /// parameters for running the simulation
    struct run_options
    {
        /// Maximum number of ions to run
        size_t max_no_ions{ 100 };
        /// Maximum cpu time to run (s)
        size_t max_cpu_time{ 0 };
        /// Number of threads to use
        int threads{ 1 };
        /// Seed for the random number generator
        unsigned int seed{ 123456789 };
    };

    /// output parameters
    struct output_options
    {
        /// Simulation title
        std::string title{ "Ion Simulation" };
        /// Base name for the output file
        std::string outfilename{ "out" };
        /// Interval in ms to backup simulation data @TODO
        int storage_interval{ 60000 };
        /// Store ion exit events
        bool store_exit_events{ false };
        /// Store the pka events
        bool store_pka_events{ false };
        /// Store the damage events
        bool store_damage_events{ false };
        /// Store electronic energy loss data
        bool store_dedx{ true };
    };

    /// General Simulation parameters
    mccore::parameters Simulation;
    /// Ion transport parameters
    mccore::transport_options Transport;
    /// Source ion beam parameters
    ion_beam::parameters IonBeam;
    /// Target definition
    target::target_desc_t Target;
    /// Options for running the simulation
    run_options Run;
    /// Output options
    output_options Output;
    /// UserTally options
    std::vector<user_tally::parameters> UserTally;

    /**
     * @brief Parse simulation mcconfig from JSON formatted input
     *
     * For a full list of available mcconfig and details on JSON
     * formatting see \ref json_config.
     *
     * The function first parses the whole JSON string. On formatting errors
     * the function stops and prints an error message to stderr.
     *
     * After that validate() is called to check the given mcconfig. Errors
     * are again reported to stderr.
     *
     * @param js a JSON formatted input stream
     * @param doValidation if true the function calls validate()
     * @param os optional pointer to an output stream to recieve error messages
     * @param strict if true reject unrecognized option keys during config validation
     * @return 0 if succesfull, negative value otherwise
     */
    int parseJSON(std::istream &js, bool doValidation = true, std::ostream *os = nullptr,
                  bool strict = true);

    /// Pretty print JSON formattet mcconfig to a stream
    void printJSON(std::ostream &os) const;

    /**
     * @brief Returns a json string with specifications for all mcconfig options
     *
     * The returned json structure includes, for every mcconfig option
     * - mnemonic
     * - path in OpenTRIM json config
     * - type (string, number, enum, etc)
     * - valid range
     * - short description
     *
     * @return The json code as a std::string
     */
    static std::string options_spec();

    /**
     * @brief Returns a configuration template with all default options
     *
     * The returned configuration has placeholders for all available options,
     * including
     * - materials
     * - regions
     * - user-specified tallies
     *
     * @return The template configuration
     */
    static mcconfig config_template();

    /// Return mcconfig as a JSON string
    std::string toJSON() const;

    /**
     * @brief Set config options using JSON
     *
     * Example to set the number of ions to run:
     * @code{.cpp}
     * mcconfig config;
     * config.set("/Run/max_no_ions","1000");
     * @endcode
     *
     * Enum options are string-encoded in JSON. In this case
     * the value must be pased with double quotes so that the JSON
     * parser encodes it as a string. Example
     *
     * @code{.cpp}
     * mcconfig config;
     * config.set("/Simulation/simulation_type","\"FullCascade\"");
     * @endcode
     *
     * @param path the json path to the configuration option
     * @param json_str json formatted value to be stored
     * @param os optional stream to receive warnings and error messages
     * @return true if succesfull, false otherwise
     */
    bool set(const std::string &path, const std::string &json_str, std::ostream *os = nullptr);

    /**
     * @brief Get config options in JSON format
     *
     * Example to get the number of ions to run:
     * @code{.cpp}
     * mcconfig config;
     * std::string s;
     * config.get("/Run/max_no_ions",s); // s = "100"
     * @endcode
     *
     * @param path the json path to the configuration option
     * @param json_str json formatted value of the option
     * @param os optional stream to receive warnings and error messages
     * @return true if succesfull, false otherwise
     */
    bool get(const std::string &path, std::string &json_str, std::ostream *os = nullptr) const;

    /**
     * @brief Validate the simulation mcconfig
     *
     * A number of checks are performed including
     * - correct parameter range
     * - allowed parameter combinations
     * - target definition (geometry, materials, regions)
     *
     * On error, a std::invalid_argument exception is thrown.
     * exception::what() returns a relevant error message.
     *
     * @param AcceptIncomplete if true, empty values are accepted
     * @return
     */
    int validate(bool AcceptIncomplete = false) const;

private:
    void set_impl_(const std::string &path, const std::string &json_str);
    void get_impl_(const std::string &path, std::string &json_str) const;
};

/**
 * @brief The mcdriver class facilitates the setup and running of a simulation.
 *
 * \ref mcdriver and its sub-classes can be used to perform
 * the following tasks:
 *
 * - Parse the configuration options from JSON
 * - Validate the configuration
 * - Create the \ref mccore object, generate the geometry and load all options
 * - Run the simulation
 * - Save the results
 *
 * A typical usage scenario would be:
 *
 * @code{.cpp}
 * #include "mcdriver.h"
 * #include <iostream>
 *
 * mcdriver::options opt;
 * opt.parseJSON(std::cin);
 * auto d = mcdriver::create(opt);
 * d->exec();
 * d->save();
 * @endcode
 *
 * @ingroup Core
 */
class mcdriver : public std::enable_shared_from_this<mcdriver>
{
public:
    /// Typedef for a function to be called during simulation execution
    typedef void (*progress_callback)(const mcdriver *v, void *p);

    struct run_data
    {
        std::string start_time;
        std::string end_time;
        double ions_per_cpu_s;
        double cpu_time_s;
        int nthreads;
        size_t run_ion_count;
        size_t total_ion_count;
    };

    struct version_info_t
    {
        const char *project_name;
        const char *description;
        const char *version;
        const char *git_tag;
        const char *build_time;
        const char *compiler_id;
        const char *compiler_version;
        const char *system_id;
    };

protected:
    static version_info_t version_info_;

    std::vector<run_data> run_history_;

    // config
    mcconfig config_;

    // the simulation object
    std::unique_ptr<mccore> s_;

    // The following 2 are populated when the
    // simulation runs
    // 1. threads
    std::vector<std::thread> thread_pool_;
    // 2. simulation execution clones
    std::vector<mccore *> sim_clones_;

    // No default constructor
    mcdriver() = delete;
    // Protected constructor. use either create() or load()
    mcdriver(const mcconfig &cfg);

public:
    /**
     * @brief Create a simulation driver with the given options
     *
     * This function creates a driver object and initializes the
     * simulation according to the options in @a cfg.
     *
     * @param cfg A \ref mcconfig struct with the required specs
     * @return
     */
    static std::shared_ptr<mcdriver> create(const mcconfig &cfg, std::ostream *os = nullptr);

    ~mcdriver();

    static const version_info_t &version_info() { return version_info_; }

    /**
     * @brief Get the currently active configuration
     */
    const mcconfig &config() const { return config_; }

    /// Set the output options.
    // void setOutputOptions(const mcconfig::output_options &opts) { config_.Output = opts; }
    /// Set the run options.
    // void setRunOptions(const mcconfig::run_options &opts) { config_.Run = opts; }

    /// Returns a const pointer to the mccore simulation object
    const mccore *getSim() const { return s_.get(); }

    /// Returns the current number of simulated ions
    size_t ion_count() const { return s_->ion_count(); }

    /// Return the max number of ions to simulate
    size_t max_no_ions() const { return config_.Run.max_no_ions; }
    /**
     * @brief Set the max number of ions to simulate
     *
     * This should be set before calling exec().
     *
     * If the function is called while the simulation
     * is running, the set value will be effective the
     * next time that exec() is called.
     *
     * @param n the number of histories to run
     */
    void setMaxNoIons(size_t n) { config_.Run.max_no_ions = n; }

    /**
     * @brief Return the number of threads
     *
     * If the simulation is running, this function returns the
     * actual number of active threads.
     *
     * Otherwise, it returns the configured number of threads
     * that will be used in the next run.
     *
     * @return the number of threads
     */
    int threads() const { return is_running() ? thread_pool_.size() : config_.Run.threads; }
    /**
     * @brief Set the number threads
     *
     * Set the number of execution threads.
     *
     * A value of 0 means that OpenTRIM will use the optimum number
     * of threads for the given system.
     *
     * This value should be set before calling exec().
     *
     * If the function is called while the simulation
     * is running, the set value will be effective the
     * next time that exec() is called.
     *
     * @param n the number of histories to run
     */
    void setNthreads(size_t n) { config_.Run.threads = n; }

    /// Return the random number generator seed
    unsigned int seed() const { return config_.Run.seed; }
    /**
     * @brief Set the random number generator seed
     *
     * The seed can be set before starting the simulation, i.e.,
     * is_running() should return false and ion_count() should be 0.
     *
     * If these conditions are not satisfied the seed is
     * not set and the function returns false.
     *
     * @param n The RNG seed value
     * @return true if the seed has been set
     */
    bool setSeed(unsigned int n)
    {
        if (is_running() || ion_count())
            return false;
        config_.Run.seed = n;
        return true;
    }

    /// Return the base name of the output file
    const std::string &outFileName() const { return config_.Output.outfilename; }
    /// Set the base name of the output file
    void setOutFileName(const std::string &name) { config_.Output.outfilename = name; }

    /// Returns true if the simulation is running
    bool is_running() const { return thread_pool_.size() > 0; }
    /// Signal a running simulation to abort
    void abort();
    /// Wait for a running simulation to finish
    void wait();
    /// Returns a reference to the run history
    const std::vector<run_data> &run_history() const { return run_history_; }

    /**
     * @brief Saves all data and results in a HDF5 file
     *
     * For details on the structure of the output file see \ref out_file.
     *
     * @param h5filename the output file name
     * @param os optional stream pointer to write any error messages
     * @return 0 if succesful
     */
    int save(const std::string &h5filename, std::ostream *os = nullptr);

    /**
     * @brief Load a simulation from a HDF5 file and create a driver object
     *
     * This function attempts to load a simulation from a HDF5 file and
     * return a shared_ptr to the driver object.
     *
     * The driver can be used to run more simulation histories.
     *
     * If the function fails, an empty shared_ptr is returned. Errors are
     * reported in the optional stream pointer.
     *
     * @param h5filename the name of the file
     * @param os optional stream pointer to write any error messages
     * @return A shared_ptr to the mcdriver or an empty shared_ptr.
     */
    static std::shared_ptr<mcdriver> load(const std::string &h5filename,
                                          std::ostream *os = nullptr);

    /**
     * @brief Execute the simulation
     *
     * Runs the simulation for a max # of ion histories specified in
     * mcdriver::parameters.max_ion_count.
     *
     * The function spawns the specified number of execution threads
     * that run in parallel.
     *
     * Each thread runs a clone of the simulation. When all threads finish
     * the results are merged to the parent simulation object.
     *
     * @param cb Pointer to user-supplied callback function (optional)
     * @param msInterval Period in ms between calls to callback
     * @param callback_user_data Pointer to user data to pass to the callback function (optional)
     * @return 0 on success, non-zero otherwise
     */
    int exec(progress_callback cb = nullptr, size_t msInterval = 1000,
             void *callback_user_data = 0);
};

#endif // MCDRIVER_H
