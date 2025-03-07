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
    S.push_back("histories");
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
    const float *p;
    const atom *a;
    switch (ev) {
    case Event::BoundaryCrossing:
        A[isCollision](iid, pid) += i.ncoll();
        A[isFlightPath](iid, pid) += i.path();
        A[eLattice](iid, pid) += i.phonon();
        A[eIoniz](iid, pid) += i.ioniz();
        break;
    case Event::Replacement:
        // add a replacement
        A[cR](iid, cid)++; // this atom, current cell
        // remove a vac for the replaced atom (id passed in pointer pv), in current cell
        // and also half FP stored energy
        a = reinterpret_cast<const atom *>(pv);
        assert(a->id());
        assert(cid >= 0);
        A[cV](a->id(), cid)--;
        A[eStored](a->id(), cid) -= i.myAtom()->El() / 2;
        A[eLattice](a->id(), cid) += i.myAtom()->El() / 2;
        // if this was a recoil, add a V, a D, & stored energy
        // Efp/2 at the starting & ending cell
        if (i.recoil_id()) {
            assert(iid);
            assert(i.cellid0() >= 0);
            A[cD](iid, i.cellid0())++;
            A[cV](iid, i.cellid0())++;
            A[eStored](iid, i.cellid0()) += i.myAtom()->El() / 2;
            A[eLattice](iid, cid) += i.myAtom()->El() / 2;
        }
        A[isCollision](iid, cid) += i.ncoll();
        A[isFlightPath](iid, cid) += i.path();
        A[eIoniz](iid, cid) += i.ioniz();
        A[eLattice](iid, cid) += i.erg() + i.phonon();
        break;
    case Event::IonStop:
        A[cI](iid, cid)++; // add implantation at current (end) posvacancy
        if (i.recoil_id()) { // this is a recoil
            assert(iid);
            assert(i.cellid0() >= 0);
            A[cD](iid, i.cellid0())++; // add displacement at start pos
            A[cV](iid, i.cellid0())++; // add vacancy at start pos
            A[eStored](iid, i.cellid0()) += i.myAtom()->El() / 2; // Add half FP energy there
            A[eStored](iid, cid) += i.myAtom()->El() / 2; // Add half FP energy here
        }
        A[isCollision](iid, cid) += i.ncoll();
        A[isFlightPath](iid, cid) += i.path();
        A[eIoniz](iid, cid) += i.ioniz();
        A[eLattice](iid, cid) += i.erg() + i.phonon();
        break;
    case Event::IonExit:
        A[cL](iid, pid)++;
        if (i.recoil_id()) { // this is a recoil
            assert(iid);
            assert(i.cellid0() >= 0);
            A[cD](iid, i.cellid0())++; // add displacement at start pos
            A[cV](iid, i.cellid0())++; // add vacancy at start pos
            A[eStored](iid, i.cellid0()) += i.myAtom()->El() / 2; // Add half FP energy there
            A[eLattice](iid, cid) +=
                    i.myAtom()->El() / 2; // half FP energy released as lattice thermal energy
        }
        A[isCollision](iid, pid) += i.ncoll();
        A[isFlightPath](iid, pid) += i.path();
        A[eIoniz](iid, pid) += i.ioniz();
        A[eLattice](iid, pid) += i.phonon();
        A[eLost](iid, pid) += i.erg();
        break;
    case Event::CascadeComplete:
        A[cPKA](iid, cid)++;
        p = reinterpret_cast<const float *>(pv);
        A[ePKA](iid, cid) += p[0];
        A[dpTdam_LSS](iid, cid) += p[1];
        A[dpVnrt_LSS](iid, cid) += p[2];
        A[dpTdam](iid, cid) += p[3];
        A[dpVnrt](iid, cid) += p[4];
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
