/*
 * Adapted from Corteo by F. Schiettekatte
 * http://www.lps.umontreal.ca/~schiette/index.php?n=Recherche.Corteo
 *
 * gendedx
 *
 * Utility to generate dE/dx files for all elements in all elements for Z=1(H) .. 92(U)
 * using the SRIM utility SRModule
 *
 * Usage:
 *   gendedx srmodule_folder srmodule_cmd output_folder
 *
 * where
 *   srmodule_folder : the path to the folder of SRModule.exe
 *   srmodule_cmd : the command used to run SRModule. In Linux this could be "wine SRModule.exe"
 *   output_folder : a folder to store the output files
 *
 * Example:
 *   ./gendedx "/home/george/.local/SRIM-2013/srmodule" \
 *             'WINEPREFIX=/home/george/.local/share/wineprefixes/Wine32 WINEARCH="win32" wine
 * SRModule.exe' \ dedx
 *
 * The contents of the output folder should be copied to opentrim/dedx/ in order to build the
 * iondedx library
 *
 *
 */

#include "dedx.h"
#include "periodic_table.h"

#include <cmath>
#include <cfloat>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <charconv>

#ifdef WIN32
const char endline[] = "\n";
#else
// in Linux or mac, where SRModule is called using Wine, we must feed an input file containing
// return+linefeed
const char endline[] = "\r\n";
#endif

namespace fs = std::filesystem;
using namespace std;

double mostAbundantIsotopeMass(int Z)
{
    auto &e = periodic_table::at(Z);

    if (e.isotopes.empty())
        return 0.0;

    double max_abnd = 0;
    const periodic_table::isotope *i_max_abnd = nullptr;
    double max_hl = 0;
    const periodic_table::isotope *i_max_hl = nullptr;
    for (const periodic_table::isotope &i : e.isotopes) {
        if (i.abundance > max_abnd) {
            max_abnd = i.abundance;
            i_max_abnd = &i;
        }
        if (i.half_life > max_hl) {
            max_hl = i.half_life;
            i_max_hl = &i;
        }
    }
    if (i_max_abnd != nullptr)
        return i_max_abnd->mass;
    else if (i_max_hl != nullptr)
        return i_max_hl->mass;
    else
        return 0.0;
}

string srmodule_folder;
string srmodule_command;
string out_folder;

int parse(int argc, char *argv[]);
int doit();

int main(int argc, char *argv[])
{
    if (parse(argc, argv) < 0)
        return -1;
    return doit();
}

int parse(int argc, char *argv[])
{
    const char *usage = "Usage: gendedx srmodule_folder srmodule_cmd out_folder";
    if (argc != 4) {
        cout << usage << endl;
        return -1;
    }
    srmodule_folder = string(argv[1]);
    srmodule_command = string(argv[2]);
    out_folder = string(argv[3]);
    return 0;
}

int run_srim(int Z1, int Z2, vector<float> &data)
{
    // store current folder and change to srim srmodule folder
    fs::path work_dir(fs::current_path());
    fs::current_path(srmodule_folder);

    // create SR.IN file
    {
        // write SRModule input file
        ofstream os("SR.IN");
        os << endline << endline << "SR_OUTPUT.txt" << endline << endline << Z1 << ' '
           << mostAbundantIsotopeMass(Z1) << endline << endline << " 0 1 0" << endline << endline
           << "1" << endline << endline << Z2 << " \"X\" 1 1" << endline << endline << " 7"
           << endline << endline << " 0 0 " << endline;

        for (const float erg : dedx_index())
            os << erg / 1000 << endline;

        os << " 0 ";
    }

    // call srmodule
    int ret = system(srmodule_command.c_str());

    // if ok read output
    if (ret == 0) {
        ifstream is("SR_OUTPUT.txt");
        char buff[4096];
        is.getline(buff, 4096);
        is.getline(buff, 4096);
        is.getline(buff, 4096);
        is.getline(buff, 4096);

        int i = 0;
        while (is.good() && i < data.size()) {
            double d, dedxval;
            is >> d >> data[i];
            is.getline(buff, 4096);
            i++;
        }

        if (!is.good()) {
            cerr << "Failed to read SR_OUTPUT.txt" << endl;
            ret = -1;
        }
    }

    // return to workdir
    fs::current_path(work_dir);
    return ret;
}

// print out a 32bit float so that it is read back the same
// employ C++17 std::to_chars to be system locale independent
std::ostream &printfloat(std::ostream &os, float x)
{
    // print buffer
    static constexpr int buff_len = 32;
    char buff[buff_len]{};
    // get number of digits for a float -> text -> float round-trip
    static constexpr auto d = std::numeric_limits<float>::max_digits10;
    // print the number
    std::to_chars_result ret =
            std::to_chars(buff, buff + buff_len, x, std::chars_format::general, d);
    *ret.ptr = 0;
    os << buff;
    return os;
}s;
}

int print_cpp_matrix(ofstream &Z1cpp, int Z1, int Z2, const vector<float> &data)
{
    Z1cpp << "static const float " << periodic_table::at(Z1).symbol << "_on_"
          << periodic_table::at(Z2).symbol << "[] = {";
    const int val_per_line = 10;
    for (int i = 0; i < data.size() - 1; ++i) {
        if (i % val_per_line == 0)
            Z1cpp << endl << "    ";
        printfloat(Z1cpp, data[i]);
        Z1cpp << ", ";
    }
    Z1cpp << data.back() << "};" << endl << endl;
    return 0;
}

int print_cpp_element_matrix(ofstream &Z1cpp, int Z1, int Z2max)
{
    Z1cpp << "const float* dedx" << periodic_table::at(Z1).symbol << "[] = { nullptr,";
    const int val_per_line = 5;
    for (int Z2 = 1; Z2 <= Z2max; Z2++) {
        if (((Z2 - 1) % val_per_line) == 0)
            Z1cpp << endl << "    ";
        Z1cpp << periodic_table::at(Z1).symbol << "_on_" << periodic_table::at(Z2).symbol;
        if (Z2 < Z2max)
            Z1cpp << ", ";
        else
            Z1cpp << "};" << endl << endl;
    }
    return 0;
}

int doit()
{
    const char *preample = "// file generated by gendedx - do not edit\n\n";

    vector<float> data(dedx_index::size);

    int Zmax = dedx_max_Z;
    cout << "Generating dedx data:" << endl;
    for (int Z1 = 1; Z1 <= Zmax; Z1++) {
        cout << periodic_table::at(Z1).symbol << ' ';
        cout.flush();
        string fname(out_folder);
        fname += "/";
        fname += periodic_table::at(Z1).symbol;
        fname += ".cpp";
        ofstream Z1cpp(fname);
        // Z1cpp << fixed << setprecision(4);
        Z1cpp << preample;
        for (int Z2 = 1; Z2 <= Zmax; Z2++) {
            if (Z2 % 10 == 0) {
                cout << '.';
                cout.flush();
            }
            if (run_srim(Z1, Z2, data) < 0)
                return -1;
            print_cpp_matrix(Z1cpp, Z1, Z2, data);
        }
        print_cpp_element_matrix(Z1cpp, Z1, Zmax);
        cout << endl;
    }

    { // generate master file dedx_data.cpp
        string fname(out_folder);
        fname += "/dedx_data.cpp";
        ofstream cpp(fname);
        cpp << preample;

        for (int Z1 = 1; Z1 <= Zmax; Z1++) {
            cpp << "extern const float* dedx" << periodic_table::at(Z1).symbol << "[];" << endl;
        }
        cpp << endl;

        cpp << "const float** dedx_data[] = { nullptr, ";
        const int val_per_line = 5;
        for (int Z1 = 1; Z1 <= Zmax; Z1++) {
            if (((Z1 - 1) % val_per_line) == 0)
                cpp << endl << "    ";
            cpp << "dedx" << periodic_table::at(Z1).symbol;
            if (Z1 < Zmax)
                cpp << ", ";
            else
                cpp << "};" << endl << endl;
        }
    }

    { // generate most_abundant_isotope_mass.cpp
        ofstream cpp("most_abundant_isotope_mass.cpp");
        cpp << setprecision(15);
        cpp << preample;

        cpp << "static const double most_abundant_isotope_mass_[] = {\n    0";
        for (int Z = 1; Z < periodic_table::size(); Z++) {
            cpp << ',' << endl << "    " << mostAbundantIsotopeMass(Z);
        }
        cpp << "\n};\n";
        cpp << "const double* most_abundant_isotope_mass = most_abundant_isotope_mass_;\n\n";
    }

    return 0;
}
