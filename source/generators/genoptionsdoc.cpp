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

std::ostream &operator<<(std::ostream &os, mcconfig::option_type_t type)
{
    switch (type) {
    case mcconfig::tStruct:
        os << "Option group";
        break;
    case mcconfig::tArray:
        os << "Array of same type options";
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

    case mcconfig::tArray:
        if (!is_root) {
            os << "<tr><td>" << "Description ";
            os << "<td>" << j["label"].template get<std::string>() << endl;
        }

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
