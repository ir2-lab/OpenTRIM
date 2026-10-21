#include "dedx.h"
#include "periodic_table.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

/*
 * Generate the data to reproduce fig. 3
 * from Q.Yang, D.J. O'Connor, Z. Wang, Nucl. Instr. Meth. B61 (1991) 149
 */

#define E2 1.43996445 /* e^2 / 4 pi eps0 = e^2 c^2 in [eV][nm] */
#define EMAX double(1 << 30)

// calc Yang straggling of ion in Al target
// save to file, e.g. He_Al_Yang.dat
void calc_Al(double *x, int Nx, const char *ion)
{
    int Z1 = periodic_table::at(ion).Z;
    double M1 = periodic_table::at(ion).mass;
    int Z2 = periodic_table::at("Al").Z;

    double OmegaBohr = sqrt(4 * M_PI * Z1 * Z1 * Z2 * E2 * E2); /* eV */

    straggling_interp S(StoppingModel::SRIM13, StragglingModel::Yang, Z1, M1, Z2);

    string fname(ion);
    fname += "_Al_Yang.dat";
    ofstream os(fname);
    for (int i = 0; i < Nx; ++i) {
        double E = x[i] * pow(1.0 * Z1, 3.0 / 2) * M1 * 1.e6; // eV
        double s = E < EMAX ? S(E) / OmegaBohr : 1.0 / 0.0;
        os << s << endl;
    }
}

int main()
{
    int Z1, Z2;
    double M1, M2;

    // create x values
    const int Nx = 41;
    double x[Nx];
    {
        ofstream os("x_Yang.dat");
        for (int i = 0; i < Nx; ++i) {
            double y = -3.0 + 1.0 * i / (Nx - 1) * 4;
            x[i] = std::pow(10., y);
            os << x[i] << endl;
        }
    }

    // Target Al
    Z2 = periodic_table::at("Al").Z;

    // H ion
    Z1 = periodic_table::at("H").Z;
    M1 = periodic_table::at("H").mass;

    double OmegaBohr = sqrt(4 * M_PI * Z1 * Z1 * Z2 * E2 * E2); /* eV */

    {
        straggling_interp S(StoppingModel::SRIM13, StragglingModel::Chu, Z1, M1, Z2);

        ofstream os("H_Al_Chu.dat");
        for (int i = 0; i < Nx; ++i) {
            double E = x[i] * pow(1.0 * Z1, 3.0 / 2) * M1 * 1.e6; // eV
            os << S(E) / OmegaBohr << endl;
        }
    }

    calc_Al(x, Nx, "H");
    calc_Al(x, Nx, "He");
    calc_Al(x, Nx, "C");
    calc_Al(x, Nx, "O");
    calc_Al(x, Nx, "Cl");
    calc_Al(x, Nx, "Ar");

    return 0;
}
