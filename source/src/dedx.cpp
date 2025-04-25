#include "dedx.h"

extern const double *most_abundant_isotope_mass;

void calcStraggling(const dedx_interp &dedx_ion, const dedx_interp &dedx_H, int Z1, const float &M1,
                    int Z2, const float &Ns, StragglingModel model, float *strag);

extern const float **dedx_data[];

const float *raw_dedx(int Z1, int Z2)
{
    return (Z1 && Z2 && Z1 >= 1 && Z1 <= dedx_max_Z && Z2 >= 1 && Z2 <= dedx_max_Z)
            ? dedx_data[Z1][Z2]
            : nullptr;
}

dedx_interp::dedx_interp(int Z1, int Z2, float atomicDensity)
{
    init(Z1, most_abundant_isotope_mass[Z1], { Z2 }, { 1.f }, atomicDensity);
}

dedx_interp::dedx_interp(int Z1, float M1, int Z2, float atomicDensity)
{
    init(Z1, M1, { Z2 }, { 1.f }, atomicDensity);
}

dedx_interp::dedx_interp(int Z1, const std::vector<int> &Z2, const std::vector<float> &X2,
                         float atomicDensity)
{
    init(Z1, most_abundant_isotope_mass[Z1], Z2, X2, atomicDensity);
}

dedx_interp::dedx_interp(int Z1, float M1, const std::vector<int> &Z2, const std::vector<float> &X2,
                         float atomicDensity)
{
    init(Z1, M1, Z2, X2, atomicDensity);
}

int dedx_interp::init(int Z1, float M1, const std::vector<int> &Z2, const std::vector<float> &X2,
                      float atomicDensity)
{
    assert(Z2.size() == X2.size());
    assert(Z2.size() >= 1);

    std::vector<float> buff(dedx_index::size, 0.f);
    float amuRatio = most_abundant_isotope_mass[Z1] / M1;

    for (int j = 0; j < Z2.size(); j++) {
        /*
         * SRIM dedx tables are stored in eV / 10^15 (at/cm^2)
         * They are multiplied by the atomicDensity [at/nm^3]
         * factor 0.1 needed so that resulting dedx
         * unit is eV/nm
         */
        const float *q = raw_dedx(Z1, Z2[j]);
        float w = X2[j] * atomicDensity * 0.1;
        for (dedx_index i; i < i.end(); i++) {
            float erg = (*i) * amuRatio;
            if (erg <= dedx_index::minVal) {
                buff[i] += w * std::sqrt(erg / dedx_index::minVal) * q[0];
            } else if (erg >= dedx_index::maxVal) {
                buff[i] += w * q[dedx_index::size - 1];
            } else {
                dedx_index ie0(erg), ie1(ie0);
                ie1++;
                float d = std::log(q[ie1] / q[ie0]) / std::log((*ie1) / (*ie0));
                buff[i] += w * q[ie0] * std::pow(erg / (*ie0), d);
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

straggling_interp::straggling_interp(StragglingModel model, int Z1, float M1, int Z2, float N)
{
    init(model, Z1, M1, { Z2 }, { 1.f }, N);
}

straggling_interp::straggling_interp(StragglingModel model, int Z1, float M1,
                                     const std::vector<int> &Z2, const std::vector<float> &X2,
                                     float N)
{
    init(model, Z1, M1, Z2, X2, N);
}

int straggling_interp::init(StragglingModel model, int Z1, float M1, const std::vector<int> &Z2,
                            const std::vector<float> &X2, float atomicDensity)
{
    dedx_interp dedx_ion(Z1, M1, Z2, X2, atomicDensity);
    dedx_interp dedx_H(1, most_abundant_isotope_mass[1], Z2, X2, atomicDensity);

    std::vector<float> buff(dedx_index::size, 0.f);

    float Rat = std::pow(3.0 / 4.0 / M_PI / atomicDensity, 1.0 / 3);
    float Nl0 = atomicDensity * Rat;
    for (int i = 0; i < Z2.size(); i++)
        calcStraggling(dedx_ion, dedx_H, Z1, M1, Z2[i], Nl0 * X2[i], model, buff.data());
    for (dedx_index ie; ie != ie.end(); ie++)
        buff[ie] = std::sqrt(buff[ie] / Rat);

    set(buff);

    return 0;
}
