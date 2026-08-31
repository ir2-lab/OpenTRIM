#include <octave/oct.h>
#include <octave/parse.h>
#include <mcdriver.h>
#include "class_handle.h"

// Data passed to the C-style progress callback via void*.
struct OctaveCallbackData
{
    octave_value cb_func;
    bool interrupted = false;
};

// Called from the main thread by mcdriver::exec() at each progress interval.
static void progress_cb_wrapper(const mcdriver *d, void *userdata)
{
    if (!userdata)
        return;
    auto *cbd = static_cast<OctaveCallbackData *>(userdata);
    if (!cbd->cb_func.is_function_handle())
        return;

    double frac = 0.0;
    const auto &hist = d->run_history();
    if (!hist.empty()) {
        size_t max_ions = d->config().Run.max_no_ions;
        if (max_ions > 0)
            frac = static_cast<double>(hist.back().run_ion_count) / static_cast<double>(max_ions);
    }

    try {
        octave_value_list call_args;
        call_args(0) = octave_value(frac);
        octave::feval(cbd->cb_func.function_value(), call_args, 0);
    } catch (const octave::interrupt_exception &) {
        // Ctrl+C: signal the simulation to stop, then let exec() finish
        // cleanly before re-throwing the interrupt to Octave.
        const_cast<mcdriver *>(d)->abort();
        cbd->interrupted = true;
    } catch (...) {
    }
}

DEFUN_DLD(__opentrim_driver__, args, /* nargout */,
          "opentrim.driver dispatcher — do not call directly")
{
    octave_value_list retval;

    if (args.length() < 1 || !args(0).is_string())
        error("__opentrim_driver__: first argument must be a command string");

    const std::string cmd = args(0).string_value();

    try {
        if (cmd == "new") {
            if (args.length() < 2)
                error("opentrim.driver.new: requires (config_handle)");
            const mcconfig *cfg = handle_obj<mcconfig>(args(1));
            retval(0) = shared_handle_create(mcdriver::create(*cfg));

        } else if (cmd == "delete") {
            shared_handle_destroy<mcdriver>(args(1));

        } else if (cmd == "exec") {
            mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            octave_value cb_val;
            size_t ms = 1000;
            if (args.length() > 2)
                cb_val = args(2);
            if (args.length() > 3)
                ms = static_cast<size_t>(args(3).uint64_value());

            // Always install the callback so octave::feval is called
            // periodically, which lets Octave detect Ctrl+C mid-run.
            OctaveCallbackData cbd{ cb_val };
            int ret = d->exec(progress_cb_wrapper, ms, &cbd);
            if (ret != 0)
                error("opentrim.driver.exec: simulation returned error %d", ret);
            if (cbd.interrupted)
                throw octave::interrupt_exception();

        } else if (cmd == "config") {
            // Return a copy of the internal config as a new mcconfig handle.
            const mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            retval(0) = handle_create(new mcconfig(d->config()));

        } else if (cmd == "save") {
            if (args.length() < 3)
                error("opentrim.driver.save: requires (handle, filename)");
            mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            const std::string fn = args(2).string_value();
            std::ostringstream err;
            if (d->save(fn, &err) != 0)
                error("opentrim.driver.save: %s", err.str().c_str());

        } else if (cmd == "load") {
            if (args.length() < 3)
                error("opentrim.driver.load: requires (filename)");

            const std::string fn = args(1).string_value();
            std::ostringstream err;
            auto d = mcdriver::load(fn, &err);
            if (!d)
                error("opentrim.driver.load: %s", err.str().c_str());
            retval(0) = shared_handle_create(d);

        } else if (cmd == "is_running") {
            const mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            retval(0) = octave_value(d->is_running());

        } else if (cmd == "abort") {
            shared_handle_obj<mcdriver>(args(1))->abort();

        } else if (cmd == "wait") {
            shared_handle_obj<mcdriver>(args(1))->wait();

        } else if (cmd == "ion_count") {
            const mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            retval(0) = octave_value(d->ion_count());

        } else if (cmd == "max_no_ions") {
            const mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            retval(0) = octave_value(d->max_no_ions());

        } else if (cmd == "setMaxNoIons") {
            if (args.length() < 3)
                error("opentrim.driver.setMaxNoIons: requires (handle, n)");
            mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            d->setMaxNoIons(static_cast<size_t>(args(2).uint64_value()));

        } else if (cmd == "threads") {
            const mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            retval(0) = octave_value(d->threads());

        } else if (cmd == "setNthreads") {
            if (args.length() < 3)
                error("opentrim.driver.setNthreads: requires (handle, n)");
            mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            d->setNthreads(static_cast<size_t>(args(2).uint64_value()));

        } else if (cmd == "seed") {
            const mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            retval(0) = octave_value(d->seed());

        } else if (cmd == "setSeed") {
            if (args.length() < 3)
                error("opentrim.driver.setSeed: requires (handle, n)");
            mcdriver *d = shared_handle_obj<mcdriver>(args(1)).get();
            bool ok = d->setSeed(static_cast<unsigned int>(args(2).uint_value()));
            retval(0) = octave_value(ok);

        } else {
            error("__opentrim_driver__: unknown command '%s'", cmd.c_str());
        }

    } catch (const std::exception &e) {
        error("opentrim.driver: %s", e.what());
    }

    return retval;
}
