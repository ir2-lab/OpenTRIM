#ifndef OPENTRIM_DRIVER_BIND_H
#define OPENTRIM_DRIVER_BIND_H

#include <pybind11/pybind11.h>

#include <thread>
#include <atomic>
#include <exception>

#include "mcdriver.h"

namespace py = pybind11;

// Python-side wrapper around mcdriver.
//
// mcdriver::exec() is a blocking call that spawns its own worker pool and joins
// it before returning.  The wrapper adds the state the Python API needs on top:
//
//  - exec_thread_   runs exec() off the calling thread so Mode A run() can return
//                   immediately.
//  - exec_running_  reliable run flag.  mcdriver::is_running() reads
//                   thread_pool_.size(), which is still 0 during setup (the pool
//                   is filled at mcdriver.cpp:170) and reports false negatives.
//
// opentrim.Info (A-4) reads results through driver() once a run has finished.
class DriverWrapper
{
public:
    DriverWrapper() = default;
    ~DriverWrapper();

    DriverWrapper(const DriverWrapper &) = delete;
    DriverWrapper &operator=(const DriverWrapper &) = delete;

    void init(const mcconfig &config);
    mcconfig config() const { return driver_.config(); }

    // Mode A - launch exec() on exec_thread_ and return immediately.
    void run_mode_a();
    // Mode B - run exec() on the calling thread, release the GIL, and call back
    // into Python every interval_ms.
    void run_mode_b(py::function cb, size_t interval_ms);

    bool is_running() const { return exec_running_.load(); }
    size_t ion_count() const;

    void abort();
    void wait();
    void reset();

    long long max_ions() const;
    void set_max_ions(long long v);
    long long max_cpu_time() const;
    void set_max_cpu_time(long long v);

    // Underlying driver - used by opentrim.Info (A-4) to build mcinfo.
    const mcdriver &driver() const { return driver_; }
    mcdriver &driver() { return driver_; }

private:
    struct CallbackContext;
    static void progress_trampoline(const mcdriver &d, void *user_data);

    void join_exec_thread();
    void raise_exec_error_if_any();
    void ensure_initialized() const;

    mcdriver driver_;
    std::thread exec_thread_;
    std::atomic<bool> exec_running_{ false };
    // exception thrown by exec() on the Mode A thread, re-raised by wait()
    std::exception_ptr exec_error_;
};

#endif // OPENTRIM_DRIVER_BIND_H
