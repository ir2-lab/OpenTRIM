#ifndef OPENTRIM_DRIVER_BIND_H
#define OPENTRIM_DRIVER_BIND_H

#include <pybind11/pybind11.h>

#include <thread>
#include <atomic>
#include <exception>
#include <memory>
#include <string>

#include "mcdriver.h"

namespace py = pybind11;

// Python-side wrapper around mcdriver.
//
// mcdriver is created from a config and is always in a valid, initialized state
// (mcdriver::create()).  It is used only through a shared_ptr, which mcinfo also
// holds, so an Info view keeps the driver alive on its own.
//
// mcdriver::exec() is a blocking call that spawns its own worker pool and joins
// it before returning.  The wrapper adds the state the Python API needs:
//
//  - exec_thread_   runs exec() off the calling thread so Mode A run() can return
//                   immediately.
//  - exec_running_  reliable run flag.  mcdriver::is_running() reads
//                   thread_pool_.size(), which is still 0 during setup and would
//                   report false negatives.
class DriverWrapper
{
public:
    // Create a driver from a configuration.  Throws if the config is invalid.
    explicit DriverWrapper(const mcconfig &config);
    ~DriverWrapper();

    DriverWrapper(const DriverWrapper &) = delete;
    DriverWrapper &operator=(const DriverWrapper &) = delete;

    // Load a saved simulation from an HDF5 file into a new driver.
    static DriverWrapper *from_file(const std::string &filename);

    mcconfig config() const { return driver_->config(); }

    // Mode A: launch exec() on exec_thread_ and return immediately.
    void run_mode_a();
    // Mode B: run exec() on the calling thread, release the GIL, and call back
    // into Python every interval_ms.
    void run_mode_b(py::function cb, size_t interval_ms);

    bool is_running() const { return exec_running_.load(); }
    size_t ion_count() const { return driver_->ion_count(); }

    void abort();
    void wait();

    // Save the simulation and results to an HDF5 file.
    void save(const std::string &filename);

    long long max_ions() const;
    void set_max_ions(long long v);
    long long max_cpu_time() const;

    // Shared driver, used by opentrim.Info to build the mcinfo tree.  mcinfo
    // takes a shared_ptr, so the driver outlives this wrapper if an Info exists.
    std::shared_ptr<mcdriver> driver() const { return driver_; }

private:
    struct CallbackContext;
    static void progress_trampoline(const mcdriver *d, void *user_data);

    void join_exec_thread();
    void raise_exec_error_if_any();

    // Adopt an already-created driver (used by from_file()).
    explicit DriverWrapper(std::shared_ptr<mcdriver> d) : driver_(std::move(d)) { }

    std::shared_ptr<mcdriver> driver_;
    std::thread exec_thread_;
    std::atomic<bool> exec_running_{ false };
    // exception thrown by exec() on the Mode A thread, re-raised by wait()
    std::exception_ptr exec_error_;
};

#endif // OPENTRIM_DRIVER_BIND_H
