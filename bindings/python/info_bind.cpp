/*
 * info_bind.cpp
 *
 * Binds opentrim.Info - the read side of the API.
 *
 * mcinfo is a self-describing tree: every node carries a type, a description,
 * child nodes, and typed getter lambdas that pull live data from the driver at
 * query time.  Rather than bind one Python attribute per result, we bind the
 * generic node once.  keys() / __getitem__ then walk the whole tree, so any
 * key mcinfo.cpp adds in the future shows up in Python with no change here.
 */

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "driver_bind.h"
#include "mcinfo.h"

namespace py = pybind11;

namespace {

const char *type_name(mcinfo::info_t t)
{
    switch (t) {
    case mcinfo::group: return "group";
    case mcinfo::string: return "string";
    case mcinfo::json: return "json";
    case mcinfo::real64: return "real64";
    case mcinfo::real32: return "real32";
    case mcinfo::uint64: return "uint64";
    case mcinfo::tally_score: return "tally_score";
    default: return "invalid";
    }
}

// build an owning numpy array shaped by dim.  mcinfo returns copies, so Python
// never holds a pointer into C++ memory.
template <class T>
py::array_t<T> make_array(const std::vector<T> &v, const mcinfo::dim_t &dim)
{
    std::vector<py::ssize_t> shape(dim.begin(), dim.end());

    size_t n = 1;
    for (size_t d : dim)
        n *= d;
    // fall back to a flat shape if the reported dim does not match the data -
    // better a correct 1-D array than a buffer overrun on the memcpy.
    if (n != v.size())
        shape = { (py::ssize_t)v.size() };

    py::array_t<T> arr(shape);
    if (!v.empty())
        std::memcpy(arr.mutable_data(), v.data(), v.size() * sizeof(T));
    return arr;
}

// mcinfo does not flag whether a string node holds one value or a list, and a
// single-element list looks identical to a scalar (both report dim {1}).  these
// are the string leaves that are genuinely scalar in mcinfo.cpp; everything else
// (material names, atom symbols, bin names, ...) is a list, even with one entry.
// a future is_scalar flag on mcinfo would make this lookup unnecessary.
bool is_scalar_string_key(const std::string &key)
{
    // "decription" / "event_decription" are spelled that way in mcinfo.cpp -
    // these must match the source keys exactly, so do not "correct" them.
    static const std::unordered_set<std::string> scalar_keys = {
        "title", "json_config", "run_history", "version", "git_tag", "compiler",
        "compiler_version", "build_system", "build_time", "decription", "event",
        "event_decription"
    };
    return scalar_keys.count(key) != 0;
}

// read a leaf node's value.  groups are handled by __getitem__ and never reach
// here.  key is the node's own name, used only to tell scalar strings from lists.
py::object leaf_value(const mcinfo &n, const std::string &key)
{
    mcinfo::dim_t dim;
    switch (n.type()) {
    case mcinfo::string:
    case mcinfo::json: {
        std::vector<std::string> s;
        n.get(s, dim);
        // json is always a single document; named scalar fields collapse to a
        // plain str.  list-valued fields stay a list even with one element.
        if (n.type() == mcinfo::json || is_scalar_string_key(key))
            return py::str(s.empty() ? std::string() : s[0]);
        py::list out;
        for (const auto &x : s)
            out.append(py::str(x));
        return out;
    }
    case mcinfo::real64: {
        std::vector<double> s;
        n.get(s, dim);
        return make_array(s, dim);
    }
    case mcinfo::real32: {
        std::vector<float> s;
        n.get(s, dim);
        return make_array(s, dim);
    }
    case mcinfo::uint64: {
        std::vector<uint64_t> s;
        n.get(s, dim);
        return make_array(s, dim);
    }
    case mcinfo::tally_score: {
        // tally data is returned as (values, sem) - both normalized per ion.
        std::vector<double> s, ds;
        if (!n.get(s, ds, dim))
            throw std::runtime_error("tally data unavailable; run the simulation first");
        return py::make_tuple(make_array(s, dim), make_array(ds, dim));
    }
    default:
        throw std::runtime_error("info node has no readable value");
    }
}

// walk the static tree structure for __repr__.  this reads node metadata only
// (type, name, children) and never calls a getter, so it has no side effects and
// is safe to call even while a simulation is running.
void repr_tree(const mcinfo &n, const std::string &prefix, std::string &out)
{
    const auto &kids = n.children();
    size_t i = 0, last = kids.size();
    for (const auto &kv : kids) {
        bool is_last = (++i == last);
        out += prefix;
        out += is_last ? "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 " // "└── "
                       : "\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 "; // "├── "
        out += kv.first;
        out += " [";
        out += type_name(kv.second.type());
        out += "]\n";
        if (kv.second.type() == mcinfo::group)
            repr_tree(kv.second, prefix + (is_last ? "    " : "\xe2\x94\x82   "), out);
    }
}

std::string render_tree(const mcinfo &n)
{
    std::string out = "Info [";
    out += type_name(n.type());
    out += "]\n";
    repr_tree(n, "", out);
    return out;
}

// escape the few characters that would otherwise break the HTML.  node names are
// usually code identifiers, but user_tally ids come from user config.
void html_escape(const std::string &in, std::string &out)
{
    for (char c : in) {
        switch (c) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        default: out += c;
        }
    }
}

// render the tree as nested <details> so Jupyter shows a collapsible view.  like
// repr_tree this reads metadata only and never calls a getter.
void html_tree(const mcinfo &n, std::string &out)
{
    out += "<ul style=\"list-style:none;margin:0;padding-left:1.2em\">";
    for (const auto &kv : n.children()) {
        const mcinfo &child = kv.second;
        out += "<li>";
        if (child.type() == mcinfo::group) {
            out += "<details><summary><b>";
            html_escape(kv.first, out);
            out += "</b> <span style=\"color:#888\">[group]</span></summary>";
            html_tree(child, out);
            out += "</details>";
        } else {
            html_escape(kv.first, out);
            out += " <span style=\"color:#888\">[";
            out += type_name(child.type());
            out += "]</span>";
        }
        out += "</li>";
    }
    out += "</ul>";
}

} // namespace

void bind_info(py::module_ &m)
{
    py::class_<mcinfo> info(m, "Info",
        "Read-only view of simulation results.  Wraps the C++ mcinfo tree.\n\n"
        "Example::\n\n"
        "    info = opentrim.Info(sim)            # sim is an initialized Driver\n"
        "    v, dv = info[\"tally\"][\"damage_events\"][\"Vacancies\"]\n\n"
        "Tally nodes return (values, sem) numpy tuples normalized per ion.\n"
        "Note: reading results while a Mode A run is in progress may race;\n"
        "prefer calling sim.wait() first.");

    info.def(py::init([](py::object driver_obj) {
                 DriverWrapper &w = driver_obj.cast<DriverWrapper &>();
                 // mcinfo's constructor dereferences getSim() (mcinfo.cpp:442),
                 // so the driver must be initialized before we build the tree.
                 if (!w.driver().getSim())
                     throw std::runtime_error(
                             "driver not initialized; call init(config) before Info(driver)");
                 return new mcinfo(&w.driver());
             }),
             py::arg("driver"),
             // keep the Driver alive as long as this Info exists - mcinfo holds a
             // raw mcdriver* and reads through it on every access.
             py::keep_alive<1, 2>(),
             "Build an Info view over an initialized Driver.");

    info.def("keys",
             [](const mcinfo &n) {
                 std::vector<std::string> k;
                 for (const auto &kv : n.children())
                     k.push_back(kv.first);
                 return k;
             },
             "Available keys at this level, in definition order.");

    info.def("__getitem__",
             [](py::object self, const std::string &key) -> py::object {
                 const mcinfo &node = self.cast<const mcinfo &>();
                 // mcinfo::operator[] silently inserts on a missing key
                 // (mcinfo.h:42), so look it up through children().at() instead.
                 const mcinfo *child;
                 try {
                     child = &node.children().at(key);
                 } catch (const std::out_of_range &) {
                     throw py::key_error(key);
                 }
                 if (child->type() == mcinfo::group)
                     // return the sub-node, tied to this Info's lifetime.
                     return py::cast(child, py::return_value_policy::reference_internal, self);
                 return leaf_value(*child, key);
             },
             py::arg("key"),
             "Return a sub-Info for a group, or the value for a leaf node.");

    info.def("__contains__",
             [](const mcinfo &n, const std::string &key) {
                 return n.children().find(key) != n.children().end();
             });

    info.def("__len__", [](const mcinfo &n) { return n.children().size(); });

    info.def("__iter__",
             [](py::object self) { return py::iter(self.attr("keys")()); });

    info.def_property_readonly(
            "type", [](const mcinfo &n) { return std::string(type_name(n.type())); },
            "Node type: group, string, json, real64, real32, uint64, or tally_score.");

    info.def_property_readonly(
            "description", [](const mcinfo &n) { return n.description(); },
            "Human-readable description of this node.");

    info.def_static("info_spec", []() { return mcinfo::info_spec(); },
                    "Return the JSON spec of the full info tree structure.");

    // structural tree only - reads node metadata, never calls a getter, so it has
    // no side effects and is safe to print even while a simulation is running.
    info.def("__repr__", [](const mcinfo &n) { return render_tree(n); });
    info.def("__str__", [](const mcinfo &n) { return render_tree(n); });

    // Jupyter rich display.  the protocol method is _repr_html_ (single
    // underscores); same side-effect-free structural walk as __repr__.
    info.def("_repr_html_", [](const mcinfo &n) {
        std::string out = "<div style=\"font-family:monospace\"><b>Info</b> "
                          "<span style=\"color:#888\">[";
        out += type_name(n.type());
        out += "]</span>";
        html_tree(n, out);
        out += "</div>";
        return out;
    });
}
