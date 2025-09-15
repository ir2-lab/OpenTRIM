#include "json_defs_p.h"
#include "mcdriver.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

void jsonPrint(std::ostream &os, const ojson &j, int level = 0, string path = "");
void jsonPrintTable(std::ostream &os, const ojson &j, string path = "");

int linkID;

const char *tableStart = "<table>\n"
                         "<caption>OpenTRIM JSON config - Detailed Description</caption>\n";

const char *tableEnd = "</table>\n";

void printH5table(std::ostream &os);

int main()
{
    {
        cout << "[genoptionsdoc] Generating \"options.dox.md\" ... ";

        std::istringstream is(mcconfig::options_spec());
        ojson j = ojson::parse(is, nullptr, true, true);

        std::ofstream os("options.dox.md");

        os << "## JSON config string" << endl << endl;

        os << "> <br>" << endl;
        linkID = 0;
        jsonPrint(os, j);
        os << "<br>" << endl << endl;

        os << "## Detailed description" << endl << endl;

        os << endl << endl;
        os << tableStart;
        linkID = 0;
        jsonPrintTable(os, j);
        os << tableEnd;

        cout << " done." << endl;
    }

    {
        cout << "[genoptionsdoc] Generating \"h5file.dox.md\" ... ";

        std::ofstream os("h5file.dox.md");

        os << "## Archive structure" << endl << endl;

        printH5table(os);

        os << endl << endl;

        cout << " done." << endl;
    }

    return 0;
}

#define TAB_WIDTH 2

void indent(std::ostream &os, int n)
{
    for (int i = 0; i < n * TAB_WIDTH; i++)
        os << "&emsp;";
}

void linkCode(std::ostream &os, const string &path, bool ref)
{
    os << (ref ? "\\ref " : "\\anchor ");
    for (int i = 0; i < path.size(); i++) {
        char c = path[i];
        if (c == '/')
            c = '_';
        os << c;
    }
    os << " ";
}

void jsonPrint(std::ostream &os, const ojson &j, int level, string path)
{
    string val;
    mcconfig opt;

    mcconfig::option_type_t type = j["type"].template get<mcconfig::option_type_t>();

    if (level) {
        path += '/';
        path += j["name"].template get<std::string>();
        opt.get(path, val);
    };

    switch (type) {
    case mcconfig::tStruct:
        if (level) {
            indent(os, level);
            linkCode(os, path, true);
            os << R"("\")" << j["name"].template get<string>() << R"(\"")" << ": ";
        }
        os << '{' << "<br>" << endl;
        if (!j["fields"].empty()) {
            auto it = j["fields"].begin();
            const ojson &obj = *it++;
            jsonPrint(os, obj, level + 1, path);
            for (; it != j["fields"].end(); ++it) {
                os << ',' << "<br>" << endl;
                const ojson &obj = *it;
                jsonPrint(os, obj, level + 1, path);
            }
        }
        os << "<br>" << endl;
        indent(os, level);
        os << '}';
        if (!level)
            os << "<br>" << endl;
        break;
    case mcconfig::tEnum:
    case mcconfig::tFloat:
    case mcconfig::tVector:
    case mcconfig::tIntVector:
    case mcconfig::tInt:
    case mcconfig::tBool:
    case mcconfig::tString:
        indent(os, level);
        linkCode(os, path, true);
        os << R"("\")" << j["name"].template get<string>() << R"(\"")" << ": ";
        os << val;
        break;
    default:
        assert(0);
        break;
    }
}

std::ostream &operator<<(std::ostream &os, mcconfig::option_type_t type)
{
    switch (type) {
    case mcconfig::tStruct:
        os << "Option group";
        break;
    case mcconfig::tEnum:
        os << "Enumerator";
        break;
    case mcconfig::tFloat:
        os << "Floating point number";
        break;
    case mcconfig::tVector:
        os << "Vector of floating point values";
        break;
    case mcconfig::tIntVector:
        os << "Vector of integer values";
        break;
    case mcconfig::tInt:
        os << "Integer";
        break;
    case mcconfig::tBool:
        os << "Boolean";
        break;
    case mcconfig::tString:
        os << "String";
        break;
    default:
        assert(0);
        break;
    }
    return os;
}

void printPath(std::ostream &os, const string &path)
{
    os << path;
    // for (int i = 1; i < path.size(); i++) {
    //     char c = path[i];
    //     if (c == '/')
    //         c = '.';
    //     os << c;
    // }
}

void jsonPrintTable(std::ostream &os, const ojson &j, string path)
{
    bool is_root = path.empty();
    string name, val;
    mcconfig opt;

    mcconfig::option_type_t type = j["type"].template get<mcconfig::option_type_t>();

    if (!is_root) {
        name = j["name"].template get<std::string>();
        path += name;
        os << "<tr><th colspan=\"2\">";
        linkCode(os, path, false);
        printPath(os, path); // os << path << endl;
        os << "<tr><td>" << "Type ";
        os << "<td>" << type << endl;
        opt.get(path, val);
        path += '/';
    } else
        path += '/';

    switch (type) {

    case mcconfig::tStruct:
        if (!is_root) {
            os << "<tr><td>" << "Description ";
            os << "<td>" << j["label"].template get<std::string>() << endl;
        }

        for (auto it = j["fields"].begin(); it != j["fields"].end(); ++it) {
            const ojson &obj = *it;
            jsonPrintTable(os, obj, path);
        }

        return;

    case mcconfig::tEnum:

        os << "<tr><td>Values<td> ";
        {
            vector<string> s;
            j["values"].get_to(s);
            auto it = s.cbegin();
            os << *it++;
            for (; it != s.cend(); it++)
                os << " | " << *it;
            os << endl;
        }
        os << "<tr><td>Default Value<td>" << val;
        break;

    case mcconfig::tVector:
    case mcconfig::tIntVector:

        os << "<tr><td>Size<td>" << j["size"].template get<int>() << endl;
        os << "<tr><td>Element range<td>";
        os << j["min"].template get<float>() << "..." << j["max"].template get<float>() << endl;
        os << "<tr><td>Default Value<td>" << val;
        break;

    case mcconfig::tInt:
    case mcconfig::tFloat:

        os << "<tr><td>Range<td>";
        os << j["min"].template get<float>() << "..." << j["max"].template get<float>() << endl;
        os << "<tr><td>Default Value<td>" << val;
        break;

    case mcconfig::tBool:
    case mcconfig::tString:

        os << "<tr><td>Default Value<td>" << val;
        break;

    default:
        assert(0);
        break;
    }

    os << "<tr><td>" << "Description ";
    os << "<td>" << j["toolTip"].template get<std::string>() << endl;
    if (j["whatsThis"].is_array()) {
        os << "<br>" << endl;
        vector<string> s;
        j["whatsThis"].get_to(s);
        if (!s.empty()) {
            auto it = s.cbegin();
            os << *it++ << "<br>" << endl;
            for (; it != s.cend(); it++)
                os << *it << "<br>" << endl;
        }
    }
}

enum ds_type { ds_group, ds_string, ds_numeric };

struct dsd;

typedef std::vector<dsd> ds_list_t;

struct dsd
{
    string name;
    ds_type type{ ds_group };
    std::vector<string> shape;
    string description;
    ds_list_t children{};
    void add(const dsd &d) { children.push_back(d); }
};

int create_h5_dir(dsd &root)
{
    {
        // 1. run_info
        dsd run_info{ "run_info" };
        run_info.add({ "title", ds_string, { "Scalar" }, "User supplied simulation title" });
        run_info.add(
                { "total_ion_count", ds_numeric, { "Scalar" }, "Total number of simulated ions" });
        run_info.add(
                { "json_config", ds_string, { "Scalar" }, "JSON formatted simulation options" });
        run_info.add({ "variable_list",
                       ds_string,
                       { "Scalar" },
                       "List of all data variables in the file" });
        {
            dsd version_info{ "version_info" };
            version_info.add({ "name", ds_string, { "Scalar" }, "program name" });
            version_info.add({ "version", ds_string, { "Scalar" }, "version" });
            version_info.add({ "compiler", ds_string, { "Scalar" }, "compiler" });
            version_info.add({ "compiler_version", ds_string, { "Scalar" }, "compiler version" });
            version_info.add({ "build_system", ds_string, { "Scalar" }, "build system" });
            version_info.add({ "build_time", ds_string, { "Scalar" }, "build time" });
            run_info.add(version_info);
        }
        run_info.add({ "run_history", ds_string, { "Scalar" }, "run history" });
        run_info.add({ "rng_state", ds_numeric, { "4" }, "random generator state" });
        root.add(run_info);
    }
    {
        // 2. target
        dsd g{ "target" };
        {
            dsd ch{ "grid" };
            ch.add({ "name", ds_string, { "Scalar" }, "program name" });
            ch.add({ "X", ds_numeric, { "Nx" }, "x-axis grid" });
            ch.add({ "Y", ds_numeric, { "Nx" }, "y-axis grid" });
            ch.add({ "Z", ds_numeric, { "Nx" }, "z-axis grid" });
            ch.add({ "cell_xyz", ds_numeric, { "Nc", "3" }, "Cell center coordinates" });
            g.add(ch);
        }
        {
            dsd ch{ "atoms" };
            ch.add({ "label",
                     ds_string,
                     { "Nat" },
                     "labels = [Atom (Chemical symbol)] in [Material]" });
            ch.add({ "symbol", ds_string, { "Nat" }, "Chemical symbol" });
            ch.add({ "Z", ds_numeric, { "Nat" }, "Atomic number" });
            ch.add({ "M", ds_numeric, { "Nat" }, "Atomic mass" });
            ch.add({ "Ed", ds_numeric, { "Nat" }, "Displacement energy [eV]" });
            ch.add({ "El", ds_numeric, { "Nat" }, "Lattice binding energy [eV]" });
            ch.add({ "Es", ds_numeric, { "Nat" }, "Surface binding energy [eV]" });
            ch.add({ "Er", ds_numeric, { "Nat" }, "Replacement energy [eV]" });
            g.add(ch);
        }
        {
            dsd ch{ "materials" };
            ch.add({ "name", ds_string, { "Nmat" }, "name of material" });
            ch.add({ "atomic_density", ds_numeric, { "Nmat" }, "atomic density [at/nm^3]" });
            ch.add({ "mass_density", ds_numeric, { "Nmat" }, "mass density [at/nm^3]" });
            ch.add({ "atomic_radius", ds_numeric, { "Nmat" }, "atomic radius [nm]" });
            g.add(ch);
        }
        {
            dsd ch{ "dedx" };
            ch.add({ "erg", ds_numeric, { "Ne" }, "dEdx table energy grid [eV]" });
            ch.add({ "eloss", ds_numeric, { "Nat", "Nmat", "Ne" }, "dEdx values [eV/nm]" });
            ch.add({ "strag", ds_numeric, { "Nat", "Nmat", "Ne" }, "straggling values [eV/nm]" });
            g.add(ch);
        }
        {
            dsd ch{ "flight_path" };
            ch.add({ "erg", ds_numeric, { "Ne" }, "flight path table energy grid [eV]" });
            ch.add({ "mfp", ds_numeric, { "Nat", "Nmat", "Ne" }, "mean free path [nm]" });
            ch.add({ "Tcutoff", ds_numeric, { "Nat", "Nmat", "Ne" }, "recoil energy cut0ff [eV]" });
            g.add(ch);
        }
        root.add(g);
    }
    {
        // 3. ion beam
        dsd g{ "ion_beam" };
        g.add({ "E0", ds_numeric, { "Scalar" }, "mean energy [eV]" });
        g.add({ "Z", ds_numeric, { "Scalar" }, "atomic number" });
        g.add({ "M", ds_numeric, { "Scalar" }, "mass [amu]" });
        g.add({ "dir0", ds_numeric, { "3" }, "mean direction" });
        g.add({ "x0", ds_numeric, { "3" }, "mean position [nm]" });
        root.add(g);
    }
    {
        // 4. tally
        tally t;
        dsd g{ "tally" };
        int k = 1;
        while (k < tally::std_tallies) {
            std::string name;
            name += tally::arrayGroup(k);
            name += "/";
            name += tally::arrayName(k);
            g.add({ name, ds_numeric, { "Nat", "Nc" }, tally::arrayDescription(k) });
            k++;
        }
        {
            dsd ch{ "totals" };
            ch.add({ "data",
                     ds_numeric,
                     { std::to_string(tally::std_tallies), "Nat" },
                     "tally totals per atom" });
            ch.add({ "column_names",
                     ds_string,
                     { std::to_string(tally::std_tallies) },
                     "names of totals" });
            g.add(ch);
        }
        root.add(g);
    }
    {
        // 5. events
        dsd g{ "events" };
        {
            dsd ch{ "pka" };
            ch.add({ "event_data", ds_numeric, { "Nev", "10" }, "event data" });
            ch.add({ "column_names", ds_string, { "10" }, "event data column names" });
            ch.add({ "column_descriptions",
                     ds_string,
                     { "10" },
                     "event data column descriptions" });
            g.add(ch);
        }
        {
            dsd ch{ "exit" };
            ch.add({ "event_data", ds_numeric, { "Nev", "10" }, "event data" });
            ch.add({ "column_names", ds_string, { "10" }, "event data column names" });
            ch.add({ "column_descriptions",
                     ds_string,
                     { "10" },
                     "event data column descriptions" });
            g.add(ch);
        }
        root.add(g);
    }
    return 0;
}

void h5Print(std::ostream &os, const dsd &d, int level = 0)
{
    switch (d.type) {
    case ds_group:
        os << "<tr><td>";
        if (level) {
            indent(os, level);
            os << d.name << "/" << endl;
        } else
            os << "/" << endl;
        os << "<td>Group<td><td>" << endl;
        for (const dsd &d1 : d.children)
            h5Print(os, d1, level + 1);
        break;
    default:
        os << "<tr><td>";
        if (level) {
            indent(os, level);
        };
        os << d.name << endl;
        os << "<td>" << (d.type == ds_string ? "String" : "Numeric") << endl;
        os << "<td>" << d.shape[0];
        for (int i = 1; i < d.shape.size(); i++) {
            os << " x ";
            os << d.shape[i];
        }
        os << endl;
        os << "<td>" << d.description << endl;
    }
}

void printH5table(std::ostream &os)
{
    dsd root({ "/" });
    create_h5_dir(root);

    os << "<table>\n"
          "<caption>OpenTRIM HDF5 output archive structure</caption>\n";

    os << "<tr><th>Path\n<th>Type\n<th>Size\n<th>Description\n";
    h5Print(os, root);
    os << "</table>\n";
}
