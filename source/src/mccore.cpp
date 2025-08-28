#include "mccore.h"
#include "random_vars.h"
#include "event_stream.h"
#include "xs_corteo.h"
#include "cascade_queue.h"

mccore::mccore()
    : source_(new ion_beam),
      target_(new target),
      ref_count_(new int(0)),
      ion_counter_(new std::atomic_size_t(0)),
      thread_ion_counter_(0),
      abort_flag_(new std::atomic_bool()),
      tally_mutex_(new std::mutex),
      utally_(nullptr),
      dutally_(nullptr),
      ution_(nullptr)
{
}

mccore::mccore(const parameters &p, const transport_options &t)
    : par_(p),
      tr_opt_(t),
      source_(new ion_beam),
      target_(new target),
      ref_count_(new int(0)),
      ion_counter_(new std::atomic_size_t(0)),
      thread_ion_counter_(0),
      abort_flag_(new std::atomic_bool()),
      tally_mutex_(new std::mutex),
      utally_(nullptr),
      dutally_(nullptr),
      ution_(nullptr)
{
}

mccore::mccore(const mccore &s)
    : par_(s.par_),
      tr_opt_(s.tr_opt_),
      source_(s.source_),
      target_(s.target_),
      ref_count_(s.ref_count_),
      ion_counter_(s.ion_counter_),
      thread_ion_counter_(0),
      abort_flag_(s.abort_flag_),
      tally_mutex_(s.tally_mutex_),
      dedx_calc_(s.dedx_calc_),
      flight_path_calc_(s.flight_path_calc_),
      scattering_matrix_(s.scattering_matrix_),
      rng(s.rng),
      pka(s.pka),
      utally_(nullptr),
      dutally_(nullptr),
      ution_(nullptr)
{
    tally_.copy(s.tally_);
    dtally_.copy(s.dtally_);
    tion_.copy(s.tion_);
    tally_.clear();
    dtally_.clear();
    tion_.clear();

    if (s.utally_) {
        utally_ = new user_tally(*s.utally_);
        dutally_ = new user_tally(*s.utally_);
        ution_ = new user_tally(*s.utally_);
        utally_->clear();
        dutally_->clear();
        ution_->clear();
    }
}

mccore::~mccore()
{
    q_.clear();
    if (ref_count_.use_count() == 1) {
        delete source_;
        delete target_;
        if (!scattering_matrix_.isNull()) {
            abstract_xs_lab **xs = scattering_matrix_.data();
            for (int i = 0; i < scattering_matrix_.size(); i++)
                if (xs[i])
                    delete xs[i];
        }
    }
}

int mccore::init()
{
    /* the order of object initialization is important */
    target_->init();
    source_->init(*target_, par_.simulation_type == CascadesOnly);

    // init dedx & straggling tables
    dedx_calc_.init(*this);

    /*
     * create a scattering matrix for all ion compinations
     * # of combinations =
     * (all target atoms + projectile ) x (all target atoms)
     */
    auto materials = target_->materials();
    int nmat = materials.size();
    auto atoms = target_->atoms();
    int natoms = atoms.size();
    scattering_matrix_ = ArrayND<abstract_xs_lab *>(natoms, natoms);
    for (int z1 = 0; z1 < natoms; z1++) {
        for (int z2 = 1; z2 < natoms; z2++) {

            int Z1 = atoms[z1]->Z();
            float M1 = atoms[z1]->M();
            int Z2 = atoms[z2]->Z();
            float M2 = atoms[z2]->M();

            switch (par_.screening_type) {
            case Screening::ZBL:
                scattering_matrix_(z1, z2) = new xs_lab_zbl(Z1, M1, Z2, M2);
                break;
            case Screening::Bohr:
                scattering_matrix_(z1, z2) = new xs_lab_bohr(Z1, M1, Z2, M2);
                break;
            case Screening::KrC:
                scattering_matrix_(z1, z2) = new xs_lab_krc(Z1, M1, Z2, M2);
                break;
            case Screening::Moliere:
                scattering_matrix_(z1, z2) = new xs_lab_moliere(Z1, M1, Z2, M2);
                break;
            default:
                scattering_matrix_(z1, z2) = new xs_lab_zbl(Z1, M1, Z2, M2);
                break;
            }
        }
    }

    // init flight path selection tables
    flight_path_calc_.init(*this);

    /*
     * Allocate Tally Memory
     */
    int ncells = target_->grid().ncells();
    tally_.init(natoms, ncells);
    dtally_.init(natoms, ncells);
    tion_.init(natoms, ncells);

    /*
     * Init user tally(ies)
     */
    if (utally_) {
        utally_->init();
        dutally_->init();
        ution_->init();
    }

    // prepare event buffers
    std::vector<std::string> atom_labels = target_->atom_labels();
    atom_labels.erase(atom_labels.begin());
    pka.setNatoms(natoms - 1, atom_labels);

    return 0;
}

int mccore::reset()
{
    *ion_counter_ = 0;
    return 0;
}

int mccore::run()
{
    cascade_queue *cq = par_.intra_cascade_recombination
            ? new cascade_queue(par_.correlated_recombination)
            : nullptr;

    bool cascadesOnly = par_.simulation_type == CascadesOnly;

    while (!(*abort_flag_)) {
        // get the next ion id
        size_t ion_id = ion_counter_->fetch_add(1) + 1;
        if (ion_id > max_no_ions_) {
            (*ion_counter_)--;
            break;
        }

        // generate ion
        ion *i = q_.new_ion();
        i->setId(ion_id);
        i->setRecoilId(cascadesOnly ? 1 : 0);
        i->reset_counters();
        source_->source_ion(rng, *target_, *i);
        tion_(Event::NewSourceIon, *i);

        /*
         * If it is a cascades-only simulation put the ion in the pka queue
         * else transport it
         */
        if (cascadesOnly) {
            if (i->erg() >= i->myAtom()->Ed()) {
                i->setErg(i->erg() - i->myAtom()->El());
                if (par_.move_recoil) {
                    i->move(i->myAtom()->Rc());
                    dedx_calc_.preload(i, target_->cell(i->cellid()));
                    dedx_calc_(*i, i->myAtom()->Rc());
                }
                q_.push_pka(i);
            } else
                q_.free_ion(i);
        } else {
            transport(i);
            // free the ion buffer
            q_.free_ion(i);
        }

        // transport all PKAs
        pka.mark(tion_);
        while (ion *j = q_.pop_pka()) {
            // transport PKA
            pka.init(j);
            ion j1(*j); // keep a clone ion to have initial position

            // FullCascade or CascadesOnly
            if (par_.simulation_type != IonsOnly) {

                pka.cascade_start(*j);

                if (cq)
                    cq->init(j);

                transport(j, cq);

                // transport all secondary recoils
                while (ion *k = q_.pop_recoil()) {
                    transport(k, cq);
                    // free the ion buffer
                    q_.free_ion(k);
                }

                if (cq) {
                    cq->intra_cascade_recombination(target_->grid());
                    cq->tally_update(tion_);
                }

                pka.mark(tion_);
                pka.cascade_end(*j, cq);
            }

            // free the ion buffer
            q_.free_ion(j);

            // register NRT values (using j1 - at initial pos!)
            pka.cascade_complete(j1, tion_,
                                 par_.nrt_calculation == NRT_average ? target_->cell(j1.cellid())
                                                                     : nullptr);

            // send pka to the stream
            pka_stream_.write(&pka);

        } // end pka loop

        // compute total sums for current ion tally
        tion_.computeSums();

        // add this ion's tally to total score
        // lock the tally_mutex to allow merge operations
        {
            std::lock_guard<std::mutex> lock(*tally_mutex_);
            tally_ += tion_;
            dtally_.addSquared(tion_);
            if (utally_) {
                *utally_ += *ution_;
                dutally_->addSquared(*ution_);
            }
        }

        // clear the tally scores
        tion_.clear();
        if (utally_)
            ution_->clear();

        // update thread counter
        thread_ion_counter_++;

    } // ion loop

    if (cq)
        delete cq;

    return 0;
}

int mccore::transport(ion *i, cascade_queue *q)
{
    // collision flag
    bool doCollision;
    // flight path & impact parameter
    float fp, ip;

    // get the material at the ion's position
    const material *mat = target_->cell(i->cellid());

    // preload dEdx and fp tables for ion/material combination
    if (mat) {
        dedx_calc_.preload(i, mat);
        flight_path_calc_.preload(i, mat);
    }

    // transport loop
    while (1) {

        // Check if ion has enough energy to continue
        if (i->erg() < tr_opt_.min_energy) {
            /* projectile has to stop. Store as implanted/interstitial atom*/
            ionEvent(Event::IonStop, *i);
            if (q) // q->push(Event::IonStop,*i);
                q->push_i(i);
            return 0; // history ends
        }

        if (!mat) { // Vacuum.
            // set flight path to ~inf
            // to intentionally hit a boundary
            // Then the boundary crossing algorithm
            // will take care of things
            fp = 1e30f;
            BoundaryCrossing crossing = i->propagate(fp);
            switch (crossing) {
            case BoundaryCrossing::None:
            case BoundaryCrossing::InternalPBC:
                break;
            case BoundaryCrossing::Internal:
                // register event
                ionEvent(Event::BoundaryCrossing, *i);
                i->reset_counters();
                // get new material and dEdx, mfp tables
                mat = target_->cell(i->cellid());
                if (mat) {
                    dedx_calc_.preload(i, mat);
                    flight_path_calc_.preload(i, mat);
                }
                break;
            case BoundaryCrossing::External:
                // register ion exit event
                ionEvent(Event::IonExit, *i);
                if (exit_stream_.is_open()) {
                    exit_ev.set(i);
                    exit_stream_.write(&exit_ev);
                }
                return 0; // ion history ends!
            }
            continue; // go to next iter
        }

        // select flight path & impact param.
        doCollision = flight_path_calc_(rng, i->erg(), fp, ip);

        // propagate ion, checking also for boundary crossing
        BoundaryCrossing crossing = i->propagate(fp);

        // subtract ionization & straggling
        dedx_calc_(*i, fp, rng);

        // handle boundary
        switch (crossing) {
        case BoundaryCrossing::Internal:
            // register event
            ionEvent(Event::BoundaryCrossing, *i);
            i->reset_counters();
            // get new material and dEdx, mfp tables
            mat = target_->cell(i->cellid());
            if (mat) {
                dedx_calc_.preload(i, mat);
                flight_path_calc_.preload(i, mat);
            }
            doCollision = false; // the collision will be in the new material
            break;
        case BoundaryCrossing::InternalPBC:
            // InternalPBC = particle crossed periodic boundary without
            // changing cell !!
            doCollision = false;
            break;
        case BoundaryCrossing::External:
            // register ion exit event
            ionEvent(Event::IonExit, *i);
            if (exit_stream_.is_open()) {
                exit_ev.set(i);
                exit_stream_.write(&exit_ev);
            }
            return 0; // ion history ends!
        default:
            break;
        }

        // if no collision, start again
        // no collision =
        // either BoundaryCrossing::Internal OR
        // rejected by flight path algorithm
        if (!doCollision)
            continue;

        // Now comes the COLLISSION PART

        // select collision partner
        const atom *z2 = mat->selectAtom(rng);

        // get the cross-section and calculate scattering
        auto xs = scattering_matrix_(i->myAtom()->id(), z2->id());
        float T; // recoil energy
        float sintheta, costheta; // Lab sys scattering angle sin & cos
        xs->scatter(i->erg(), ip, T, sintheta, costheta);
        assert(finite(T));

        // get random azimuthal dir
        float nx, ny; // nx = cos(phi), ny = sin(phi), phi: az. angle

#if SAMPLE_P_AND_N == 1

        nx = flight_path_calc_.nx();
        ny = flight_path_calc_.ny();

#else

        rng.random_azimuth_dir(nx, ny);

#endif

        // register scattering event (before changing ion data)
        // t(Event::Scattering,*i);

        // apply new ion direction & energy
        vector3 dir0 = i->dir(); // store initial dir
        i->deflect(vector3(nx * sintheta, ny * sintheta, costheta));
        i->add_coll();

        // Check if recoil is displaced (T>=E_d)
        if (T >= z2->Ed()) {

            // subtract recoil energy from the ion's kinetic energy
            i->de_recoil(T);

            // calc recoil dir from momentum conservation
            float b = i->erg() / T;
            vector3 nt = dir0 - i->dir() * std::sqrt(b / (1.f + b)); // un-normalized
            nt.normalize();

            // TODO
            // If T-Ed < cutoff then the recoil will die immediately
            // we could just add the interstitial here

            // create recoil (it is stored in the ion queue)
            ion *j = new_recoil(i, z2, T, nt);
            ion j1(*j); // keep a copy

            // move recoil to the edge of recomb. area
            // checking also for boundary crossing
            if (par_.move_recoil) {
                j->move(z2->Rc());
                dedx_calc_(*j, z2->Rc());
                if (par_.recoil_sub_ed) {
                    double de = j->erg() + z2->Ed() - T;
                    j->de_phonon(de);
                }
            }

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
                ionEvent(Event::Replacement, *i, z2);
                j->setUid(i->uid());
                // if (q) q->push(Event::Replacement,*i);
                return 0; // end of ion history
            }

            if (q)
                q->push_v(&j1);

        } else { // T<E_d, recoil cannot be displaced
            // energy goes to phonons
            i->de_phonon(T);
        }

    } // main ion transport loop

    return 0;
}

void mccore::mergeTallies(mccore &other)
{
    std::lock_guard<std::mutex> lock(*tally_mutex_);
    tally_ += other.tally_;
    other.tally_.clear();
    dtally_ += other.dtally_;
    other.dtally_.clear();
    if (utally_) {
        *utally_ += *(other.utally_);
        other.utally_->clear();
        *dutally_ += *(other.dutally_);
        other.dutally_->clear();
    }
}

void mccore::mergeEvents(mccore &other)
{
    pka_stream_.merge(other.pka_stream_);
    other.pka_stream_.clear();
    exit_stream_.merge(other.exit_stream_);
    other.exit_stream_.clear();
}

ArrayNDd mccore::getTallyTable(int i) const
{
    if (i < 0 || i >= tally::tEnd)
        return ArrayNDd();

    ArrayNDd A;
    {
        std::lock_guard<std::mutex> lock(*tally_mutex_);
        A = tally_.at(i).copy();
    }

    return A;
}
ArrayNDd mccore::getTallyTableVar(int i) const
{
    if (i < 0 || i >= tally::tEnd)
        return ArrayNDd();

    ArrayNDd A;
    {
        std::lock_guard<std::mutex> lock(*tally_mutex_);
        A = dtally_.at(i).copy();
    }

    return A;
}
void mccore::copyTallyTable(int i, ArrayNDd &A) const
{
    if (i < 0 || i >= tally::tEnd)
        return;
    std::lock_guard<std::mutex> lock(*tally_mutex_);
    tally_.at(i).copyTo(A);
}
void mccore::copyTallyTableVar(int i, ArrayNDd &dA) const
{
    if (i < 0 || i >= tally::tEnd)
        return;
    std::lock_guard<std::mutex> lock(*tally_mutex_);
    dtally_.at(i).copyTo(dA);
}

void mccore::addUserTally(const user_tally::parameters &p)
{
    utally_ = new user_tally(p);
    dutally_ = new user_tally(p);
    ution_ = new user_tally(p);
}
