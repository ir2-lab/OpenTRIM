#include "flight_path.h"

#include "mccore.h"

flight_path_calc::flight_path_calc() { }

flight_path_calc::flight_path_calc(const flight_path_calc &o)
    : type_(o.type_),
      fp_const(o.fp_const),
      ip0(o.ip0),
      mfp_(o.mfp_),
      ipmax_(o.ipmax_),
      fp_max_(o.fp_max_),
      umin_(o.umin_)
{
}

int flight_path_calc::init(const mccore &s)
{
    auto &trgt = s.getTarget();
    auto &tr_opt_ = s.getTransportOptions();

    type_ = tr_opt_.flight_path_type;

    // calculate sqrt{l/l0} in each material for constant flight path
    auto &materials = trgt.materials();
    // auto& tr_opt_ = s_->getTransportOptions();
    int nmat = materials.size();
    fp_const.resize(nmat);
    ip0.resize(nmat);
    for (int i = 0; i < nmat; i++) {
        if (type_ == Constant) {
            ip0[i] = SQRT_4over3 / std::sqrt(tr_opt_.flight_path_const)
                    * materials[i]->atomicRadius();
            fp_const[i] = tr_opt_.flight_path_const * materials[i]->atomicRadius();
        } else {
            fp_const[i] = materials[i]->atomicRadius();
            ip0[i] = SQRT_4over3 * materials[i]->atomicRadius();
        }
    }

    /*
     * create tables of parameters for flight path selection,
     * (mfp, ipmax, fpmax, umin)
     *
     * @sa flightpath
     */
    auto &atoms = trgt.atoms();
    int natoms = atoms.size();
    int nerg = fp_tbl_erange::count;
    auto ScMatrix = s.scattering_matrix();
    auto dedx = s.get_dedx_calc().dedx();
    mfp_ = ArrayNDf(natoms, nmat, nerg);
    ipmax_ = ArrayNDf(natoms, nmat, nerg);
    fp_max_ = ArrayNDf(natoms, nmat, nerg);
    umin_ = ArrayNDf(natoms, nmat, nerg);
    float delta_dedx = tr_opt_.max_rel_eloss;
    float Tmin = tr_opt_.min_recoil_energy;

    for (int z1 = 0; z1 < natoms; z1++) {
        for (int im = 0; im < materials.size(); im++) {
            const material *m = materials[im];
            const float &N = m->atomicDensity();
            const float &Rat = m->atomicRadius();
            float mfp_lb = tr_opt_.mfp_range[0] * Rat;
            float mfp_ub = tr_opt_.mfp_range[1] * Rat;

            for (fp_tbl_iterator ie; ie != ie.end(); ie++) {
                float &mfp = mfp_(z1, im, ie);
                float &ipmax = ipmax_(z1, im, ie);
                float &fpmax = fp_max_(z1, im, ie);
                float &umin = umin_(z1, im, ie);

                if (type_ == Constant) {

                    // set const mfp
                    mfp = tr_opt_.flight_path_const * Rat;
                    ipmax = std::sqrt(1.f / M_PI / mfp / N);
                    fpmax = mfp;
                    umin = 0;

                } else {

                    float E = *ie;
                    float T0 = Tmin;

                    // find the lowest T0 so that T0 < Tmin & Theta < Theta_min
                    for (const atom *a : m->atoms()) {
                        int z2 = a->id();
                        float Tm = E * ScMatrix(z1, z2)->gamma();
                        float theta_min = tr_opt_.min_scattering_angle / 180.f * M_PI;
                        theta_min *= (1 + ScMatrix(z1, z2)->mass_ratio());
                        float ss = std::sin(0.5 * theta_min);
                        float T0_ = Tm * ss * ss;
                        if (T0 > T0_)
                            T0 = T0_;
                    }

                    // Calc mfp corresponding to T0, mfp = 1/(N*sig0), sig0 = pi*sum_i{ X_i *
                    // [P_i(E,T0)]^2 }
                    ipmax = 0.f;
                    for (const atom *a : m->atoms()) {
                        int z2 = a->id();
                        float d = ScMatrix(z1, z2)->find_p(E, T0);
                        ipmax += a->X() * d * d;
                    }
                    ipmax = std::sqrt(ipmax);

                    // calc fpmax : fpmax*dEdx/E < max_rel_eloss
                    fpmax = 1e30f;
                    if (s.getSimulationParameters().electronic_stopping
                        != dedx_calc::electronic_stopping_t::Off) {
                        fpmax = delta_dedx * E / dedx(z1, im)->data()[ie];
                    }

                    // calc mfp
                    mfp = 1.f / M_PI / N / ipmax / ipmax;

                    // ensure mfp not smaller than lower bound
                    if (mfp < mfp_lb) {
                        mfp = mfp_lb;
                        ipmax = std::sqrt(1.f / M_PI / mfp / N);
                    }

                    // ensure mfp not larger than upper bound
                    if (mfp > mfp_ub) {
                        mfp = mfp_ub;
                        ipmax = std::sqrt(1.f / M_PI / mfp / N);
                    }

                    // Variable: u<umin -> reject collision
                    umin = std::exp(-fpmax / mfp);
                }

                // Calc dedxn for  T<T0 = N*sum_i { X_i * Sn(E,T0) }
                // Add this to dedx
                /// @todo: this is very slow. dedxn is very small, can be ignored
                // We need to re-calc T0 from mfp
                /// @todo: solve ipmax^2 = sum_i { X_i * ipmax_i(e,T0) } for T0
                //                int z2 = m->atoms().front()->id();
                //                float s1,c1;
                //                scattering_matrix_(z1,z2)->scatter(E,ipmax,T0,s1,c1);
                //                dedxn = 0;
                //                for(const atom* a : m->atoms()) {
                //                    int z2 = a->id();
                //                    dedxn += scattering_matrix_(z1,z2)->stoppingPower(E,T0) *
                //                    a->X();
                //                }
                //                dedxn *= N;
                //                if (tr_opt_.flight_path_type == MyFFP) dedx_(z1,im,ie) += dedxn;

            } // energy
        } // material
    } // Z1

    return 0;
}

int flight_path_calc::preload(const ion *i, const material *m)
{

    assert(i);
    assert(m);

    int iid = i->myAtom()->id();
    int mid = m->id();

    fp_ = fp_const[mid];
    ip_ = ip0[mid];

    switch (type_) {
    case Variable:
        ipmax_tbl = &(ipmax_(iid, mid, 0));
        mfp_tbl = &(mfp_(iid, mid, 0));
        fpmax_tbl = &(fp_max_(iid, mid, 0));
        umin_tbl = &(umin_(iid, mid, 0));
        break;
    default:
        break;
    }

    return 0;
}
