#include "flight_path_calc.h"

#include "mccore.h"

flight_path_calc::flight_path_calc() { }

flight_path_calc::flight_path_calc(const flight_path_calc &o)
    : type_(o.type_),
      fp_const(o.fp_const),
      sqrtfp_const(o.sqrtfp_const),
      ip0(o.ip0),
      mfp_(o.mfp_),
      ipmax_(o.ipmax_),
      fp_max_(o.fp_max_),
      Tcutoff_(o.Tcutoff_),
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
    sqrtfp_const.resize(nmat);
    ip0.resize(nmat);
    for (int i = 0; i < nmat; i++) {
        fp_const[i] = tr_opt_.flight_path_const;
        sqrtfp_const[i] = std::sqrt(fp_const[i] / materials[i]->atomicRadius());
        ip0[i] = materials[i]->meanImpactPar();
        if (type_ == Constant)
            ip0[i] /= sqrtfp_const[i];
    }

    /*
     * create tables of parameters for flight path selection, pairs of (mfp, ipmax)
     *
     * - Set a low cutoff recoil energy T0 (min_recoil_energy)
     * - T0 = min(min_recoil_energy, 0.01*Tm)
     * - Scattering events with T<T0 are not considered in the MC
     * - T0/Tm = sin(th0/2)^2, th0 : low cutoff scattering angle
     * - ipmax = ip(e,th0) : max impact parameter
     * - sig0 = pi*ipmax^2 : total xs for events T>T0
     * - mfp = 1/(N*sig0) : mean free path
     *
     * Additional conditions:
     *
     * - Electron energy loss
     *   - Δxmax for el. energy loss below 5%
     *   - mfp = min(mfp, Dxmax)
     *
     * - Allow flight path < atomic radius Rat ??
     *   - if not then mfp = max(mfp, Rat)
     *
     * - User selected max_mfp
     *   - mfp = min(mfp, max_mfp)

     * Finally:
     *
     * - ipmax = 1/(pi*mfp*N)^(1/2)
     *
     */
    auto &atoms = trgt.atoms();
    int natoms = atoms.size();
    int nerg = dedx_index::size;
    auto ScMatrix = s.scattering_matrix();
    auto dedx = s.get_dedx_calc().dedx();
    mfp_ = ArrayNDf(natoms, nmat, nerg);
    ipmax_ = ArrayNDf(natoms, nmat, nerg);
    fp_max_ = ArrayNDf(natoms, nmat, nerg);
    Tcutoff_ = ArrayNDf(natoms, nmat, nerg);
    umin_ = ArrayNDf(natoms, nmat, nerg);
    float delta_dedx = tr_opt_.max_rel_eloss;
    float Tmin = tr_opt_.min_recoil_energy;
    float mfp_ub = tr_opt_.max_mfp;
    float Tmin_rel = 0.99f; /// @TODO: make it user option
    for (int z1 = 0; z1 < natoms; z1++) {
        for (int im = 0; im < materials.size(); im++) {
            const material *m = materials[im];
            const float &N = m->atomicDensity();
            const float &Rat = m->atomicRadius();
            float mfp_lb = type_ == FullMC && tr_opt_.allow_sub_ml_scattering ? 0.f : Rat;
            for (dedx_index ie; ie != ie.end(); ie++) {
                float &mfp = mfp_(z1, im, ie);
                float &ipmax = ipmax_(z1, im, ie);
                float &fpmax = fp_max_(z1, im, ie);
                float &T0 = Tcutoff_(z1, im, ie);
                float &umin = umin_(z1, im, ie);
                // float & dedxn = dedxn_(z1,im,ie);
                float E = *ie;
                T0 = Tmin;

                // ensure Tmin is below Tm of all target atoms
                for (const atom *a : m->atoms()) {
                    int z2 = a->id();
                    float Tm = E * ScMatrix(z1, z2)->gamma();
                    if (T0 > Tmin_rel * Tm)
                        T0 = Tmin_rel * Tm;
                }

                // Calc mfp corresponding to T0, mfp = 1/(N*sig0), sig0 = pi*sum_i{ X_i *
                // [P_i(E,T0)]^2 }
                mfp = 0.f;
                for (const atom *a : m->atoms()) {
                    int z2 = a->id();
                    float d = ScMatrix(z1, z2)->impactPar(E, T0);
                    mfp += a->X() * d * d;
                }
                mfp = 1 / N / M_PI / mfp; // mfp = 1/(N*sig0) = 1/(N*pi*sum_i(ip_i^2))

                // ensure mfp*dEdx/E is below max_rel_eloss
                fpmax = delta_dedx * E / dedx(z1, im)->data()[ie];
                mfp = std::min(mfp, fpmax);

                // ensure mfp not smaller than lower bound
                mfp = std::max(mfp, mfp_lb);

                // ensure mfp not larger than upper bound
                mfp = std::min(mfp, mfp_ub);

                // Find the max impact parameter ipmax = (N*pi*mfp)^(-1/2)
                ipmax = std::sqrt(1.f / M_PI / mfp / N);

                // FullMC: u<umin reject collision
                umin = std::exp(-fpmax / mfp);

                // Calc dedxn for  T<T0 = N*sum_i { X_i * Sn(E,T0) }
                // Add this to dedx
                /// @TODO: this is very slow. dedxn is very small, can be ignored
                // We need to re-calc T0 from mfp
                /// @TODO: solve ipmax^2 = sum_i { X_i * ipmax_i(e,T0) } for T0
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

    switch (type_) {
    case AtomicSpacing:
        fp_ = m->atomicRadius();
        sqrtfp_ = 1.f;
        ip_ = ip0[mid];
        break;
    case Constant:
        fp_ = fp_const[mid];
        sqrtfp_ = sqrtfp_const[mid];
        ip_ = ip0[mid];
        break;
    case MendenhallWeller:
    case FullMC:
        ipmax_tbl = &(ipmax_(iid, mid, 0));
        mfp_tbl = &(mfp_(iid, mid, 0));
        fpmax_tbl = &(fp_max_(iid, mid, 0));
        umin_tbl = &(umin_(iid, mid, 0));
        fp_ = m->atomicRadius();
        sqrtfp_ = 1.f;
        ip_ = ip0[mid];
        break;
    default:
        assert(false); // should never reach here
    }

    return 0;
}
