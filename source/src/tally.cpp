#include "tally.h"
#include "target.h"

const char *tally::arrayName(int i)
{
    static const char *names[] = {
        "Totals",        "Vacancies",   "Implantations", "Replacements", "Recombinations",
        "Displacements", "Ionization",  "Lattice",       "Stored",       "Lost",
        "Pka",           "Pka_energy",  "Tdam",          "Tdam_LSS",     "Vnrt",
        "Vnrt_LSS",      "Flight_path", "Collisions",    "Lost",         "X"
    };

    return (i < std_tallies && i >= 0) ? names[i] : names[std_tallies];
}

std::vector<std::string> tally::arrayNames() const
{
    std::vector<std::string> S;
    S.push_back("Histories");
    for (int i = 1; i < std_tallies; i++)
        S.push_back(arrayName(i));
    return S;
}

const char *tally::arrayDescription(int i)
{
    static const char *desc[] = { "Totals of all quantities",
                                  "Vacancies",
                                  "Implantations & Interstitials",
                                  "Replacements",
                                  "Intra-cascade recombinations",
                                  "Displacements",
                                  "Energy deposited to ionization [eV]",
                                  "Energy deposited to the lattice as thermal energy [eV]",
                                  "Energy stored in lattice defects [eV]",
                                  "Energy lost due to ions exiting the simulation [eV]",
                                  "Primary knock-on atoms (PKAs)",
                                  "PKA recoil energy [eV]",
                                  "Damage energy [eV]",
                                  "Damage energy estimated by the LSS approximation [eV]",
                                  "Vacancies per the NRT model using Tdam",
                                  "Vacancies per the NRT model using Tdam_LSS",
                                  "Flight path [nm]",
                                  "Ion collisions",
                                  "Ions that exit the simulation volume",
                                  "X" };

    return (i < std_tallies && i >= 0) ? desc[i] : desc[std_tallies];
}

const char *tally::arrayGroup(int i)
{
    static const char *desc[] = { "totals",
                                  "damage_events",
                                  "damage_events",
                                  "damage_events",
                                  "damage_events",
                                  "damage_events",
                                  "energy_deposition",
                                  "energy_deposition",
                                  "energy_deposition",
                                  "energy_deposition",
                                  "pka_damage",
                                  "pka_damage",
                                  "pka_damage",
                                  "pka_damage",
                                  "pka_damage",
                                  "pka_damage",
                                  "ion_stat",
                                  "ion_stat",
                                  "ion_stat",
                                  "X" };

    return (i < std_tallies && i >= 0) ? desc[i] : desc[std_tallies];
}

// #pragma GCC push_options
// #pragma GCC optimize("O0")

void tally::operator()(Event ev, const ion &i, const void *pv)
{
    int iid = i.myAtom()->id(), cid = i.cellid(), pid = i.prev_cellid();
    size_t k = iid * ncells_ + cid;
    size_t kp = iid * ncells_ + pid;
    const float *p;
    const atom *a;
    size_t ka, kb;
    switch (ev) {
    case Event::BoundaryCrossing:
        A[isCollision](kp) += i.ncoll();
        A[isFlightPath](kp) += i.path();
        A[eLattice](kp) += i.phonon();
        A[eIoniz](kp) += i.ioniz();
        break;
    case Event::Replacement:
        // add a replacement
        A[cR](k)++; // this atom, current cell
        // remove a vac for the replaced atom (id passed in pointer pv), in current cell
        // and also half FP stored energy
        a = reinterpret_cast<const atom *>(pv);
        assert(a->id());
        assert(cid >= 0);
        ka = a->id() * ncells_ + cid;
        A[cV](ka)--;
        A[eStored](ka) -= i.myAtom()->El() / 2;
        A[eLattice](ka) += i.myAtom()->El() / 2;
        // if this was a recoil, add a V, a D, & stored energy
        // Efp/2 at the starting & ending cell
        if (i.recoil_id()) {
            assert(iid);
            assert(i.cellid0() >= 0);
            kb = iid * ncells_ + i.cellid0();
            A[cD](kb)++;
            A[cV](kb)++;
            A[eStored](kb) += i.myAtom()->El() / 2;
            A[eLattice](k) += i.myAtom()->El() / 2;
        }
        A[isCollision](k) += i.ncoll();
        A[isFlightPath](k) += i.path();
        A[eIoniz](k) += i.ioniz();
        A[eLattice](k) += i.erg() + i.phonon();
        break;
    case Event::IonStop:
        A[cI](k)++; // add implantation at current (end) posvacancy
        if (i.recoil_id()) { // this is a recoil
            assert(iid);
            assert(i.cellid0() >= 0);
            kb = iid * ncells_ + i.cellid0();
            A[cD](kb)++; // add displacement at start pos
            A[cV](kb)++; // add vacancy at start pos
            A[eStored](kb) += i.myAtom()->El() / 2; // Add half FP energy there
            A[eStored](k) += i.myAtom()->El() / 2; // Add half FP energy here
        }
        A[isCollision](k) += i.ncoll();
        A[isFlightPath](k) += i.path();
        A[eIoniz](k) += i.ioniz();
        A[eLattice](k) += i.erg() + i.phonon();
        break;
    case Event::IonExit:
        A[cL](kp)++;
        if (i.recoil_id()) { // this is a recoil
            assert(iid);
            assert(i.cellid0() >= 0);
            kb = iid * ncells_ + i.cellid0();
            A[cD](kb)++; // add displacement at start pos
            A[cV](kb)++; // add vacancy at start pos
            A[eStored](kb) += i.myAtom()->El() / 2; // Add half FP energy there
            A[eLattice](k) +=
                    i.myAtom()->El() / 2; // half FP energy released as lattice thermal energy
        }
        A[isCollision](kp) += i.ncoll();
        A[isFlightPath](kp) += i.path();
        A[eIoniz](kp) += i.ioniz();
        A[eLattice](kp) += i.phonon();
        A[eLost](kp) += i.erg();
        break;
    case Event::CascadeComplete:
        A[cPKA](k)++;
        p = reinterpret_cast<const float *>(pv);
        A[ePKA](k) += p[0];
        A[dpTdam_LSS](k) += p[1];
        A[dpVnrt_LSS](k) += p[2];
        A[dpTdam](k) += p[3];
        A[dpVnrt](k) += p[4];
        break;
    default:
        break;
    }
}

// #pragma GCC pop_options

bool tally::debugCheck(int id, double E0)
{
    double s(0), sI(0), sPh(0), sL(0);
    size_t ncell = A[1].dim()[1];
    double *p;

    p = &A[eIoniz](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sI += *p++;
    p = &A[eLattice](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sPh += *p++;
    p = &A[eStored](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sPh += *p++;
    p = &A[eLost](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sL += *p++;

    s = sI + sPh + sL;

    assert(std::abs(s - E0) < 1e-3);
    return std::abs(s - E0) < 1e-3;
}

bool tally::debugCheck(double E0)
{
    double s0(0), sI0(0), sPh0(0), sL0(0);
    size_t ncell = A[1].dim()[1];
    double *p;
    int id = 0;

    p = &A[eIoniz](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sI0 += *p++;

    p = &A[eLattice](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sPh0 += *p++;
    p = &A[eStored](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sPh0 += *p++;
    p = &A[eLost](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sL0 += *p++;

    s0 = sI0 + sPh0 + sL0;

    double s(0), sI(0), sPh(0), sL(0);
    size_t n = A[1].size();

    p = A[eIoniz].data();
    for (size_t i = 0; i < n; i++)
        sI += *p++;

    p = &A[eLattice](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sPh += *p++;
    p = &A[eStored](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sPh += *p++;

    p = A[eLost].data();
    for (size_t i = 0; i < n; i++)
        sL += *p++;

    s = sI + sPh + sL;

    assert(std::abs(s - E0) < 1e-3);
    assert(std::abs(s0 - E0) < 1e-3);
    return std::abs(s - E0) < 1e-3;
}

double tally::totalErg(int id)
{
    double s(0), sI(0), sPh(0), sL(0);
    size_t ncell = A[1].dim()[1];
    double *p;

    p = &A[eIoniz](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sI += *p++;
    p = &A[eLattice](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sPh += *p++;
    p = &A[eStored](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sPh += *p++;
    p = &A[eLost](id, 0);
    for (size_t i = 0; i < ncell; i++)
        sL += *p++;

    return sI + sPh + sL;
}

double tally::totalErg()
{
    double s(0), sI(0), sPh(0), sL(0);
    size_t n = A[1].size();
    double *p;

    p = A[eIoniz].data();
    for (size_t i = 0; i < n; i++)
        sI += *p++;
    p = A[eLattice].data();
    for (size_t i = 0; i < n; i++)
        sPh += *p++;
    p = A[eStored].data();
    for (size_t i = 0; i < n; i++)
        sPh += *p++;
    p = A[eLost].data();
    for (size_t i = 0; i < n; i++)
        sL += *p++;

    s = sI + sPh + sL;
    return s;
}
