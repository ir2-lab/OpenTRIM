/*
 * info_bind.cpp
 *
 * Binds opentrim.Info, the read side of the API.
 *
 * mcinfo is a self-describing tree of nodes: group nodes (mcinfo) hold children,
 * data nodes (mcinfo_data_node) hold a typed value pulled from the driver at
 * query time.  We bind one generic Info view over a node, so keys()/__getitem__
 * walk the whole tree and any key mcinfo.cpp adds later shows up with no change
 * here.
 *
 * mcinfo holds a shared_ptr to the driver, so an Info keeps the driver alive on
 * its own.  The Python Info view holds a shared_ptr to the root group (which
 * owns the tree and the driver), so a sub-view stays valid as long as it lives.
 */

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "driver_bind.h"
#include "mcinfo.h"

namespace py = pybind11;

namespace {

const char *type_name(const mcinfo_node &n)
{
    if (n.is_group())
        return "group";
    switch (static_cast<const mcinfo_data_node &>(n).type()) {
    case mcinfo_data_node::string: return "string";
    case mcinfo_data_node::json: return "json";
    case mcinfo_data_node::real64: return "real64";
    case mcinfo_data_node::real32: return "real32";
    case mcinfo_data_node::uint64: return "uint64";
    case mcinfo_data_node::tally_score: return "tally_score";
    default: return "invalid";
    }
}

// build an owning numpy array shaped by dim.  mcinfo returns copies, so Python
// never holds a pointer into C++ memory.
template <class T>
py::array_t<T> make_array(const std::vector<T> &v, const std::vector<size_t> &dim)
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

// read a data node's value.  group nodes are handled by __getitem__ and never
// reach here.  key is the node's own name, used only to tell scalar strings from
// lists.
py::object leaf_value(const mcinfo_data_node &d, const std::string &key)
{
    std::vector<size_t> dim = d.dim();
    switch (d.type()) {
    case mcinfo_data_node::string:
    case mcinfo_data_node::json: {
        std::vector<std::string> s;
        d.get(s);
        // json is always a single document; named scalar fields collapse to a
        // plain str.  list-valued fields stay a list even with one element.
        if (d.type() == mcinfo_data_node::json || is_scalar_string_key(key))
            return py::str(s.empty() ? std::string() : s[0]);
        py::list out;
        for (const auto &x : s)
            out.append(py::str(x));
        return out;
    }
    case mcinfo_data_node::real64: {
        std::vector<double> s;
        d.get(s);
        return make_array(s, dim);
    }
    case mcinfo_data_node::real32: {
        std::vector<float> s;
        d.get(s);
        return make_array(s, dim);
    }
    case mcinfo_data_node::uint64: {
        std::vector<uint64_t> s;
        d.get(s);
        return make_array(s, dim);
    }
    case mcinfo_data_node::tally_score: {
        // tally data is returned as (values, sem), both normalized per ion.
        std::vector<double> s, ds;
        if (!d.get(s, ds))
            throw std::runtime_error("tally data unavailable; run the simulation first");
        return py::make_tuple(make_array(s, dim), make_array(ds, dim));
    }
    default:
        throw std::runtime_error("info node has no readable value");
    }
}

// walk the static tree structure for __repr__.  this reads node metadata only
// (name, type, children) and never calls a data getter, so it has no side
// effects and is safe to call even while a simulation is running.
void repr_tree(const mcinfo &g, const std::string &prefix, std::string &out)
{
    std::vector<std::string> keys = g.keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        bool is_last = (i + 1 == keys.size());
        const mcinfo_node *child = g.at(keys[i]);
        out += prefix;
        out += is_last ? "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 " // "└── "
                       : "\xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 "; // "├── "
        out += keys[i];
        out += " [";
        out += type_name(*child);
        out += "]\n";
        if (child->is_group())
            repr_tree(static_cast<const mcinfo &>(*child),
                      prefix + (is_last ? "    " : "\xe2\x94\x82   "), out);
    }
}

std::string render_tree(const mcinfo_node &n)
{
    std::string out = "Info [";
    out += type_name(n);
    out += "]\n";
    if (n.is_group())
        repr_tree(static_cast<const mcinfo &>(n), "", out);
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
// repr_tree this reads metadata only and never calls a data getter.
void html_tree(const mcinfo &g, std::string &out)
{
    out += "<ul style=\"list-style:none;margin:0;padding-left:1.2em\">";
    for (const std::string &key : g.keys()) {
        const mcinfo_node *child = g.at(key);
        out += "<li>";
        if (child->is_group()) {
            out += "<details><summary><b>";
            html_escape(key, out);
            out += "</b> <span style=\"color:#888\">[group]</span></summary>";
            html_tree(static_cast<const mcinfo &>(*child), out);
            out += "</details>";
        } else {
            html_escape(key, out);
            out += " <span style=\"color:#888\">[";
            out += type_name(*child);
            out += "]</span>";
        }
        out += "</li>";
    }
    out += "</ul>";
}

// Python-side Info view.  root owns the tree and the driver shared_ptr; node is
// the position in that tree (root.get() or any descendant).  Copies of the view
// share the same root, so every sub-view keeps the data alive.
struct InfoView
{
    std::shared_ptr<mcinfo> root;
    const mcinfo_node *node;
};

const mcinfo &as_group(const InfoView &self)
{
    if (!self.node->is_group())
        throw py::type_error("this Info node is a value, not a group");
    return static_cast<const mcinfo &>(*self.node);
}

} // namespace

void bind_info(py::module_ &m)
{
    py::class_<InfoView> info(m, "Info",
        "Read-only view of simulation results.  Wraps the C++ mcinfo tree.\n\n"
        "Example::\n\n"
        "    info = opentrim.Info(sim)            # sim is a Driver\n"
        "    v, dv = info[\"tally\"][\"damage_events\"][\"Vacancies\"]\n\n"
        "Tally nodes return (values, sem) numpy tuples normalized per ion.\n"
        "Note: reading results while a Mode A run is in progress may race;\n"
        "prefer calling sim.wait() first.");

    info.def(py::init([](py::object driver_obj) {
                 DriverWrapper &w = driver_obj.cast<DriverWrapper &>();
                 // mcinfo holds a shared_ptr to the driver, so the Info keeps it
                 // alive even if the Python Driver is dropped.
                 auto root = std::make_shared<mcinfo>(w.driver());
                 return InfoView{ root, root.get() };
             }),
             py::arg("driver"),
             "Build an Info view over a Driver.");

    info.def("keys",
             [](const InfoView &self) { return as_group(self).keys(); },
             "Available keys at this level, in definition order.");

    info.def("__getitem__",
             [](const InfoView &self, const std::string &key) -> py::object {
                 const mcinfo &g = as_group(self);
                 const mcinfo_node *child = g.at(key);
                 if (!child)
                     throw py::key_error(key);
                 if (child->is_group())
                     // sub-view shares this view's root, so it stays valid.
                     return py::cast(InfoView{ self.root, child });
                 return leaf_value(static_cast<const mcinfo_data_node &>(*child), key);
             },
             py::arg("key"),
             "Return a sub-Info for a group, or the value for a data node.");

    info.def("__contains__",
             [](const InfoView &self, const std::string &key) {
                 return self.node->is_group()
                         && static_cast<const mcinfo &>(*self.node).contains(key);
             });

    info.def("__len__",
             [](const InfoView &self) {
                 return self.node->is_group()
                         ? static_cast<const mcinfo &>(*self.node).size()
                         : (std::size_t)0;
             });

    info.def("__iter__",
             [](py::object self) { return py::iter(self.attr("keys")()); });

    info.def_property_readonly(
            "type", [](const InfoView &self) { return std::string(type_name(*self.node)); },
            "Node type: group, string, json, real64, real32, uint64, or tally_score.");

    info.def_property_readonly(
            "description", [](const InfoView &self) { return self.node->description(); },
            "Human-readable description of this node.");

    info.def_property_readonly(
            "path", [](const InfoView &self) { return self.node->path(); },
            "Path of this node in the tree, e.g. \"/tally/totals\".");

    info.def_static("info_spec", []() { return mcinfo::info_spec(); },
                    "Return the JSON spec of the full info tree structure.");

    // structural tree only: reads node metadata, never calls a data getter, so
    // it has no side effects and is safe to print even while a run is active.
    info.def("__repr__", [](const InfoView &self) { return render_tree(*self.node); });
    info.def("__str__", [](const InfoView &self) { return render_tree(*self.node); });

    // Jupyter rich display.  the protocol method is _repr_html_ (single
    // underscores); same side-effect-free structural walk as __repr__.
    info.def("_repr_html_", [](const InfoView &self) {
        std::string out = "<div style=\"font-family:monospace\"><b>Info</b> "
                          "<span style=\"color:#888\">[";
        out += type_name(*self.node);
        out += "]</span>";
        if (self.node->is_group())
            html_tree(static_cast<const mcinfo &>(*self.node), out);
        out += "</div>";
        return out;
    });
}
