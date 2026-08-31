#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include <memory>
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

DriverWrapper::DriverWrapper(const mcconfig &config)
{
    // create() validates the config and builds the simulation; on failure it
    // returns an empty pointer and writes the reason to the stream.
    std::ostringstream err;
    driver_ = mcdriver::create(config, &err);
    if (!driver_)
        throw std::invalid_argument("cannot create Driver: " + err.str());
}

DriverWrapper *DriverWrapper::from_file(const std::string &filename)
{
    std::ostringstream err;
    std::shared_ptr<mcdriver> d = mcdriver::load(filename, &err);
    if (!d)
        throw std::runtime_error("load failed: " + err.str());
    return new DriverWrapper(std::move(d));
}

DriverWrapper::~DriverWrapper()
{
    if (exec_running_.load())
        driver_->abort();
    join_exec_thread();
}

void DriverWrapper::join_exec_thread()
{
    if (exec_thread_.joinable()) {
        // exec() does not touch Python, so drop the GIL while we block on the
        // join. Otherwise wait() would freeze the whole interpreter.
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

void DriverWrapper::run_mode_a()
{
    if (exec_running_.load())
        throw std::runtime_error("simulation already running; call wait() first");
    join_exec_thread(); // reap a previously finished run
    raise_exec_error_if_any(); // surface a prior failed run before starting again

    exec_running_.store(true);
    exec_thread_ = std::thread([this] {
        // an exception escaping a std::thread calls std::terminate, so catch it
        // here and re-raise it from wait().
        try {
            driver_->exec();
        } catch (...) {
            exec_error_ = std::current_exception();
        }
        exec_running_.store(false);
    });
}

void DriverWrapper::progress_trampoline(const mcdriver *, void *user_data)
{
    auto *ctx = static_cast<CallbackContext *>(user_data);
    if (ctx->error)
        return; // a previous callback already failed. Stop calling Python.

    py::gil_scoped_acquire gil;
    try {
        ctx->cb();
    } catch (py::error_already_set &) {
        // capture the python error and abort instead of letting it unwind through
        // exec(), which would skip the worker joins and wedge the driver.
        // run_mode_b() re-raises it after exec() returns cleanly.
        ctx->error = std::current_exception();
        ctx->driver.abort();
    }
}

void DriverWrapper::run_mode_b(py::function cb, size_t interval_ms)
{
    if (exec_running_.load())
        throw std::runtime_error("simulation already running; call wait() first");
    join_exec_thread();
    raise_exec_error_if_any(); // surface a prior failed Mode A run

    CallbackContext ctx{ cb, *driver_, nullptr };
    exec_running_.store(true);
    try {
        py::gil_scoped_release release;
        driver_->exec(&DriverWrapper::progress_trampoline, interval_ms, &ctx);
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

void DriverWrapper::abort()
{
    driver_->abort(); // no-op if no simulation is active
}

void DriverWrapper::wait()
{
    join_exec_thread();
    raise_exec_error_if_any(); // surface a Mode A exec() failure
}

void DriverWrapper::save(const std::string &filename)
{
    if (exec_running_.load())
        throw std::runtime_error("cannot save while a simulation is running; call wait() first");
    // save() walks an mcinfo tree that normalizes tallies by ion_count; with zero
    // ions that divides by zero and writes NaN, so require at least one run ion.
    if (driver_->ion_count() == 0)
        throw std::runtime_error("nothing to save; run at least one ion before save()");

    std::ostringstream err;
    int rc;
    {
        // file i/o touches no Python, so drop the GIL while other threads run.
        py::gil_scoped_release release;
        rc = driver_->save(filename, &err);
    }
    if (rc != 0)
        throw std::runtime_error("save failed: " + err.str());
}

long long DriverWrapper::max_ions() const
{
    return (long long)driver_->max_no_ions();
}

void DriverWrapper::set_max_ions(long long v)
{
    if (v < 0)
        throw std::invalid_argument("max_ions must be >= 0");
    // mcdriver allows this during a run; the value takes effect on the next run.
    driver_->setMaxNoIons((size_t)v);
}

long long DriverWrapper::max_cpu_time() const
{
    return (long long)driver_->config().Run.max_cpu_time;
}

void bind_driver(py::module_ &m)
{
    py::class_<DriverWrapper> drv(m, "Driver",
        "Runs an OpenTRIM simulation.  Wraps the C++ mcdriver.\n\n"
        "A driver is created from a Config and is ready to run immediately.\n\n"
        "Example::\n\n"
        "    sim = opentrim.Driver(config)\n"
        "    sim.run()          # Mode A - non-blocking\n"
        "    sim.wait()");

    drv.def(py::init<const mcconfig &>(), py::arg("config"),
            "Create a driver from a Config.  Raises ValueError if the config is\n"
            "invalid.");

    drv.def_static("load", &DriverWrapper::from_file, py::arg("filename"),
                   "Load a saved simulation from an HDF5 file and return a new Driver.\n"
                   "The run can be continued with run() and read with Info(sim).");

    drv.def("config", &DriverWrapper::config,
            "Return a copy of the current configuration as a Config.");

    // Mode A: non-blocking.  exec() runs on a background thread.
    drv.def("run", &DriverWrapper::run_mode_a,
            "Mode A: start the simulation and return immediately.  exec() runs on\n"
            "a background thread; use wait() to block until it finishes.");

    // Mode B: blocking with a Python callback.  The GIL is released during
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
            "Number of ions simulated so far.");

    drv.def("abort", &DriverWrapper::abort,
            "Signal a running simulation to stop gracefully.");

    drv.def("wait", &DriverWrapper::wait,
            "Block until the Mode A run thread finishes.  No-op otherwise.");

    drv.def("save", &DriverWrapper::save, py::arg("filename"),
            "Save the simulation and all results to an HDF5 file.  Same format as\n"
            "the CLI output; reload it with Driver.load().  Requires at least one\n"
            "simulated ion (tallies are normalized per ion).");

    drv.def_property("max_ions", &DriverWrapper::max_ions, &DriverWrapper::set_max_ions,
                     "Maximum ions to simulate (config.Run.max_no_ions).  May be changed\n"
                     "between runs; takes effect on the next run().");

    drv.def_property_readonly("max_cpu_time", &DriverWrapper::max_cpu_time,
                              "Maximum CPU time [s] from the config.  0 = unlimited.");

    drv.def("__repr__", [](const DriverWrapper &d) {
        return "Driver(running=" + std::string(d.is_running() ? "True" : "False")
            + ", ion_count=" + std::to_string(d.ion_count()) + ")";
    });
}
