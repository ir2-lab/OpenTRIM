#include "json_defs_p.h"
#include "mcdriver.h"
#include "mcinfo.h"

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

        const ojson &j = json_options_spec();

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

        printH5table(os);

        os << endl << endl;

        cout << " done." << endl;
    }

    return 0;
}

#define TAB_WIDTH 2

std::string htmlEscape(const std::string &s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
        case '&':
            out += "&amp;";
            break;
        case '<':
            out += "&lt;";
            break;
        case '>':
            out += "&gt;";
            break;
        case '"':
            out += "&quot;";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

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

mcconfig opt = mcconfig::config_template();

void jsonPrint(std::ostream &os, const ojson &j, int level, string path)
{
    string val;

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

    case mcconfig::tArray:
        if (level) {
            indent(os, level);
            linkCode(os, path, true);
            os << R"("\")" << j["name"].template get<string>() << R"(\"")" << ": ";
        }
        os << '[' << "<br>" << endl;
        {
            const ojson &items = j["items"];
            mcconfig::option_type_t item_type;
            items["type"].get_to(item_type);
            if (item_type == mcconfig::tStruct) {
                level++;
                indent(os, level);
                os << '{' << "<br>" << endl;

                if (!items["fields"].empty()) {
                    auto it = items["fields"].begin();
                    const ojson &obj = *it++;
                    jsonPrint(os, obj, level + 1, path + "/0");
                    for (; it != items["fields"].end(); ++it) {
                        os << ',' << "<br>" << endl;
                        const ojson &obj = *it;
                        jsonPrint(os, obj, level + 1, path + "/0");
                    }
                }
                os << "<br>" << endl;
                indent(os, level);
                os << '}';
                level--;
            }
        }
        os << "<br>" << endl;
        indent(os, level);
        os << ']';
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

void printPath(std::ostream &os, const string &path)
{
    os << path;
}

void jsonPrintDesc(std::ostream &os, const ojson &j, mcconfig::option_type_t type)
{
    os << "<tr><td>" << "Description ";
    os << "<td>" << htmlEscape(j["toolTip"].template get<std::string>()) << endl;
    if (type == mcconfig::tEnum) {
        vector<string> values, labels, desc;
        j["values"].get_to(values);
        j["valueLabels"].get_to(labels);
        j["valueDescriptions"].get_to(desc);
        os << "<h4>Options</h4><ul>";
        for (int i = 0; i < int(values.size()); ++i) {
            os << "<li>";
            os << "<strong>" << htmlEscape(values[i]) << "</strong>";
            if (labels[i] != values[i]) {
                os << " [" << htmlEscape(labels[i]) << "]";
            }
            os << " - " << htmlEscape(desc[i]);
            os << "</li>";
        }
        os << "</ul>";
    }
    if (j.contains("whatsThis")) {
        vector<string> s;
        if (j["whatsThis"].is_array())

            j["whatsThis"].get_to(s);
        else {
            string s1;
            j["whatsThis"].get_to(s1);
            if (!s1.empty())
                s.push_back(s1);
        }
        if (!s.empty()) {
            os << "<h4>Notes</h4><ul>";
            for (auto it = s.cbegin(); it != s.cend(); it++)
                os << "<li>" << htmlEscape(*it) << "</li>" << endl;
            os << "</ul>";
        }
    }
}

void jsonPrintTable(std::ostream &os, const ojson &j, string path)
{
    bool is_root = path.empty();
    string name, val;

    mcconfig::option_type_t type = j["type"].template get<mcconfig::option_type_t>();

    if (!is_root) {
        name = j["name"].template get<std::string>();
        path += name;
        os << "<tr><th colspan=\"2\">";
        linkCode(os, path, false);
        printPath(os, path); // os << path << endl;
        os << "<tr><td>" << "Label ";
        os << "<td>" << htmlEscape(j["label"].template get<std::string>()) << endl;
        os << "<tr><td>" << "Type ";
        os << "<td>" << toString(type) << endl;
        opt.get(path, val);
        path += '/';
    } else
        path += '/';

    switch (type) {

    case mcconfig::tStruct:
        if (!is_root)
            jsonPrintDesc(os, j, type);

        for (auto it = j["fields"].begin(); it != j["fields"].end(); ++it) {
            const ojson &obj = *it;
            jsonPrintTable(os, obj, path);
        }

        return;

    case mcconfig::tArray:
        if (!is_root)
            jsonPrintDesc(os, j, type);

        {
            const ojson &items = j["items"];
            mcconfig::option_type_t item_type;
            items["type"].get_to(item_type);
            if (item_type == mcconfig::tStruct) {
                for (auto it = items["fields"].begin(); it != items["fields"].end(); ++it) {
                    const ojson &obj = *it;
                    jsonPrintTable(os, obj, path + "0/");
                }
            }
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

        os << "<tr><td>Size<td>";
        {
            int sz = j["size"].template get<int>();
            if (sz)
                os << sz << endl;
            else
                os << "Variable" << endl;
        }
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

    jsonPrintDesc(os, j, type);
}

void h5Print(std::ostream &os, const ojson &specs, int level = 0)
{
    assert(specs.find("id") != specs.end());
    assert(specs.find("type") != specs.end());
    assert(specs.find("description") != specs.end());

    std::string id, type, desc;
    specs["id"].get_to(id);
    specs["type"].get_to(type);
    specs["description"].get_to(desc);

    if (type == "Group") {

        os << "<tr><td>";
        if (level) {
            indent(os, level);
            os << id << "/" << endl;
        } else
            os << "/" << endl;
        os << "<td>Group<td><td>" << desc << endl;

        assert(specs.find("objects") != specs.end());
        assert(specs["objects"].is_array());
        const ojson &objects = specs["objects"];
        for (auto i = objects.begin(); i != objects.end(); ++i)
            h5Print(os, *i, level + 1);

    } else { // Dataset

        assert(specs.find("datatype") != specs.end());
        assert(specs.find("size") != specs.end());

        std::string datatype, size;
        specs["datatype"].get_to(datatype);
        specs["size"].get_to(size);

        os << "<tr><td>";
        if (level) {
            indent(os, level);
        };
        os << id << endl;
        os << "<td>" << datatype << endl;
        os << "<td>" << size << endl;
        os << "<td>" << desc << endl;
    }
}

void printH5table(std::ostream &os)
{
    os << "<table>\n"
          "<caption>OpenTRIM HDF5 output archive structure</caption>\n";

    os << "<tr><th>Path\n<th>Type\n<th>Size\n<th>Description\n";

    std::istringstream is(mcinfo::info_spec());
    ojson info_specs = ojson::parse(is, nullptr, true, true);

    h5Print(os, info_specs);

    os << "</table>\n";
}
