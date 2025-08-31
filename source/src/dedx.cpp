#include "dedx.h"
#include "periodic_table.h"
#include "mccore.h"

#include <spline.h>

/**************** dedx_interp *****************/
dedx_interp::dedx_interp(StoppingModel m, int Z1, int Z2, float atomicDensity)
{
    float M1 = periodic_table::at(Z1).mass;
    init(m, Z1, M1, { Z2 }, { 1.f }, atomicDensity);
}

dedx_interp::dedx_interp(StoppingModel m, int Z1, float M1, int Z2, float atomicDensity)
{
    init(m, Z1, M1, { Z2 }, { 1.f }, atomicDensity);
}

dedx_interp::dedx_interp(StoppingModel m, int Z1, const std::vector<int> &Z2,
                         const std::vector<float> &X2, float atomicDensity)
{
    float M1 = periodic_table::at(Z1).mass;
    init(m, Z1, M1, Z2, X2, atomicDensity);
}

dedx_interp::dedx_interp(StoppingModel m, int Z1, float M1, const std::vector<int> &Z2,
                         const std::vector<float> &X2, float atomicDensity)
{
    init(m, Z1, M1, Z2, X2, atomicDensity);
}

int dedx_interp::init(StoppingModel m, int Z1, float M1, const std::vector<int> &Z2,
                      const std::vector<float> &X2, float atomicDensity)
{
    assert(Z2.size() == X2.size());
    assert(Z2.size() >= 1);

    std::vector<float> buff(dedx_erange::count, 0.f);

    for (int j = 0; j < Z2.size(); j++) {
        /*
         * dedx tables are stored in eV / 10^15 (at/cm^2)
         * They are multiplied by the atomicDensity [at/nm^3]
         * factor 0.1 needed so that resulting dedx
         * unit is eV/nm
         */
        const float *e, *Se;
        int n;
        stopping_xs(static_cast<STOPPING_MODEL>(m), Z1, Z2[j], &e, &Se, &n);

        std::vector<double> logE(n), logSe(n);
        for (int i = 0; i < n; i++) {
            logE[i] = std::log(1000.0 * e[i] * M1); // from keV/u to eV
            logSe[i] = std::log(Se[i]);
        }
        tk::spline qspl(logE, logSe);
        double E0 = 1000.0 * e[0] * M1;

        float w = X2[j] * atomicDensity * 0.1;
        for (dedx_iterator i; i < dedx_erange::count; i++) {
            double logEi = std::log(1.0 * (*i));
            if (logEi <= logE.front()) {
                buff[i] += w * std::sqrt(1.0 * (*i) / E0) * Se[0];
            } else {
                buff[i] += w * std::exp(qspl(logEi));
            }
        }

        /* The compound correction needs to be added here !!! */
        /* following copied from corteo:
            compound correction according to Zeigler & Manoyan NIMB35(1998)215, Eq.(16) (error in
        Eq. 14) if(compoundCorr!=1.0f) for(k=0; k<DIMD; k++) { f = d2f( 1.0/(1.0+exp( 1.48*(
        sqrt(2.*Dval(k)/projectileMass/25e3) -7.0) )) ); spp[k]*=f*(compoundCorr-1.0f)+1.0f;
        }
        */
    }

    set(buff);

    return 0;
}

/******************* straggling_interp *************************/

void calcStraggling(const dedx_interp &dedx_ion, const dedx_interp &dedx_H, int Z1, const float &M1,
                    int Z2, const float &Ns, StragglingModel model, float *strag);

straggling_interp::straggling_interp(StoppingModel mstop, StragglingModel mstrag, int Z1, float M1,
                                     int Z2, float N)
{
    init(mstop, mstrag, Z1, M1, { Z2 }, { 1.f }, N);
}

straggling_interp::straggling_interp(StoppingModel mstop, StragglingModel mstrag, int Z1, float M1,
                                     const std::vector<int> &Z2, const std::vector<float> &X2,
                                     float N)
{
    init(mstop, mstrag, Z1, M1, Z2, X2, N);
}

int straggling_interp::init(StoppingModel mstop, StragglingModel mstrag, int Z1, float M1,
                            const std::vector<int> &Z2, const std::vector<float> &X2,
                            float atomicDensity)
{
    dedx_interp dedx_ion(mstop, Z1, M1, Z2, X2, atomicDensity);
    dedx_interp dedx_H(mstop, 1, periodic_table::at(1).mass, Z2, X2, atomicDensity);

    std::vector<float> buff(dedx_erange::count, 0.f);

    float Rat = std::pow(3.0 / 4.0 / M_PI / atomicDensity, 1.0 / 3);
    float Nl0 = atomicDensity * Rat;
    for (int i = 0; i < Z2.size(); i++)
        calcStraggling(dedx_ion, dedx_H, Z1, M1, Z2[i], Nl0 * X2[i], mstrag, buff.data());
    for (dedx_iterator ie; ie < dedx_erange::count; ie++)
        buff[ie] = std::sqrt(buff[ie] / Rat);

    set(buff);

    return 0;
}

/******************* dedx_calc *************************/

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
            dedx_(iat1, im) = new dedx_interp(par.stopping_model, at1->Z(), at1->M(), mat->Z(),
                                              mat->X(), mat->atomicDensity());
            de_strag_(iat1, im) =
                    new straggling_interp(par.stopping_model, par.straggling_model, at1->Z(),
                                          at1->M(), mat->Z(), mat->X(), mat->atomicDensity());
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
