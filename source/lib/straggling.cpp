#include "dedx.h"

#include <cmath>

#define E2 1.43996445 /* e^2 / 4 pi eps0 = e^2 c^2 in [eV][nm] */

/*
 * Bohr straggling correction parameter according to
 * W. K. Chu, Phys. Rev. A13 (1976) 2057; values extracted
 * from Q.Yang, D.J. O'Connor, Z. Wang, Nucl. Instr. Meth. B61 (1991) 149
 *
 * Table is Z x 4 coefficients, 0<=Z<=97
 */
static const float chu_coef[][4] = {
    { +0.0000f, +0.0000f, +0.0000f, +0.0000f }, { +0.0000f, +0.0000f, +0.0000f, +0.0000f },
    { -0.3291f, -0.8312f, +0.2460f, -1.0220f }, { -0.5615f, -0.5898f, +0.5205f, -0.7258f },
    { -0.5280f, -0.4981f, +0.5519f, -0.5865f }, { -0.5125f, -0.4625f, +0.5660f, -0.5190f },
    { -0.5127f, -0.8595f, +0.5626f, -0.8721f }, { -0.5174f, -1.1930f, +0.5565f, -1.1980f },
    { -0.5179f, -1.1850f, +0.5560f, -1.2070f }, { -0.5209f, -0.9355f, +0.5590f, -1.0250f },
    { -0.5255f, -0.7766f, +0.5720f, -0.9412f }, { -0.5776f, -0.6665f, +0.6598f, -0.8484f },
    { -0.6013f, -0.6045f, +0.7321f, -0.7671f }, { -0.5781f, -0.5518f, +0.7605f, -0.6919f },
    { -0.5587f, -0.4981f, +0.7835f, -0.6195f }, { -0.5466f, -0.4656f, +0.7978f, -0.5771f },
    { -0.5406f, -0.4690f, +0.8031f, -0.5718f }, { -0.5391f, -0.5061f, +0.8024f, -0.5974f },
    { -0.5380f, -0.6483f, +0.7962f, -0.6970f }, { -0.5355f, -0.7722f, +0.7962f, -0.7839f },
    { -0.5329f, -0.7720f, +0.7988f, -0.7846f }, { -0.5335f, -0.7671f, +0.7984f, -0.7933f },
    { -0.5324f, -0.7612f, +0.7998f, -0.8031f }, { -0.5305f, -0.7300f, +0.8031f, -0.7990f },
    { -0.5307f, -0.7178f, +0.8049f, -0.8216f }, { -0.5248f, -0.6621f, +0.8165f, -0.7919f },
    { -0.5180f, -0.6502f, +0.8266f, -0.7986f }, { -0.5084f, -0.6408f, +0.8396f, -0.8048f },
    { -0.4967f, -0.6331f, +0.8549f, -0.8093f }, { -0.4861f, -0.6508f, +0.8712f, -0.8432f },
    { -0.4700f, -0.6186f, +0.8961f, -0.8132f }, { -0.4545f, -0.5720f, +0.9227f, -0.7710f },
    { -0.4404f, -0.5226f, +0.9481f, -0.7254f }, { -0.4288f, -0.4778f, +0.9701f, -0.6850f },
    { -0.4199f, -0.4425f, +0.9874f, -0.6539f }, { -0.4131f, -0.4188f, +0.9998f, -0.6332f },
    { -0.4089f, -0.4057f, +1.0070f, -0.6218f }, { -0.4039f, -0.3913f, +1.0150f, -0.6107f },
    { -0.3987f, -0.3698f, +1.0240f, -0.5938f }, { -0.3977f, -0.3608f, +1.0260f, -0.5852f },
    { -0.3972f, -0.3600f, +1.0260f, -0.5842f }, { -0.3985f, -0.3803f, +1.0200f, -0.6013f },
    { -0.3985f, -0.3979f, +1.0150f, -0.6168f }, { -0.3968f, -0.3990f, +1.0160f, -0.6195f },
    { -0.3971f, -0.4432f, +1.0050f, -0.6591f }, { -0.3944f, -0.4665f, +1.0010f, -0.6825f },
    { -0.3924f, -0.5109f, +0.9921f, -0.7235f }, { -0.3882f, -0.5158f, +0.9947f, -0.7343f },
    { -0.3838f, -0.5125f, +0.9999f, -0.7370f }, { -0.3786f, -0.4976f, +1.0090f, -0.7310f },
    { -0.3741f, -0.4738f, +1.0200f, -0.7155f }, { -0.3969f, -0.4496f, +1.0320f, -0.6982f },
    { -0.3663f, -0.4297f, +1.0430f, -0.6828f }, { -0.3630f, -0.4120f, +1.0530f, -0.6689f },
    { -0.3597f, -0.3964f, +1.0620f, -0.6564f }, { -0.3555f, -0.3809f, +1.0720f, -0.6454f },
    { -0.3525f, -0.3607f, +1.0820f, -0.6289f }, { -0.3505f, -0.3465f, +1.0900f, -0.6171f },
    { -0.3397f, -0.3570f, +1.1020f, -0.6384f }, { -0.3314f, -0.3552f, +1.1130f, -0.6441f },
    { -0.3235f, -0.3531f, +1.1230f, -0.6498f }, { -0.3150f, -0.3483f, +1.1360f, -0.6539f },
    { -0.3060f, -0.3441f, +1.1490f, -0.6593f }, { -0.2968f, -0.3396f, +1.1630f, -0.6649f },
    { -0.2935f, -0.3225f, +1.1760f, -0.6527f }, { -0.2797f, -0.3262f, +1.1940f, -0.6722f },
    { -0.2704f, -0.3202f, +1.2100f, -0.6770f }, { -0.2815f, -0.3227f, +1.2480f, -0.6775f },
    { -0.2880f, -0.3245f, +1.2810f, -0.6801f }, { -0.3034f, -0.3263f, +1.3270f, -0.6778f },
    { -0.2936f, -0.3215f, +1.3430f, -0.6835f }, { -0.3282f, -0.3200f, +1.3980f, -0.6650f },
    { -0.3260f, -0.3070f, +1.4090f, -0.6552f }, { -0.3511f, -0.3074f, +1.4470f, -0.6442f },
    { -0.3501f, -0.3064f, +1.4500f, -0.6442f }, { -0.3490f, -0.3027f, +1.4550f, -0.6418f },
    { -0.3487f, -0.3048f, +1.4570f, -0.6447f }, { -0.3478f, -0.3074f, +1.4600f, -0.6483f },
    { -0.3501f, -0.3283f, +1.4540f, -0.6669f }, { -0.3494f, -0.3373f, +1.4550f, -0.6765f },
    { -0.3485f, -0.3373f, +1.4560f, -0.6774f }, { -0.3462f, -0.3300f, +1.4630f, -0.6728f },
    { -0.3462f, -0.3225f, +1.4690f, -0.6662f }, { -0.3453f, -0.3094f, +1.4790f, -0.6553f },
    { -0.3844f, -0.3134f, +1.5240f, -0.6412f }, { -0.3848f, -0.3018f, +1.5310f, -0.6303f },
    { -0.3862f, -0.2955f, +1.5360f, -0.6237f }, { -0.4262f, -0.2991f, +1.5860f, -0.6115f },
    { -0.4278f, -0.2910f, +1.5900f, -0.6029f }, { -0.4303f, -0.2817f, +1.5940f, -0.5927f },
    { -0.4315f, -0.2719f, +1.6010f, -0.5829f }, { -0.4359f, -0.2914f, +1.6050f, -0.6010f },
    { -0.4365f, -0.2982f, +1.6080f, -0.6080f }, { -0.4253f, -0.3037f, +1.6120f, -0.6150f },
    { -0.4335f, -0.3245f, +1.6160f, -0.6377f }, { -0.4307f, -0.3292f, +1.6210f, -0.6447f },
    { -0.4284f, -0.3204f, +1.6290f, -0.6380f }, { -0.4227f, -0.3217f, +1.6360f, -0.6438f }
};

/**
 * @brief Calculate ion straggling
 *
 * Calculates the ion straggling parameters for an ion Z1
 * traveling in a target with atomic number Z2 and sheet density Ns.
 *
 * Straggling is calculated for ion energies spanning the corteo dedx_index.
 *
 * The calculated straggling parameter \f$\Delta E_s\f$ is essentially the std of energy loss
 * fluctuation per square root of \f$x/R_{at}\f$, where \f$x\f$ is the travelled
 * ion path and \f$R_{at}\f$ the atomic radius. \f$\Delta E_s\f$ is given in eV
 * if \f$N_s = N\cdot R_{at}\f$ is given in at/nm^2
 *
 * In a MC simulation, the actual straggling value is \f$\Delta E_s\cdot u\f$,
 * where \f$u\f$ is sampled from the normal distribution \f$N(0,1)\f$.
 *
 * @param dedx_ion ion stopping interpolator
 * @param dedx_H proton stopping interpolator
 * @param Z1 is the projectile atomic number
 * @param M1 is the projectile atomic mass
 * @param Z2 target atomic number
 * @param Ns target sheet density [at/nm^2]
 * @param model straggling model
 * @param strag pointer to table to receive straggling values [eV]
 */
void calcStraggling(const dedx_interp &dedx_ion, const dedx_interp &dedx_H, int Z1, const float &M1,
                    int Z2, const float &Ns, StragglingModel model, float *strag)

{
    /*
     * Start by calculating squared Bohr straggling
     * (all other models need this anyway)
     */
    double OmegaBohr2 = 4 * M_PI * Z1 * Z1 * Z2 * E2 * E2 * Ns; /* eV^2 */
    ;

    for (dedx_iterator ie; ie != ie.end(); ie++) {
        double energy = *ie;
        double stopping_ion = dedx_ion(*ie);
        double stopping_H = dedx_H(*ie / M1);

        /*
         * the effective charge state is obtained from comparing
         * stopping of the ion and the hydrogen:
         * chargestate^2 = stopping(H)/stopping(ion) Z_ion^2
         * for stopping at same speed
         */
        double chargestate2 = stopping_ion / stopping_H / Z1 / Z1;
        // double chargestate2 = dedx1[je]/stopping/Z1/Z1;

        /*
         * Calculate the Chu correction factor.
         * The formula needs energy[MeV]/mass
         *
         * Chu values undefined for target_Z==1, because the Chu table
         * has no data on hydrogen --> use Bohr.
         */
        double MEV_energy_amu = energy * 1e-6 / M1;
        double Chu_factor = 1.0;
        if (Z2 > 1) {
            const float *cc = chu_coef[Z2];
            Chu_factor = cc[0] * std::pow(MEV_energy_amu, cc[1])
                    + cc[2] * std::pow(MEV_energy_amu, cc[3]);
            Chu_factor = 1.0 / (1.0 + Chu_factor);
        }

        /*
         * To calculate Yang's extra straggling contribution
         * caused by charge state fluctuations, we need his
         * Gamma and his epsilon (eq.6-8 from the paper):
         *
         * For hydrogen we need the B and for other projectile
         * the C constants:
         */
        double Yang;
        if (Z1 == 1) {
            static const double B[] = { 0., 0.1955, 0.6941, 2.522, 1.040 };
            double Gamma = B[3] * (1.0 - exp(-B[4] * MEV_energy_amu));
            Yang = B[1] * Gamma / (pow((MEV_energy_amu - B[2]), 2.0) + Gamma * Gamma);
        } else {
            static const double C[] = { 0., 1.273e-2, 3.458e-2, 0.3931, 3.812 };
            double epsilon = MEV_energy_amu * pow(Z1, -1.5) * pow(Z2, -0.5); /* solid targets */
            double Gamma = C[3] * (1.0 - exp(-C[4] * epsilon));
            Yang = (pow(Z1, 1.333333333333) / pow(Z2, 0.33333333333)) * C[1] * Gamma
                    / (pow((epsilon - C[2]), 2.0) + Gamma * Gamma);
        }

        /*
         * Now we have all ingredients for any of the straggling models.
         * We could have saved some calculations by checking the model first,
         * but well... we'll probably use Yang's model in most cases
         */
        switch (model) {
        case StragglingModel::Bohr: /* Bohr */
            strag[ie] += OmegaBohr2;
            break;
        case StragglingModel::Chu: /* Chu */
            strag[ie] += OmegaBohr2 * Chu_factor;
            break;
        case StragglingModel::Yang: /* Chu + Yang correction */
            strag[ie] += OmegaBohr2 * (chargestate2 * Chu_factor + Yang);
            break;
        default:
            assert(0); // never get here
        }
    }
}
