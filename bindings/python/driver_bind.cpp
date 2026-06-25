#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include "driver_bind.h"

namespace py = pybind11;

// user data carried through exec() to the callback trampoline
struct DriverWrapper::CallbackContext
{
    py::function &cb;
    mcdriver &driver;
    std::exception_ptr error;
};

DriverWrapper::~DriverWrapper()
{
    if (exec_running_.load())
        driver_.abort();
    join_exec_thread();
}

void DriverWrapper::join_exec_thread()
{
    if (exec_thread_.joinable()) {
        // exec() does not touch Python, so drop the GIL while we block on the
        // join - otherwise wait() would freeze the whole interpreter.
        py::gil_scoped_release release;
        exec_thread_.join();
    }
}

void DriverWrapper::raise_exec_error_if_any()
{
    // re-raise an exception captured by the Mode A thread so a failed background
    // run is never silently lost. clears the slot so it surfaces only once.
    if (exec_error_) {
        std::exception_ptr e = exec_error_;
        exec_error_ = nullptr;
        std::rethrow_exception(e);
    }
}

void DriverWrapper::ensure_initialized() const
{
    // exec() dereferences mccore at mcdriver.cpp:115 without a null check, so a
    // run() before init() would segfault - guard it here.
    if (!driver_.getSim())
        throw std::runtime_error("driver not initialized; call init(config) before run()");
}

void DriverWrapper::init(const mcconfig &config)
{
    if (exec_running_.load())
        throw std::runtime_error("cannot init() while a simulation is running");
    join_exec_thread();
    exec_error_ = nullptr; // old error belongs to the simulation we are replacing
    driver_.init(config);
}

void DriverWrapper::run_mode_a()
{
    if (exec_running_.load())
        throw std::runtime_error("simulation already running; call wait() first");
    ensure_initialized();
    join_exec_thread(); // reap a previously finished run
    raise_exec_error_if_any(); // surface a prior failed run before starting again

    exec_running_.store(true);
    exec_thread_ = std::thread([this] {
        // an exception escaping a std::thread calls std::terminate, so catch it
        // here and re-raise it from wait().
        try {
            driver_.exec();
        } catch (...) {
            exec_error_ = std::current_exception();
        }
        exec_running_.store(false);
    });
}

void DriverWrapper::progress_trampoline(const mcdriver &, void *user_data)
{
    auto *ctx = static_cast<CallbackContext *>(user_data);
    if (ctx->error)
        return; // a previous callback already failed - stop calling Python

    py::gil_scoped_acquire gil;
    try {
        ctx->cb();
    } catch (py::error_already_set &) {
        // capture the python error and abort instead of letting it unwind through
        // exec() - that would skip the worker joins at mcdriver.cpp:199 and wedge
        // the driver. run_mode_b() re-raises it after exec() returns cleanly.
        ctx->error = std::current_exception();
        ctx->driver.abort();
    }
}

void DriverWrapper::run_mode_b(py::function cb, size_t interval_ms)
{
    if (exec_running_.load())
        throw std::runtime_error("simulation already running; call wait() first");
    ensure_initialized();
    join_exec_thread();
    raise_exec_error_if_any(); // surface a prior failed Mode A run

    CallbackContext ctx{ cb, driver_, nullptr };
    exec_running_.store(true);
    try {
        py::gil_scoped_release release;
        driver_.exec(&DriverWrapper::progress_trampoline, interval_ms, &ctx);
    } catch (...) {
        // clear the flag even if exec() throws, then let the error propagate
        exec_running_.store(false);
        throw;
    }
    exec_running_.store(false);

    // GIL is held again here, so the Python exception re-raises correctly.
    if (ctx.error)
        std::rethrow_exception(ctx.error);
}

size_t DriverWrapper::ion_count() const
{
    const mccore *s = driver_.getSim();
    return s ? s->ion_count() : 0;
}

void DriverWrapper::abort()
{
    driver_.abort(); // no-op if no simulation is active
}

void DriverWrapper::wait()
{
    join_exec_thread();
    raise_exec_error_if_any(); // surface a Mode A exec() failure
}

void DriverWrapper::reset()
{
    driver_.abort();
    join_exec_thread();
    exec_error_ = nullptr; // state is being cleared - drop any pending error
    driver_.reset();
}

void DriverWrapper::save(const std::string &filename)
{
    if (exec_running_.load())
        throw std::runtime_error("cannot save while a simulation is running; call wait() first");
    // save() walks an mcinfo tree that normalizes tallies by ion_count
    // (mcinfo.cpp:592); with zero ions that divides by zero and writes NaN, so
    // require at least one simulated ion - not just an initialized driver.
    const mccore *sim = driver_.getSim();
    if (!sim || sim->ion_count() == 0)
        throw std::runtime_error("nothing to save; run at least one ion before save()");

    std::ostringstream err;
    int rc;
    {
        // file i/o touches no Python - drop the GIL so other threads run.
        py::gil_scoped_release release;
        rc = driver_.save(filename, &err);
    }
    if (rc != 0)
        throw std::runtime_error("save failed: " + err.str());
}

void DriverWrapper::load(const std::string &filename)
{
    if (exec_running_.load())
        throw std::runtime_error("cannot load while a simulation is running; call wait() first");
    // mcdriver::load refuses if a simulation already exists (h5serialize.cpp:303),
    // so clear any current state first.  load() then rebuilds the sim through init(),
    // leaving getSim() valid for Info(driver) and allowing run() to continue.
    reset();

    std::ostringstream err;
    int rc;
    {
        py::gil_scoped_release release;
        rc = driver_.load(filename, &err);
    }
    if (rc != 0)
        throw std::runtime_error("load failed: " + err.str());
}

long long DriverWrapper::max_ions() const
{
    return (long long)driver_.config().Run.max_no_ions;
}

void DriverWrapper::set_max_ions(long long v)
{
    if (v < 0)
        throw std::invalid_argument("max_ions must be >= 0");
    if (exec_running_.load())
        throw std::runtime_error("cannot change max_ions while a simulation is running");
    mcconfig::run_options ro = driver_.config().Run;
    ro.max_no_ions = (size_t)v;
    driver_.setRunOptions(ro);
}

long long DriverWrapper::max_cpu_time() const
{
    return (long long)driver_.config().Run.max_cpu_time;
}

void DriverWrapper::set_max_cpu_time(long long v)
{
    if (v < 0)
        throw std::invalid_argument("max_cpu_time must be >= 0");
    if (exec_running_.load())
        throw std::runtime_error("cannot change max_cpu_time while a simulation is running");
    mcconfig::run_options ro = driver_.config().Run;
    ro.max_cpu_time = (size_t)v;
    driver_.setRunOptions(ro);
}

void bind_driver(py::module_ &m)
{
    py::class_<DriverWrapper> drv(m, "Driver",
        "Runs an OpenTRIM simulation.  Wraps the C++ mcdriver.\n\n"
        "Example::\n\n"
        "    sim = opentrim.Driver()\n"
        "    sim.init(config)\n"
        "    sim.run()          # Mode A - non-blocking\n"
        "    sim.wait()");
    drv.def(py::init<>());

    drv.def("init", &DriverWrapper::init, py::arg("config"),
            "Initialize the driver from a Config.  Resets any current simulation.");

    drv.def("config", &DriverWrapper::config,
            "Return a copy of the current configuration as a Config.");

    // Mode A - non-blocking.  exec() runs on a background thread.
    drv.def("run", &DriverWrapper::run_mode_a,
            "Mode A: start the simulation and return immediately.  exec() runs on\n"
            "a background thread; use wait() to block until it finishes.");

    // Mode B - blocking with a Python callback.  The GIL is released during
    // exec() and re-acquired for each callback.  Ctrl+C raises KeyboardInterrupt
    // at the next callback boundary and aborts the run.
    drv.def("run", &DriverWrapper::run_mode_b, py::arg("callback"),
            py::arg("interval_ms") = 1000,
            "Mode B: run the simulation on the calling thread and call callback()\n"
            "every interval_ms milliseconds.  The GIL is released during the run.\n"
            "interval_ms is quantized to 100 ms ticks (minimum effective 100 ms).");

    drv.def("is_running", &DriverWrapper::is_running,
            "True while a simulation is active.  Tracked by the binding, not by\n"
            "mcdriver::is_running() (which is unreliable during setup).");

    drv.def("ion_count", &DriverWrapper::ion_count,
            "Ions simulated so far.  Delegates to mccore::ion_count(); 0 if no\n"
            "simulation has been created yet.");

    drv.def("abort", &DriverWrapper::abort,
            "Signal a running simulation to stop gracefully.");

    drv.def("wait", &DriverWrapper::wait,
            "Block until the Mode A run thread finishes.  No-op otherwise.");

    drv.def("reset", &DriverWrapper::reset,
            "Abort the running simulation and clear all simulation state.");

    drv.def("save", &DriverWrapper::save, py::arg("filename"),
            "Save the simulation and all results to an HDF5 file.  Same format as\n"
            "the CLI output; reload it with load().  Requires at least one\n"
            "simulated ion (tallies are normalized per ion).");

    drv.def("load", &DriverWrapper::load, py::arg("filename"),
            "Load a simulation from an HDF5 file, replacing any current state.\n"
            "Afterwards results are available via Info(sim) and the run can be\n"
            "continued with run().");

    drv.def_property("max_ions", &DriverWrapper::max_ions, &DriverWrapper::set_max_ions,
                     "Maximum ions to simulate (config.Run.max_no_ions).  Read once at\n"
                     "exec() start; changes take effect on the next run().");

    drv.def_property("max_cpu_time", &DriverWrapper::max_cpu_time,
                     &DriverWrapper::set_max_cpu_time,
                     "Maximum wall-clock time [s].  0 = unlimited.");

    drv.def("__repr__", [](const DriverWrapper &d) {
        return "Driver(running=" + std::string(d.is_running() ? "True" : "False")
            + ", ion_count=" + std::to_string(d.ion_count()) + ")";
    });
}
