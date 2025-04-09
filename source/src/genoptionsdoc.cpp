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

enum type_t { tEnum, tFloat, tInt, tBool, tString, tVector3D, tIntVector3D, tStruct, tInvalid };

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

The JSON-formatted options string has the following self-explanatory structure shown bellow.
Click on any of the options to see more information.

)";

const char *postJSON = R"(
@note `opentrim` accepts comments in the JSON config string
                        
The easiest way to get started is to get a template with all default options by running
                                        
    opentrim -t > template.json
                        
and make changes to the new template file.
                
Most of the options shown above are default values and can be omitted.
)";

const char *tableStart = "<table>\n"
                         "<caption>Detailed Description</caption>\n";

const char *tableEnd = "</table>\n";

int main(int argc, char *argv[])
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
    else if (typeName == "vector3d")
        return tVector3D;
    else if (typeName == "ivector3d")
        return tIntVector3D;
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
    case tVector3D:
    case tIntVector3D:
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
    case tVector3D:
        os << "Floating point 3d Vector";
        break;
    case tIntVector3D:
        os << "Integer 3d Vector";
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

    case tVector3D:
    case tIntVector3D:
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
