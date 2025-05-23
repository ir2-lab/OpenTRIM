#include "xs.h"
#include "xs_corteo.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <charconv>

#include <cxxopts.hpp>

using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;

template <Screening ScreeningType>
int gencorteo4bit(const std::string &short_screening_name);

int main(int argc, char *argv[])
{
    cxxopts::Options cli_options("gencorteo", "Generate corteo-type screened Coulomb XS table");

    cli_options.add_options()("s,screening", "Screening type: one of ZBL, LJ, KrC, Moliere",
                              cxxopts::value<std::string>())("h,help",
                                                             "Display short help message");

    Screening s(Screening::ZBL);
    std::string short_screening_name("zbl");

    try {
        auto result = cli_options.parse(argc, argv);

        if (result.count("help")) {
            cout << cli_options.help() << endl;
            return 0;
        }

        if (result.count("s")) {
            short_screening_name = result["s"].as<std::string>();
            std::transform(short_screening_name.begin(), short_screening_name.end(),
                           short_screening_name.begin(),
                           [](unsigned char c) { return std::tolower(c); });
        }
    } catch (const cxxopts::exceptions::exception &e) {
        cerr << "error parsing options: " << e.what() << endl;
        return -1;
    }

    if (short_screening_name == "zbl")
        s = Screening::ZBL;
    else if (short_screening_name == "krc")
        s = Screening::KrC;
    else if (short_screening_name == "lj")
        s = Screening::LenzJensen;
    else if (short_screening_name == "moliere")
        s = Screening::Moliere;
    else {
        cerr << "Unknown screening type option: " << short_screening_name << endl;
        return -1;
    }

    switch (s) {
    case Screening::ZBL:
        return gencorteo4bit<Screening::ZBL>(short_screening_name);
        break;
    case Screening::KrC:
        return gencorteo4bit<Screening::KrC>(short_screening_name);
        break;
    case Screening::LenzJensen:
        return gencorteo4bit<Screening::LenzJensen>(short_screening_name);
        break;
    case Screening::Moliere:
        return gencorteo4bit<Screening::Moliere>(short_screening_name);
        break;
    default:
        return -1;
        break;
    }

    return 0;
}

const char *preample = "/*\n"
                       " * file generated by gencorteo utility of " PROJECT_NAME "\n"
                       " * \n"
                       " * do not edit \n"
                       " */\n";

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
}

template <Screening ScreeningType>
int gencorteo4bit(const std::string &short_screening_name)
{
    typedef xs_corteo_index corteo4bit; // 4bit corteo indexing

    xs_cms<ScreeningType> xs;

    cout << "Computing " << xs.screeningName() << " scattering table";

    // create func name
    std::string func("xs_");
    func += short_screening_name;
    func += "_data";

    ofstream ofs(func + ".cpp");
    ofs << preample << endl;
    ofs << "static const float " << func << "_[] = {\n";

    // compute matrix for each reduced energy, reduced impact parameter pair
    int k = 0;
    int klast = corteo4bit::rows * corteo4bit::cols - 1;
    for (corteo4bit::e_index ie; ie < ie.end(); ie++) {

        if (ie % (corteo4bit::rows / 10) == 0) {
            cout << ".";
            cout.flush();
        }

        for (corteo4bit::s_index is; is < is.end(); is++) {
            printfloat(ofs, xs.sin2Thetaby2(*ie, *is));
            if (k != klast)
                ofs << ',';
            k++;
            if (k % 16 == 0)
                ofs << endl;
        }
    }

    ofs << "}; \n\n";

    ofs << "const float* " << func << "() { return " << func << "_; } \n";

    cout << " done.\n";
    cout.flush();

    return 0;
}
