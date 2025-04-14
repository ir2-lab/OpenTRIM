#include "dedx_calc.h"

#include "mccore.h"

dedx_calc::dedx_calc() { }

dedx_calc::dedx_calc(const dedx_calc &other)
    : type_(other.type_), dedx_(other.dedx_), de_strag_(other.de_strag_)
{
}

dedx_calc::~dedx_calc()
{
    if (!dedx_.isNull() && dedx_.use_count() == 1) {
        dedx_interp **p = dedx_.data();
        for (int i = 0; i < dedx_.size(); i++)
            delete p[i];
    }
    if (!de_strag_.isNull() && de_strag_.use_count() == 1) {
        straggling_interp **p = de_strag_.data();
        for (int i = 0; i < de_strag_.size(); i++)
            delete p[i];
    }
}

int dedx_calc::init(const mccore &s)
{
    /*
     * create dedx and straggling tables for all ion - material
     * combinations, # =
     * (all target atoms + projectile ) x (all target materials)
     * For each combi, get an interpolator object
     */
    auto &materials = s.getTarget().materials();
    auto &par = s.getSimulationParameters();
    type_ = par.eloss_calculation;
    int nmat = materials.size();
    auto &atoms = s.getTarget().atoms();
    int natoms = atoms.size();
    dedx_ = ArrayND<dedx_interp *>(natoms, nmat);
    de_strag_ = ArrayND<straggling_interp *>(natoms, nmat);
    for (atom *at1 : atoms) {
        int iat1 = at1->id();
        for (const material *mat : materials) {
            int im = mat->id();
            auto desc = mat->getDescription();
            dedx_(iat1, im) =
                    new dedx_interp(at1->Z(), at1->M(), mat->Z(), mat->X(), mat->atomicDensity());
            de_strag_(iat1, im) = new straggling_interp(par.straggling_model, at1->Z(), at1->M(),
                                                        mat->Z(), mat->X(), mat->atomicDensity());
        }
    }

    return 0;
}

int dedx_calc::preload(const ion *i, const material *m)
{
    assert(i);
    assert(m);
    int ia = i->myAtom()->id();
    int im = m->id();
    stopping_interp_ = dedx_(ia, im);
    straggling_interp_ = type_ == EnergyLossAndStraggling ? de_strag_(ia, im) : nullptr;
    return 0;
}
