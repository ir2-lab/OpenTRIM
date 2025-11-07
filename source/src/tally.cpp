#include "tally.h"
#include "event_stream.h"
#include "target.h"

const char *event_name(Event ev)
{
    switch (ev) {
    case Event::NewSourceIon:
        return "NewSourceIon";
        break;
    case Event::NewRecoil:
        return "NewRecoil";
        break;
    case Event::Scattering:
        return "Scattering";
        break;
    case Event::IonExit:
        return "IonExit";
        break;
    case Event::IonStop:
        return "IonStop";
        break;
    case Event::BoundaryCrossing:
        return "BoundaryCrossing";
        break;
    case Event::Replacement:
        return "Replacement";
        break;
    case Event::Vacancy:
        return "Vacancy";
        break;
    case Event::CascadeComplete:
        return "CascadeComplete";
        break;
    case Event::NewFlightPath:
        return "NewFlightPath";
        break;
    case Event::NEvent:
        return "NEvent";
        break;
    case Event::Invalid:
        return "Invalid";
        break;
    default:
        return "Unknown";
        break;
    }
}

const char *event_description(Event ev)
{
    switch (ev) {
    case Event::NewSourceIon:
        return "A new ion track is started.";
        break;
    case Event::NewRecoil:
        return "A new recoil track is started.";
        break;
    case Event::Scattering:
        return "An ion scattering occured.";
        break;
    case Event::IonExit:
        return "An ion exits the simulation volume.";
        break;
    case Event::IonStop:
        return "An ion stops inside the simulation volume.";
        break;
    case Event::BoundaryCrossing:
        return "An ion crosses an internal boundary.";
        break;
    case Event::Replacement:
        return "A replacement event occurs.";
        break;
    case Event::Vacancy:
        return "A vacancy is created.";
        break;
    case Event::CascadeComplete:
        return "A PKA cascade is complete.";
        break;
    case Event::NewFlightPath:
        return "";
        break;
    case Event::NEvent:
        return "";
        break;
    case Event::Invalid:
        return "Invalid";
        break;
    default:
        return "Unknown";
        break;
    }
}

const char *tally::arrayName(int i)
{
    static const char *names[] = {
        "Totals",   "Vacancies", "Implantations", "Replacements", "Recombinations", "Ionization",
        "Lattice",  "Stored",    "Lost",          "Pka",          "Pka_energy",     "Tdam",
        "Tdam_LSS", "Vnrt",      "Vnrt_LSS",      "Flight_path",  "Collisions",     "Lost",
        "X"
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
                                  "Recombinations",
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
    int iid = i.myAtom()->id();
    size_t k;
    const pka_buffer *p;

    switch (ev) {

    case Event::BoundaryCrossing:
        k = iid * ncells_ + i.prev_cellid();
        A[isCollision](k) += i.ncoll();
        A[isFlightPath](k) += i.path();
        A[eLattice](k) += i.phonon();
        A[eIoniz](k) += i.ioniz();
        ionizationCounter_ += i.ioniz();
        break;

    case Event::Replacement:
        k = iid * ncells_ + i.cellid();
        A[cR](k)++; // this atom, current cell
        A[isCollision](k) += i.ncoll();
        A[isFlightPath](k) += i.path();
        A[eIoniz](k) += i.ioniz();
        A[eLattice](k) += i.erg() + i.phonon();
        ionizationCounter_ += i.ioniz();
        break;

    case Event::IonStop:
        k = iid * ncells_ + i.cellid();
        A[cI](k)++; // add implantation at current pos
        if (i.recoil_id()) // if this is a recoil (not a beam ion)
            A[eStored](k) += i.myAtom()->El() / 2; // Add half FP energy here to stored energy
        A[isCollision](k) += i.ncoll();
        A[isFlightPath](k) += i.path();
        A[eIoniz](k) += i.ioniz();
        A[eLattice](k) += i.erg() + i.phonon();
        ionizationCounter_ += i.ioniz();
        break;

    case Event::Vacancy:
        k = iid * ncells_ + i.cellid();
        A[cV](k)++; // add a vacancy at current pos
        A[eStored](k) += i.myAtom()->El() / 2; // Add half FP energy here to stored energy
        break;

    case Event::IonExit:
        k = iid * ncells_ + i.prev_cellid();
        A[cL](k)++;
        // if it was a recoil
        // half FP energy is released as lattice thermal energy
        if (i.recoil_id())
            A[eLattice](k) += i.myAtom()->El() / 2;
        A[isCollision](k) += i.ncoll();
        A[isFlightPath](k) += i.path();
        A[eIoniz](k) += i.ioniz();
        A[eLattice](k) += i.phonon();
        A[eLost](k) += i.erg();
        ionizationCounter_ += i.ioniz();
        break;

    case Event::CascadeComplete:
        k = iid * ncells_ + i.cellid();
        A[cPKA](k)++;
        // pv = pointer to pka_event struct
        p = reinterpret_cast<const pka_buffer *>(pv);
        A[ePKA](k) += p->recoilE();
        A[dpTdam_LSS](k) += p->Tdam_LSS();
        A[dpVnrt_LSS](k) += p->NRT_LSS();
        A[dpTdam](k) += p->Tdam();
        A[dpVnrt](k) += p->NRT();
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
