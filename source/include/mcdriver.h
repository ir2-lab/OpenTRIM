#ifndef MCDRIVER_H
#define MCDRIVER_H

#include "mccore.h"

#include <ctime>
#include <thread>
#include "user_tally.h"


/**
 * \defgroup Driver Driver
 *
 * \brief Classes for setting up and running a simulation
 *
 * The \ref mcdriver and its sub-classes can be used to perform
 * the following tasks:
 *
 * - Parse the configuration options from JSON
 * - Validate the configuration
 * - Create the \ref mccore object, generate the geometry and load all options
 * - Run the simulation
 * - Save the results
 *
 * @{
 *
 * @ingroup MC
 *
 * @}
 *
 *
 */

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
 * @ingroup Driver
 */
struct mcconfig
{
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
        /// Interval in sec to store the output @todo
        int storage_interval{ 1000 };
        /// Store ion exit events
        bool store_exit_events{ false };
        /// Store the pka events
        bool store_pka_events{ false };
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
    user_tally::parameters UserTally;

    /**
     * @brief Parse simulation mcconfig from JSON formatted input
     *      * For a full list of available mcconfig and details on JSON
     * formatting see \ref json_config.
     *      * The function first parses the whole JSON string. On formatting errors
     * the function stops and prints an error message to stderr.
     *      * After that validate() is called to check the given mcconfig. Errors
     * are again reported to stderr.
     *      * @param js a JSON formatted input stream
     * @param doValidation if true the function calls validate()
     * @return 0 if succesfull, negative value otherwise
     */
    int parseJSON(std::istream &js, bool doValidation = true, std::ostream *os = nullptr);

    /// Pretty print JSON formattet mcconfig to a stream
    void printJSON(std::ostream &os) const;

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
     *      * A number of checks are performed
     * including
     * - correct parameter range
     * - allowed parameter combinations
     * - target definition (geometry, materials, regions)
     *      * On error, a std::invalid_argument exception is thrown.
     * exception::what() returns a relevant error message.
     *      * @param AcceptIncomplete if true, empty values are accepted
     * @return
     */
    int validate(bool AcceptIncomplete = false);

    /**
     * @brief Create a simulation object from the given mcconfig
     * @return a pointer to a mccore object
     */
    mccore *createSimulation() const;

private:
    void set_impl_(const std::string &path, const std::string &json_str);
    void get_impl_(const std::string &path, std::string &json_str) const;
};

/**
 * @brief The mcdriver class facilitates the setup and running of a simulation.
 *
 * A typical usage scenario would be:
 *
 * @code{.cpp}
 * #include "mcdriver.h"
 * #include <iostream>
 *
 * mcdriver::options opt;
 * opt.parseJSON(std::cin);
 * mcdriver d;
 * d.setOptions(opt);
 * d.exec();
 * d.save();
 * @endcode
 *
 * @ingroup Driver
 */
class mcdriver
{
public:
    /// Typedef for a function to be called during simulation execution
    typedef void (*progress_callback)(const mcdriver &v, void *p);

    struct run_data
    {
        std::string start_time;
        std::string end_time;
        double ips;
        double cpu_time;
        int nthreads;
        size_t run_ion_count;
        size_t total_ion_count;
    };

protected:
    std::vector<run_data> run_history_;

    // config
    mcconfig config_;

    // the simulation object
    mccore *s_;

    // The following 2 are populated when the
    // simulation runs
    // 1. threads
    std::vector<std::thread> thread_pool_;
    // 2. simulation execution clones
    std::vector<mccore *> sim_clones_;

public:
    mcdriver();
    ~mcdriver();

    /**
     * @brief Get the currently active configuration
     */
    const mcconfig &config() const { return config_; }

    /**
     * @brief Initialize the driver with the given options
     *
     * This functions first calls mcdriver::reset() to
     * kill and delete the current simulation, if it exists.
     *
     * Then it creates a new simulation according to the
     * options in @a config.
     *
     * @param config A \ref mcconfig struct with the required specs
     */
    void init(const mcconfig &config);

    std::string outFileName() const;

    /// Set the output options.
    void setOutputOptions(const mcconfig::output_options &opts) { config_.Output = opts; }
    /// Set the run options.
    void setRunOptions(const mcconfig::run_options &opts) { config_.Run = opts; }
    /// Returns a const pointer to the mccore simulation object
    const mccore *getSim() const { return s_; }
    /// Returns true if the simulation is running
    bool is_running() const { return thread_pool_.size() > 0; }
    /// Signal a running simulation to abort
    void abort();
    /// Wait for a running simulation to finish
    void wait();
    /// Abort and delete the current simulation.
    void reset();
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
     * @brief Load a simulation from a HDF5 file
     * @param h5filename the name of the file
     * @param os optional stream pointer to write any error messages
     * @return 0 if succesfull
     */
    int load(const std::string &h5filename, std::ostream *os = nullptr);

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
