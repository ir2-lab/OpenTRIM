#include <octave/oct.h>
#include <sstream>
#include <mcdriver.h>
#include "class_handle.h"

DEFUN_DLD(__opentrim_config__, args, /* nargout */,
          "opentrim.config dispatcher — do not call directly")
{
    octave_value_list retval;

    if (args.length() < 1 || !args(0).is_string())
        error("__opentrim_config__: first argument must be a command string");

    const std::string cmd = args(0).string_value();

    try {
        if (cmd == "new") {
            retval(0) = handle_create(new mcconfig());

        } else if (cmd == "delete") {
            handle_destroy<mcconfig>(args(1));

        } else if (cmd == "set") {
            if (args.length() < 4)
                error("opentrim.config.set: requires (handle, path, json_value)");
            mcconfig *cfg = handle_obj<mcconfig>(args(1));
            const std::string path = args(2).string_value();
            const std::string json_str = args(3).string_value();
            std::ostringstream err;
            if (!cfg->set(path, json_str, &err))
                error("opentrim.config.set('%s'): %s", path.c_str(), err.str().c_str());

        } else if (cmd == "get") {
            if (args.length() < 3)
                error("opentrim.config.get: requires (handle, path)");
            const mcconfig *cfg = handle_obj<mcconfig>(args(1));
            const std::string path = args(2).string_value();
            std::string json_str;
            std::ostringstream err;
            if (!cfg->get(path, json_str, &err))
                error("opentrim.config.get('%s'): %s", path.c_str(), err.str().c_str());
            retval(0) = octave_value(json_str);

        } else if (cmd == "validate") {
            mcconfig *cfg = handle_obj<mcconfig>(args(1));
            int ret = cfg->validate();
            retval(0) = octave_value(ret == 0);

        } else if (cmd == "to_json") {
            const mcconfig *cfg = handle_obj<mcconfig>(args(1));
            retval(0) = octave_value(cfg->toJSON());

        } else if (cmd == "from_json") {
            if (args.length() < 3)
                error("opentrim.config.from_json: requires (handle, json_string)");
            mcconfig *cfg = handle_obj<mcconfig>(args(1));
            const std::string json_str = args(2).string_value();
            std::istringstream ss(json_str);
            std::ostringstream err;
            int ret = cfg->parseJSON(ss, true, &err, false);
            if (ret != 0)
                error("opentrim.config.from_json: %s", err.str().c_str());

        } else {
            error("__opentrim_config__: unknown command '%s'", cmd.c_str());
        }

    } catch (const std::exception &e) {
        error("opentrim.config: %s", e.what());
    }

    return retval;
}
