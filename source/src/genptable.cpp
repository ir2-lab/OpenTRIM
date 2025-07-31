#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <limits>

using namespace std;

void parse_csv_line(const std::string s, vector<string> &tokens)
{
    bool in_string_token{ false };
    string token;
    for (int i = 0; i < s.size(); ++i) {
        if (s[i] == ',' && !in_string_token) {
            tokens.push_back(token);
            token.clear();
        } else if (s[i] == '"') {
            in_string_token = !in_string_token;
        } else {
            token.push_back(s[i]);
        }
    }
}

struct isotope
{
    int A;
    std::string symbol;
    double mass;
    double abundance;
    double half_life;
};

struct element
{
    int Z;
    std::string symbol;
    std::string name;
    double mass;
    double density;
    string phase;
    std::vector<isotope> isotopes;
};

vector<element> elements_;

map<string, int> symbol2z_;

int loadIsotopeTable(const char *fname);
int loadPTableCSV(const char *fname);
void printCpp(const char *fname);

int main(int argc, char *argv[])
{

    cout << "Generating element tables...";
    loadIsotopeTable(argv[1]);
    loadPTableCSV(argv[2]);

    // create lookup table
    for (int z = 1; z < elements_.size(); ++z) {
        symbol2z_[elements_[z].symbol] = z;
    }

    printCpp(argv[3]);
    cout << "done.\n";
    return 0;
}

void print_isotope(ostream &os, const isotope &i)
{
    const char *idnt = "      ";
    os << idnt << '{' << std::endl;
    os << idnt << "  " << i.A << ',' << endl;
    os << idnt << "  " << '"' << i.symbol << '"' << ',' << endl;
    os << idnt << "  " << i.mass << ',' << endl;
    os << idnt << "  " << i.abundance << ',' << endl;
    os << idnt << "  ";
    if (isfinite(i.half_life))
        os << i.half_life;
    else
        os << "1.0/0.0";
    os << endl;
    os << idnt << '}';
}

void print_element(ostream &os, const element &e)
{
    const char *idnt = "  ";
    if (e.Z < 0) {
        os << idnt << "{}";
        return;
    }
    os << idnt << '{' << std::endl;
    os << idnt << "  " << e.Z << ',' << endl;
    os << idnt << "  " << '"' << e.symbol << '"' << ',' << endl;
    os << idnt << "  " << '"' << e.name << '"' << ',' << endl;
    os << idnt << "  " << e.mass << ',' << endl;
    os << idnt << "  " << e.density << ',' << endl;

    os << idnt << "  ";
    if (e.phase == "Gas")
        os << "phase_t::gas";
    else if (e.phase == "Liquid")
        os << "phase_t::liquid";
    else if (e.phase == "Solid")
        os << "phase_t::solid";
    else
        os << "phase_t::solid";
    os << ',' << endl;

    if (!e.isotopes.empty()) {
        os << idnt << "  " << '{' << endl;

        print_isotope(os, e.isotopes[0]);
        for (int k = 1; k < e.isotopes.size(); ++k) {
            os << ',' << endl;
            print_isotope(os, e.isotopes[k]);
        }
        os << endl;
        os << idnt << "  " << '}' << endl;
    } else
        os << idnt << "  "
           << "{}" << endl;

    os << idnt << '}';
}

void printCpp(const char *fname)
{
    ofstream cpp(fname);

    cpp << setprecision(numeric_limits<double>::digits10);

    cpp << "/* generated code - do not change */\n\n";
    cpp << "#include \"periodic_table.h\"\n\n";
    cpp << "const std::vector<periodic_table::element> periodic_table::elements_\n";
    cpp << "{\n";
    print_element(cpp, elements_[0]);
    for (int i = 1; i < elements_.size(); ++i) {
        cpp << ",\n";
        print_element(cpp, elements_[i]);
    }
    cpp << "\n};\n\n";

    cpp << "const std::map<std::string, int> periodic_table::symbol2z_\n";
    cpp << "{\n";
    for (auto i = symbol2z_.begin(); i != symbol2z_.end(); ++i) {
        if (i != symbol2z_.begin())
            cpp << ",\n";
        cpp << "  " << '{' << '"' << i->first << '"' << ", " << i->second << '}';
    }
    cpp << "\n};\n\n";
}

/* Columns in isotope-data/isotopes_data.csv
name	0
symbol	1
isot_symbol	2
atomic_number	3
mass_number	4
abundance	5
mass [u]	6
mass_uncertainty [u]	7
spin	8
parity	9
is_radioactive	10
half_life [s]	11
half_life_uncertainty [s]	12
gfactor	13
gfactor_uncertainty	14
quadrupole_moment [b]	15
quadrupole_moment_uncertainty [b]	16
*/
int loadIsotopeTable(const char *fname)
{
    std::ifstream f(fname);
    std::string s;

    std::getline(f, s); // skip headers
    int k = 1;
    while (!f.eof()) {
        std::getline(f, s);
        k++;
        if (s.empty())
            break;
        std::vector<std::string> tokens;
        parse_csv_line(s, tokens);

        int Z = std::stoi(tokens[3]);
        while (elements_.size() <= Z) {
            elements_.push_back(element({ -1 }));
        }

        element &E = elements_[Z];
        E.name = tokens[0];
        E.symbol = tokens[1];
        E.Z = Z;
        isotope i;
        i.A = stoi(tokens[4]);
        i.symbol = tokens[2];

        try {
            i.mass = stod(tokens[7]);
            i.abundance = stod(tokens[5]) / 100.;
            if (tokens[11] == "true" && !tokens[12].empty())
                i.half_life = stod(tokens[12]);
        } catch (const std::exception &e) {
            cerr << "Isotope table, line " << k << endl;
            cerr << "Z = " << E.Z << endl;
            cerr << "A = " << i.A << endl;
            cerr << "mass = " << i.mass << endl;
            cerr << "abundance = " << i.abundance << endl;
            cerr << "half_life = " << i.half_life << endl;
            std::cerr << e.what() << '\n';
            throw(e);
        }

        E.isotopes.push_back(i);
    }

    return elements_.size();
}

/* Column data in PeriodicTableJSON.csv
    name	0
    appearance	1
    atomic_mass	2
    boil	3
    category	4
    density	5
    discovered_by	6
    melt	7
    molar_heat	8
    named_by	9
    number	10
    period	11
    group	12
    phase	13
    source	14
    bohr_model_image	15
    bohr_model_3d	16
    spectral_img	17
    summary	18
    symbol	19
    xpos	20
    ypos	21
    wxpos	22
    wypos	23
    shells	24
    electron_configuration	25
    electron_configuration_semantic	26
    electron_affinity	27
    electronegativity_pauling	28
    ionization_energies	29
    cpk-hex	30
      block	31
      image.title	32
      image.url	33
      image.attribution	34
*/

int loadPTableCSV(const char *fname)
{
    std::ifstream f(fname);
    std::string s;

    std::getline(f, s); // skip headers
    int k = 1;
    while (!f.eof()) {
        std::getline(f, s);
        k++;
        if (s.empty())
            break;
        std::vector<std::string> tokens;
        parse_csv_line(s, tokens);

        int Z = std::stoi(tokens[10]);
        while (elements_.size() <= Z) {
            elements_.push_back(element({ -1 }));
        }

        element &E = elements_[Z];
        if (E.Z != Z) {
            E.Z = Z;
            E.name = tokens[0];
            if (!tokens[2].empty())
                E.mass = stod(tokens[2]);
            E.symbol = tokens[19];
        }
        if (!tokens[2].empty() && E.mass == 0)
            E.mass = stod(tokens[2]);
        if (!tokens[5].empty())
            E.density = stod(tokens[5]);
        E.phase = tokens[13];
    }

    return elements_.size();
}
