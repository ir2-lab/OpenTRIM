#include "mccore.h"
#include "random_vars.h"
#include "dedx.h"
#include "event_stream.h"
#include "corteo_xs.h"
#include <iostream>
#include <type_traits>

mccore::mccore() :
    source_(new ion_beam),
    target_(new target),
    ref_count_(new int(0)),
    ion_counter_(new std::atomic_size_t(0)),
    thread_ion_counter_(0),
    abort_flag_(new std::atomic_bool()),
    tally_mutex_(new std::mutex)
{
}

mccore::mccore(const parameters &p, const transport_options& t) :
    par_(p), tr_opt_(t),
    source_(new ion_beam),
    target_(new target),
    ref_count_(new int(0)),
    ion_counter_(new std::atomic_size_t(0)),
    thread_ion_counter_(0),
    abort_flag_(new std::atomic_bool()),
    tally_mutex_(new std::mutex)
{
}

mccore::mccore(const mccore &s) :
    par_(s.par_), tr_opt_(s.tr_opt_),
    source_(s.source_),
    target_(s.target_),
    ref_count_(s.ref_count_),
    ion_counter_(s.ion_counter_),
    thread_ion_counter_(0),
    abort_flag_(s.abort_flag_),
    tally_mutex_(s.tally_mutex_),
    sqrtfp_const(s.sqrtfp_const), ip0(s.ip0),
    dedx_(s.dedx_),
    de_strag_(s.de_strag_),
    scattering_matrix_(s.scattering_matrix_),
    mfp_(s.mfp_), ipmax_(s.ipmax_),
    fp_max_(s.fp_max_), Tcutoff_(s.Tcutoff_),
    rng(),
    pka(s.pka)
{
    tally_.copy(s.tally_);
    dtally_.copy(s.dtally_);
    tion_.copy(s.tion_);
    tally_.clear();
    dtally_.clear();
    tion_.clear();
}

mccore::~mccore()
{
    q_.clear();
    if (ref_count_.use_count() == 1) {
        delete source_;
        delete target_;
        if (!dedx_.isNull()) {
            dedx_interp** p = dedx_.data();
            for (int i=0; i<dedx_.size(); i++) delete p[i];
        }
        if (!de_strag_.isNull()) {
            straggling_interp** p = de_strag_.data();
            for (int i=0; i<de_strag_.size(); i++) delete p[i];
        }
        if (!scattering_matrix_.isNull()) {
            abstract_xs_lab** xs = scattering_matrix_.data();
            for (int i=0; i<scattering_matrix_.size(); i++) delete xs[i];
        }
    }
}

int mccore::init() {

    target_->init();

    // adjust projectile codes in target & source
    target_->setProjectile(source_->ionZ(), source_->ionM());
    source_->setProjectile(target_->projectile());

    // calculate sqrt{l/l0} in each material for constant flight path
    auto materials = target_->materials();
    int nmat = materials.size();
    sqrtfp_const.resize(nmat);
    ip0.resize(nmat);
    for(int i=0; i<nmat; i++) {
        sqrtfp_const[i] = std::sqrt(tr_opt_.flight_path_const / materials[i]->atomicRadius());
        ip0[i] = materials[i]->meanImpactPar();
        if (tr_opt_.flight_path_type == Constant) ip0[i] /= sqrtfp_const[i];
    }

    /*
     * create dedx and straggling tables for all ion - material
     * combinations, # =
     * (all target atoms + projectile ) x (all target materials)
     * For each combi, get an interpolator object
     */
    auto atoms = target_->atoms();
    int natoms = atoms.size();    
    dedx_ = ArrayND<dedx_interp *>(natoms, nmat);
    de_strag_ = ArrayND<straggling_interp *>(natoms, nmat);
    for (atom *at1 : atoms)
    {
        int iat1 = at1->id();
        for (const material *mat : materials)
        {
            int im = mat->id();
            auto desc = mat->getDescription();
            dedx_(iat1, im) = new dedx_interp(at1->Z(), at1->M(),
                                              desc.Z, desc.X, mat->atomicDensity());
            de_strag_(iat1, im) = new straggling_interp(par_.straggling_model,
                                                        at1->Z(), at1->M(),
                                                        desc.Z, desc.X, mat->atomicDensity());
        }
    }

    /*
     * create a scattering matrix for all ion compinations
     * # of combinations =
     * (all target atoms + projectile ) x (all target atoms)
     */
    scattering_matrix_ = ArrayND<abstract_xs_lab*>(natoms, natoms);
    for(int z1 = 0; z1<natoms; z1++)
    {
        for(int z2 = 1; z2<natoms; z2++)
        {
            switch (par_.scattering_calculation) {
            case Corteo4bitTable:
                switch (par_.screening_type) {
                case Screening::ZBL:
                    scattering_matrix_(z1,z2) = new xs_lab_zbl;
                    break;
                case Screening::LenzJensen:
                    scattering_matrix_(z1,z2) = new xs_lab_lj;
                    break;
                case Screening::KrC:
                    scattering_matrix_(z1,z2) = new xs_lab_krc;
                    break;
                case Screening::Moliere:
                    scattering_matrix_(z1,z2) = new xs_lab_moliere;
                    break;
                default:
                    scattering_matrix_(z1,z2) = new xs_lab_zbl;
                    break;
                }
                break;
            case ZBL_MAGICK:
                scattering_matrix_(z1,z2) = new xs_lab_zbl_magic;
                break;
            default:
                scattering_matrix_(z1,z2) = new xs_lab_zbl;
                break;
            }

            scattering_matrix_(z1,z2)->init(atoms[z1]->Z(), atoms[z1]->M(),
                                            atoms[z2]->Z(), atoms[z2]->M());
        }
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
    int nerg = dedx_index::size;
    mfp_   = ArrayNDf(natoms,nmat,nerg);
    ipmax_ = ArrayNDf(natoms,nmat,nerg);
    fp_max_ = ArrayNDf(natoms,nmat,nerg);
    Tcutoff_ = ArrayNDf(natoms,nmat,nerg);
    float delta_dedx = tr_opt_.max_rel_eloss;
    float Tmin = tr_opt_.min_recoil_energy;
    float mfp_ub = tr_opt_.max_mfp;
    float Tmin_rel = 0.99f; /// @TODO: make it user option
    for(int z1 = 0; z1<natoms; z1++)
    {
        for(int im=0; im<materials.size(); im++)
        {
            const material* m = materials[im];
            const float & N = m->atomicDensity();
            const float & Rat = m->atomicRadius();
            float mfp_lb = tr_opt_.flight_path_type == IPP && tr_opt_.allow_sub_ml_scattering ?
                               0.f : Rat;
            for(dedx_index ie; ie!=ie.end(); ie++) {
                float & mfp = mfp_(z1,im,ie);
                float & ipmax = ipmax_(z1,im,ie);
                float & fpmax = fp_max_(z1,im,ie);
                float & T0 = Tcutoff_(z1,im,ie);
                // float & dedxn = dedxn_(z1,im,ie);
                float E = *ie;
                T0 = Tmin;

                // ensure Tmin is below Tm of all target atoms
                for(const atom* a : m->atoms()) {
                    int z2 = a->id();
                    float Tm = E*scattering_matrix_(z1,z2)->gamma();
                    if (T0 > Tmin_rel*Tm) T0 = Tmin_rel*Tm;
                }

                // Calc mfp corresponding to T0, mfp = 1/(N*sig0), sig0 = pi*sum_i{ X_i * [P_i(E,T0)]^2 }
                mfp = 0.f;
                for(const atom* a : m->atoms()) {
                    int z2 = a->id();
                    float d = scattering_matrix_(z1,z2)->impactPar(E, T0);
                    mfp += a->X()*d*d;
                }
                mfp = 1/N/M_PI/mfp; // mfp = 1/(N*sig0) = 1/(N*pi*sum_i(ip_i^2))

                // ensure mfp*dEdx/E is below max_rel_eloss
                fpmax = delta_dedx*E / dedx_(z1,im)->data()[ie];
                mfp = std::min(mfp,fpmax);

                // ensure mfp not smaller than lower bound
                mfp = std::max(mfp,mfp_lb);

                // ensure mfp not larger than upper bound
                mfp = std::min(mfp,mfp_ub);

                // Find the max impact parameter ipmax = (N*pi*mfp)^(-1/2)
                ipmax = std::sqrt(1.f/M_PI/mfp/N);

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
//                    dedxn += scattering_matrix_(z1,z2)->stoppingPower(E,T0) * a->X();
//                }
//                dedxn *= N;
//                if (tr_opt_.flight_path_type == MyFFP) dedx_(z1,im,ie) += dedxn;

            } // energy
        } // material
    } // Z1

    /*
     * Allocate Tally Memory
     */
    int ncells = target_->grid().ncells();
    tally_.init(natoms, ncells);
    dtally_.init(natoms, ncells);
    tion_.init(natoms, ncells);

    // prepare event buffers
    std::vector<std::string> atom_labels = target_->atom_labels();
    atom_labels.erase(atom_labels.begin());
    pka.setNatoms(natoms-1,atom_labels);

    return 0;
}

int mccore::reset()
{
    *ion_counter_ = 0;
    return 0;
}

ion* mccore::new_recoil(const ion* proj, const atom *target, const float& recoil_erg,
                            const vector3& dir0, const float &sqrt_mass_ratio)
{
    // clone the projectile ion
    // this gets the correct position and cell id
    ion* j = q_.new_ion(proj);
    // init the recoil with target atomic species and
    // recoil kinetic energy
    // subtract the lattice energy (FP creation energy)
    j->init_recoil(target, recoil_erg - target->El());
    j->reset_counters();

    // calc recoil direction from momentum conserv.
    float f1, f2;
    f1 = proj->erg() / recoil_erg;
    f2 = std::sqrt(f1);
    f1 = std::sqrt(f1+1);
    j->setDir((f1*dir0 - f2*(proj->dir()))*sqrt_mass_ratio);

    // store recoil in respective queue
    if (j->recoil_id()==1) q_.push_pka(j);
    else q_.push_recoil(j);

    return j;
}

int mccore::run()
{
    while( !(*abort_flag_) )
    {
        // get the next ion id
        size_t ion_id = ion_counter_->fetch_add(1) + 1;
        if (ion_id > max_no_ions_) {
            (*ion_counter_)--;
            break;
        }

        // generate ion
        ion* i = q_.new_ion();
        i->setId(ion_id);
        i->resetRecoilId();
        i->reset_counters();
        source_->source_ion(rng, *target_, *i);
        tion_(Event::NewSourceIon,*i);

        // transport the ion
        transport(i,tion_);

        // transport all PKAs
        ion* j;
        while ((j = q_.pop_pka()) != nullptr) {
            // transport PKA
            //pka.init(j);
            ion j1(*j); // keep a clone ion to have initial position
            pka_mark(&j1,tion_,pka,true);

            // FullCascade or CascadesOnly
            if (par_.simulation_type != IonsOnly) {
                transport(j,tion_);
                // transport all secondary recoils
                ion* k;
                while ((k = q_.pop_recoil())!=nullptr) {
                    transport(k,tion_);
                    // free the ion buffer
                    q_.free_ion(k);
                }
            }
            // free the ion buffer
            q_.free_ion(j);
            pka_mark(&j1,tion_,pka,false);
            pka_stream_.write(&pka);
        } // pka loop

        // compute total sums and
        // add this ion's tally to total score
        tion_.computeSums();

        {
            std::lock_guard< std::mutex > lock(*tally_mutex_);
            //assert(tion_.debugCheck(i->erg0()));
            tally_ += tion_;
            dtally_.addSquared(tion_);
        }

        tion_.clear();

        // free the ion buffer
        q_.free_ion(i);

        // update thread counter
        thread_ion_counter_++;

    } // ion loop

    pka_stream_.close();
    exit_stream_.close();

    return 0;
}

float mccore::calcDedx(float E, float fp, float sqrtfp,
                       const dedx_interp *stopping,
                       const straggling_interp *straggling)
{


    float de_stopping = fp * (*stopping)(E);
    if (par_.eloss_calculation == EnergyLossAndStraggling)
    {
        dedx_index ie(E);
        float de_straggling = straggling->data()[ie] * rng.normal() * sqrtfp;

        /* IRADINA
         * Due to gaussian distribution, the straggling can in some cases
         * get so large that the projectile gains energy or suddenly looses
         * a huge amount of energy. Is this realistic? This might actually
         * happen. However, in the simulation, ions may have higher energy
         * than the initial energy.
         * We will therefore limit the |straggling| to |stopping|.
         * Furthermore, with hydrogen, the straggling is often so big,
         * that ions gain huge amount of energy, and the phononic system
         * would actually gain energy.
         */
        if (std::abs(de_straggling) > de_stopping)
            de_straggling = (de_straggling < 0) ? -de_stopping : de_stopping;

        de_stopping += de_straggling;
    }

    /* IRADINA
     * The stopping tables have no values below minVal = 16 eV.
     * Therefore, we do sqrt downscaling of electronic
     * stopping below 16 eV.
     */
    if (E < dedx_index::minVal)
        de_stopping *= std::sqrt(E / dedx_index::minVal);

    /*
     * This is due to some rare low-energy events (ion E<100 eV)
     * with IPP flight selection were
     * a long flight path + large straggling can cause the
     * stopping + straggling > E
     *
     * The code below changes that to almost stopping the ion,
     * E - stopping ~ 0
     */
    if (de_stopping > E)
        de_stopping = 0.99*E;

    return de_stopping;
}

int mccore::transport(ion* i, tally &t)
{
    // get the material at the ion's position
    const material* mat = target_->cell(i->cellid());
    // get the corresponding dEdx & straggling table for ion/material combination
    const dedx_interp* de_stopping_tbl = nullptr;
    const straggling_interp* de_straggling_tbl = nullptr;
    std::array<const float *, 3> fp_par_tbl;
    const float* & ipmax_tbl = fp_par_tbl[0];
    const float* & mfp_tbl   = fp_par_tbl[1];
    const float* & fpmax_tbl = fp_par_tbl[2];
    float fp, ip, sqrtfp, ipmax;
    int ie;
    bool doCollision;
    if (mat) {
        getDEtables(i->myAtom(), mat, de_stopping_tbl, de_straggling_tbl);
        getMFPtables(i->myAtom(), mat, fp, sqrtfp, fp_par_tbl);
    }

    // transport loop
    while (1) {

        // Check if ion has enough energy to continue
        if(i->erg() < tr_opt_.min_energy) { 
            /* projectile has to stop. Store as implanted/interstitial atom*/
            t(Event::IonStop,*i);
            return 0; // history ends
        } 

        if (!mat) {  // Vacuum.
            // set a long flight path (1mm)
            // to intentionally hit a boundary
            // Then the boundary crossing algorithm
            // will take care of things
            fp = 1e6f;
            BoundaryCrossing crossing = i->propagate(fp);
            switch(crossing) {
            case BoundaryCrossing::None:
                break;
            case BoundaryCrossing::Internal:
                // register event
                t(Event::BoundaryCrossing,*i);
                i->reset_counters();
                // get new material and dEdx, mfp tables
                mat = target_->cell(i->cellid());
                if (mat) {
                    getDEtables(i->myAtom(), mat, de_stopping_tbl, de_straggling_tbl);
                    getMFPtables(i->myAtom(), mat, fp, sqrtfp, fp_par_tbl);
                }
                break;
            case BoundaryCrossing::External:
                // register ion exit event
                t(Event::IonExit,*i);
                if (exit_stream_.is_open()) {
                    exit_ev.set(i);
                    exit_stream_.write(&exit_ev);
                }
                return 0; // ion history ends!
            }
            continue; // go to next iter
        }

        // select flight path & impact param.
        doCollision = flightPath(i,mat,fp,ip,sqrtfp,fp_par_tbl);

        // propagate ion, checking also boundary crossing
        BoundaryCrossing crossing = i->propagate(fp);

        // subtract ionization & straggling
        if (par_.eloss_calculation != EnergyLossOff) {
            float de = calcDedx(i->erg(), fp, sqrtfp, de_stopping_tbl, de_straggling_tbl);
            i->de_ioniz(de);
        }

        // handle boundary
        switch(crossing) {
        case BoundaryCrossing::Internal:
            // register event
            t(Event::BoundaryCrossing,*i);
            i->reset_counters();
            // get new material and dEdx, mfp tables
            mat = target_->cell(i->cellid());
            if (mat) {
                getDEtables(i->myAtom(), mat, de_stopping_tbl, de_straggling_tbl);
                getMFPtables(i->myAtom(), mat, fp, sqrtfp, fp_par_tbl);
            }
            doCollision = false; // the collision will be in the new material
            break;
        case BoundaryCrossing::External:
            // register ion exit event
            t(Event::IonExit,*i);
            if (exit_stream_.is_open()) {
                exit_ev.set(i);
                exit_stream_.write(&exit_ev);
            }
            return 0; // ion history ends!
        case BoundaryCrossing::InternalPBC:
            doCollision = false;
            break;
        default:
            break;
        }

        // if no collision, start again
        // no collision =
        // either BoundaryCrossing::Internal OR
        // rejected by flight path algorithm
        if (!doCollision) continue;

        // Now comes the COLLISSION PART

        // select collision partner
        const atom* z2 = mat->selectAtom(rng);

        // get the cross-section and calculate scattering
        auto xs = scattering_matrix_(i->myAtom()->id(),z2->id());
        float T; // recoil energy
        float sintheta, costheta; // Lab sys scattering angle sin & cos 
        xs->scatter(i->erg(), ip, T, sintheta, costheta);
        assert(finite(T));

        // get random azimuthal dir
        float nx, ny; // nx = cos(phi), ny = sin(phi), phi: az. angle
        rng.random_azimuth_dir(nx,ny);

        // register scattering event (before changing ion data)
        // t(Event::Scattering,*i);

        // apply new ion direction & energy
        vector3 dir0 = i->dir(); // store initial dir
        i->deflect(vector3(nx*sintheta, ny*sintheta, costheta));
        i->add_coll();

        /// @TODO: special treatment of surface effects (sputtering etc.)

        // Check if recoil is displaced (T>=E_d)
        if (T >= z2->Ed()) { 

            // Create recoil (it is stored in the ion queue)
            i->de_recoil(T);
            new_recoil(i,z2,T,dir0,xs->sqrt_mass_ratio());

            /*
             * Now check whether the projectile might replace the recoil
             *
             * Check if Z1==Z2 and E < Er
             *
             * This is different from Iradina, where it is required
             * Z1==Z2 && M1==M2
             */
            if ((i->myAtom()->Z() == z2->Z()) && (i->erg() < z2->Er())) {
                // Replacement event, ion energy goes to Phonons
                t(Event::Replacement,*i,z2);
                return 0; // end of ion history
            }

            /*
             * Otherwise:
             * Z2 vacancy is created and ion continues
             */
            //t(Event::Vacancy,*j);

        } else { // T<E_d, recoil cannot be displaced
            // energy goes to phonons
            // t(Event::Phonon,*i,&T);
            i->de_phonon(T);
        }

    } // main ion transport loop

    return 0;
}

bool mccore::flightPath(const ion* i, const material* mat, float& fp, float& ip, float& sqrtfp,
                        std::array<const float *, 3> &fp_par_tbl)
{
    bool doCollision = true;
    int ie;
    const float* & ipmax_tbl = fp_par_tbl[0];
    const float* & mfp_tbl   = fp_par_tbl[1];
    const float* & fpmax_tbl = fp_par_tbl[2];
    switch (tr_opt_.flight_path_type) {
    case AtomicSpacing:
    case Constant:
        ip = ipmax_tbl[0]*std::sqrt(rng.u01d_lopen());
        break;
    case MendenhallWeller:
        ie = dedx_index(float(i->erg()));
        ip = ipmax_tbl[ie];
        fp = mfp_tbl[ie];
        if (ip < mat->meanImpactPar())
        {
            sqrtfp = std::sqrt(fp/mat->atomicRadius());
            ip *= std::sqrt(-std::log(rng.u01s_open()));
            doCollision = (ip <= ipmax_tbl[ie]);
        } else { // atomic spacing
            fp = mat->atomicRadius();
            sqrtfp = 1.f;
            ip = mat->meanImpactPar()*std::sqrt(rng.u01d_lopen());
        }
        break;
    case IPP:
        ie = dedx_index(float(i->erg()));
        fp = mfp_tbl[ie]*(-std::log(rng.u01s_open()));
        doCollision = fp <= fpmax_tbl[ie];
        if (doCollision) ip = ipmax_tbl[ie]*std::sqrt(rng.u01d_lopen());
        else fp = fpmax_tbl[ie];
        sqrtfp = std::sqrt(fp/mat->atomicRadius());
        break;
    default:
        assert(false); // never get here
    }
    assert(fp>0);
    assert(finite(fp));
    return doCollision;
}

void mccore::pka_mark(const ion* i, tally &t, pka_event &pka, bool start)
{
    double sT(0.);
    int ncell = target_->grid().ncells();
    int natoms = target_->atoms().size();
    std::vector<double> sV(natoms,0.), sI(natoms,0.), sR(natoms,0);
    for(int i=1; i<natoms; i++) {
        const double * ps = &t.stored()(i,0);
        const double * pl = &t.lattice()(i,0);
        const double * pv = &t.vacancies()(i,0);
        const double * pr = &t.replacements()(i,0);
        const double * pi = &t.implantations()(i,0);
        for(int j=0; j<ncell; j++) {
            sT += *ps++ + *pl++;
            sV[i] += *pv++;
            sI[i] += *pi++;
            sR[i] += *pr++;
        }
    }

    if (start) {
        pka.init(i);
        pka.Tdam() = sT;
        for(int i=1; i<natoms; i++) {
            pka.Vac(i-1) = sV[i];
            pka.Repl(i-1)= sR[i];
            pka.Impl(i-1)= sI[i];
        }
    } else {
        pka.Tdam() = sT - pka.Tdam() + i->myAtom()->El();
        for(int i=1; i<natoms; i++) {
            pka.Vac(i-1) = sV[i] - pka.Vac(i-1);
            pka.Repl(i-1)= sR[i] - pka.Repl(i-1);
            pka.Impl(i-1)= sI[i] - pka.Impl(i-1);
        }

        const atom* z2;
        const material* m;
        std::array<float, 5> dp;
        dp.fill(0.f);
        dp[0] = pka.recoilE();
        switch (par_.nrt_calculation) {
        case NRT_element:
            // NRT/LSS damage based on element
            z2 = i->myAtom();
            dp[1] = z2->LSS_Tdam(pka.recoilE());
            dp[2] = z2->NRT(dp[1]);
            // NRT damage based on element with true Tdam
            dp[3] = pka.Tdam();
            dp[4] = z2->NRT(dp[3]);

            break;
        case NRT_average:
            m = target_->cell(i->cellid());
            // NRT/LSS damage based on material / average Ed, Z, M
            dp[1] = m->LSS_Tdam(pka.recoilE());
            dp[2] = m->NRT(dp[1]);
            // NRT damage based on material with true Tdam
            dp[3] = pka.Tdam();
            dp[4] = m->NRT(dp[3]);
            break;
        default:
            break;
        }
        t(Event::CascadeComplete,*i,dp.data());
    }
}

void mccore::merge(const mccore& other)
{
    tally_ += other.tally_;
    dtally_ += other.dtally_;
    pka_stream_.merge(other.pka_stream_);
    exit_stream_.merge(other.exit_stream_);
}

void mccore::remove_stream_files()
{
    pka_stream_.remove();
    exit_stream_.remove();
}

ArrayNDd mccore::getTallyTable(int i) const
{
    if (i<0 || i>=tally::tEnd) return ArrayNDd();
    std::lock_guard< std::mutex > lock(*tally_mutex_);
    return tally_.at(i).copy();
}
ArrayNDd mccore::getTallyTableVar(int i) const
{
    if (i<0 || i>=tally::tEnd) return ArrayNDd();
    std::lock_guard< std::mutex > lock(*tally_mutex_);
    return dtally_.at(i).copy();
}
void mccore::copyTallyTable(int i, ArrayNDd& A) const
{
    if (i<0 || i>=tally::tEnd) return;
    std::lock_guard< std::mutex > lock(*tally_mutex_);
    tally_.at(i).copyTo(A);
}
void mccore::copyTallyTableVar(int i, ArrayNDd& dA) const
{
    if (i<0 || i>=tally::tEnd) return;
    std::lock_guard< std::mutex > lock(*tally_mutex_);
    dtally_.at(i).copyTo(dA);
}




