#include "json_defs_p.h"
#include "mcdriver.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::vector;

enum type_t { tEnum, tFloat, tInt, tBool, tString, tVector, tIntVector, tStruct, tInvalid };

type_t toType(const string &typeName);
void jsonPrint(std::ostream &os, const ojson &j, type_t type = tStruct, int level = 0,
               string path = "");
void jsonPrintTable(std::ostream &os, const ojson &j, type_t type = tStruct, string path = "");

int linkID;

const char *preampleJSON = R"(
## The JSON configuration file {#json_config}

All configuration parameters for a simulation are coded in a [JSON](https://www.json.org/json-el.html) formatted string. 

This can be loaded directly from a file to the \ref cliapp "opentrim cli program" with the following command:

    opentrim -f config.json

where `config.json` is a file containing the JSON configuration.

In a C++ program one can use the \ref mcdriver::options class which offers
a parseJSON() function to parse and validate the options.
                        
The easiest way to get started is to generate a template with all default options by running
                                        
    opentrim -t > template.json
                        
and make changes to the new template file.

@note `opentrim` accepts comments in JSON 

The JSON config string has the following self-explanatory structure shown bellow.
Click on any of the options to see more information.

)";

const char *postJSON = R"(

)";

const char *preampleH5 = R"(
## The HDF5 output archive {#out_file}

The simulation produces a single HDF5 output file which contains all
results from tallies and events along with the input conditions, run
timing and other information. 
 
A brief description of the file contents is given here. More detailed
information can be found within the file itself. 

Dimensions of tables depend on:
- \f$N_{at}\f$ : # of atoms, i.e., all atoms in the target plus the
projectile. Note that the same atom species belonging to different
materials is counted as different atom.
- \f$N_{mat}\f$ : # of target materials
- \f$N_x, N_y, N_z\f$ : # of grid points along the 3 axes
- \f$N_c=(N_x-1)(N_y-1)(N_z-1)\f$ : # of target cells
- \f$N_e\f$ : # of energy points for energy loss tables
- \f$N_{ev}\f$ : # of events

)";

const char *postH5 = R"(
To reach a variable in the archive use the complete path, e.g. `/tally/damage/Tdam`.

Each number in the tally tables is a mean value over all histories.
For each tally table the standard error of the mean (SEM) is also included. This is a separate table of equal dimension and with the same name plus the ending `_sem`. E.g., for the table `/tally/damage/Tdam` there is also `/tally/damage/Tdam_sem`.

The definition of SEM employed is as follows: if \f$x_i\f$ is the contribution to the table entry \f$x\f$ from the \f$i\f$-th ion history, then the mean, \f$\bar{x}\f$, and the SEM, \f$\f$\sigma_{\bar{x}}\f$\f$, listed in the output file are calculated as:
$$
\bar{x} = \frac{1}{N_h} \sum_i {x_i}
$$
$$
\sigma_{\bar{x}}^2 = \frac{1}{N_h(N_h-1)} \sum_i { (x_i - \bar{x})^2 }=
\frac{\bar{{x^2}} - \bar{x}^2}{N_h-1}
$$

\see tally

)";

const char *tableStart = "<table>\n"
                         "<caption>Detailed Description</caption>\n";

const char *tableEnd = "</table>\n";

void printH5table(std::ostream &os);

int main(int argc, char *argv[])
{
    {
        cout << "[genoptionsdoc] input file: " << argv[1] << endl;
        std::ifstream is(argv[1]);
        ojson j = ojson::parse(is, nullptr, true, true);

        std::ofstream os("options.dox.md");

        os << preampleJSON;
        os << "> <br>" << endl;
        linkID = 0;
        jsonPrint(os, j);
        os << "<br>" << endl << endl;

        os << postJSON;

        os << endl << endl;
        os << tableStart;
        linkID = 0;
        jsonPrintTable(os, j);
        os << tableEnd;
    }

    {
        std::ofstream os("h5file.dox.md");

        os << preampleH5;

        printH5table(os);

        os << postH5;

        os << endl << endl;
    }

    return 0;
}

type_t toType(const string &typeName)
{
    if (typeName == "enum")
        return tEnum;
    else if (typeName == "float")
        return tFloat;
    else if (typeName == "int")
        return tInt;
    else if (typeName == "bool")
        return tBool;
    else if (typeName == "string")
        return tString;
    else if (typeName == "vector")
        return tVector;
    else if (typeName == "ivector")
        return tIntVector;
    else if (typeName == "struct")
        return tStruct;
    else
        return tInvalid;
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

void jsonPrint(std::ostream &os, const ojson &j, type_t type, int level, string path)
{
    string val;
    mcdriver::options opt;

    if (level) {
        path += '/';
        path += j["name"].template get<std::string>();
        opt.get(path, val);
    };

    switch (type) {
    case tStruct:
        if (level) {
            indent(os, level);
            linkCode(os, path, true);
            os << R"("\")" << j["name"].template get<string>() << R"(\"")" << ": ";
        }
        os << '{' << "<br>" << endl;
        if (!j["fields"].empty()) {
            auto it = j["fields"].begin();
            const ojson &obj = *it++;
            jsonPrint(os, obj, toType(obj["type"]), level + 1, path);
            for (; it != j["fields"].end(); ++it) {
                os << ',' << "<br>" << endl;
                const ojson &obj = *it;
                jsonPrint(os, obj, toType(obj["type"]), level + 1, path);
            }
        }
        os << "<br>" << endl;
        indent(os, level);
        os << '}';
        if (!level)
            os << "<br>" << endl;
        break;
    case tEnum:
    case tFloat:
    case tVector:
    case tIntVector:
    case tInt:
    case tBool:
    case tString:
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

std::ostream &operator<<(std::ostream &os, type_t type)
{
    switch (type) {
    case tStruct:
        os << "Option group";
        break;
    case tEnum:
        os << "Enumerator";
        break;
    case tFloat:
        os << "Floating point number";
        break;
    case tVector:
        os << "Vector of floating point values";
        break;
    case tIntVector:
        os << "Vector of integer values";
        break;
    case tInt:
        os << "Integer";
        break;
    case tBool:
        os << "Boolean";
        break;
    case tString:
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
    for (int i = 1; i < path.size(); i++) {
        char c = path[i];
        if (c == '/')
            c = '.';
        os << c;
    }
}

void jsonPrintTable(std::ostream &os, const ojson &j, type_t type, string path)
{
    bool is_root = path.empty();
    string name, val;
    mcdriver::options opt;

    if (!is_root) {
        name = j["name"].template get<std::string>();
        path += name;
        os << "<tr><th colspan=\"2\">";
        linkCode(os, path, false);
        printPath(os, path); // os << path << endl;
        os << "<tr><td>" << "Type ";
        os << "<td>" << toType(j["type"]) << endl;        
        opt.get(path, val);
        path += '/';
    } else
        path += '/';

    switch (type) {

    case tStruct:
        if (!is_root) {
            os << "<tr><td>" << "Description ";
            os << "<td>" << j["label"].template get<std::string>() << endl;
        }

        for (auto it = j["fields"].begin(); it != j["fields"].end(); ++it) {
            const ojson &obj = *it;
            jsonPrintTable(os, obj, toType(obj["type"]), path);
        }

        return;

    case tEnum:

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

    case tVector:
    case tIntVector:

        os << "<tr><td>Size<td>" << j["size"].template get<int>() << endl;
        os << "<tr><td>Element range<td>";
        os << j["min"].template get<float>() << "..." << j["max"].template get<float>() << endl;
        os << "<tr><td>Default Value<td>" << val;
        break;

    case tInt:
    case tFloat:

        os << "<tr><td>Range<td>";
        os << j["min"].template get<float>() << "..." << j["max"].template get<float>() << endl;
        os << "<tr><td>Default Value<td>" << val;
        break;

    case tBool:
    case tString:

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
                { "config_json", ds_string, { "Scalar" }, "JSON formatted simulation options" });
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
          "<caption>HDF5 output archive structure</caption>\n";

    os << "<tr><th>Path\n<th>Type\n<th>Size\n<th>Description\n";
    h5Print(os, root);
    os << "</table>\n";
}
