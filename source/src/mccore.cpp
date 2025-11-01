#include "mccore.h"
#include "random_vars.h"
#include "event_stream.h"
#include "scattering.h"
#include "cascade.h"

mccore::mccore()
    : source_(new ion_beam),
      target_(new target),
      ref_count_(new int(0)),
      ion_counter_(new std::atomic_size_t(0)),
      thread_ion_counter_(0),
      abort_flag_(new std::atomic_bool()),
      tally_mutex_(new std::mutex)
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
      tally_mutex_(new std::mutex)
{
}

mccore::mccore(const mccore &s)
    : par_(s.par_),
      tr_opt_(s.tr_opt_),
      source_(s.source_),
      target_(s.target_),
      tally_(s.tally_),
      dtally_(s.dtally_),
      tion_(s.tion_),
      ref_count_(s.ref_count_),
      ion_counter_(s.ion_counter_),
      thread_ion_counter_(0),
      abort_flag_(s.abort_flag_),
      tally_mutex_(s.tally_mutex_),
      dedx_calc_(s.dedx_calc_),
      flight_path_calc_(s.flight_path_calc_),
      scattering_matrix_(s.scattering_matrix_),
      rng(s.rng),
      pka(s.pka)
{
    tally_.clear();
    dtally_.clear();
    tion_.clear();

    if (s.utally_.size()) {
        for (int i = 0; i < s.utally_.size(); ++i) {
            auto u = s.utally_[i];
            utally_.push_back(new user_tally(*u));
            dutally_.push_back(new user_tally(*u));
            ution_.push_back(new user_tally(*u));
            utally_.back()->clear();
            dutally_.back()->clear();
            ution_.back()->clear();
        }
        utallyMask_ = s.utallyMask_;
    }
}

mccore::~mccore()
{
    ion_queue_.clear();

    if (!utally_.empty()) {
        for (int i = 0; i < (int)utally_.size(); ++i) {
            delete utally_[i];
            delete dutally_[i];
            delete ution_[i];
        }
    }

    if (ref_count_.use_count() == 1) {
        delete source_;
        delete target_;
        if (!scattering_matrix_.isNull()) {
            abstract_scattering_calc **xs = scattering_matrix_.data();
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
    scattering_matrix_ = ArrayND<abstract_scattering_calc *>(natoms, natoms);
    for (int z1 = 0; z1 < natoms; z1++) {
        for (int z2 = 1; z2 < natoms; z2++) {

            int Z1 = atoms[z1]->Z();
            float M1 = atoms[z1]->M();
            int Z2 = atoms[z2]->Z();
            float M2 = atoms[z2]->M();

            switch (par_.screening_type) {
            case Screening::ZBL:
                scattering_matrix_(z1, z2) = new zbl_scattering_calc(Z1, M1, Z2, M2);
                break;
            case Screening::Bohr:
                scattering_matrix_(z1, z2) = new bohr_scattering_calc(Z1, M1, Z2, M2);
                break;
            case Screening::KrC:
                scattering_matrix_(z1, z2) = new krc_scattering_calc(Z1, M1, Z2, M2);
                break;
            case Screening::Moliere:
                scattering_matrix_(z1, z2) = new moliere_scattering_calc(Z1, M1, Z2, M2);
                break;
            case Screening::None:
                scattering_matrix_(z1, z2) = new unscreened_scattering_calc(Z1, M1, Z2, M2);
                break;
            default:
                scattering_matrix_(z1, z2) = new zbl_scattering_calc(Z1, M1, Z2, M2);
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
    auto dim = target_->grid().dim();
    tally_.init(natoms, dim[0], dim[1], dim[2]);
    dtally_.init(natoms, dim[0], dim[1], dim[2]);
    tion_.init(natoms, dim[0], dim[1], dim[2]);

    /*
     * Init user tally(ies)
     */
    if (utally_.size()) {
        for (int i = 0; i < utally_.size(); ++i) {
            utally_[i]->init(natoms);
            dutally_[i]->init(natoms);
            ution_[i]->init(natoms);
            utallyMask_ |= static_cast<uint32_t>(ution_[i]->event());
        }
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
    abstract_cascade *cscd = nullptr;
    if (par_.intra_cascade_recombination) {
        cscd = par_.time_ordered_cascades
                ? (abstract_cascade *)(new time_ordered_cascade(target_->grid()))
                : (abstract_cascade *)(new unordered_cascade(target_->grid()));
    }

    bool cascadesOnly = par_.simulation_type == CascadesOnly;

    while (!(*abort_flag_)) {
        // get the next ion id
        size_t ion_id = ion_counter_->fetch_add(1) + 1;
        if (ion_id > max_no_ions_) {
            (*ion_counter_)--;
            break;
        }

        // generate ion
        ion *i = ion_queue_.new_ion();
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
                double T = i->erg();
                i->setErg(i->erg() - i->myAtom()->El());
                if (par_.move_recoil) {
                    i->move(i->myAtom()->Rc());
                    dedx_calc_.preload(i, target_->cell(i->cellid()));
                    dedx_calc_(*i, i->myAtom()->Rc());
                }
                if (par_.recoil_sub_ed) {
                    double de = i->erg() + i->myAtom()->Ed() - T;
                    i->de_phonon(de);
                }
                ion_queue_.push_pka(i);
            } else
                ion_queue_.free_ion(i);
        } else {
            transport(i);
            // free the ion buffer
            ion_queue_.free_ion(i);
        }

        // transport all PKAs
        pka.mark(tion_);
        while (ion *j = ion_queue_.pop_pka()) {
            // transport PKA
            pka.init(j);
            ion j1(*j); // keep a clone ion to have initial position

            // FullCascade or CascadesOnly
            if (par_.simulation_type != IonsOnly) {

                pka.cascade_start(*j);

                if (cscd) {
                    cscd->init(*j);
                } else if (damage_stream_.is_open()) {
                    damage_ev.vacancy(*j);
                    damage_stream_.write(&damage_ev);
                }

                transport(j, cscd);

                // transport all secondary recoils
                while (ion *k = ion_queue_.pop_recoil()) {
                    transport(k, cscd);
                    // free the ion buffer
                    ion_queue_.free_ion(k);
                }

                if (cscd) {
                    cscd->intra_cascade_recombination();
                    cscd->tally_update(tion_);
                    if (damage_stream_.is_open())
                        cscd->stream_update(damage_stream_);
                }

                pka.mark(tion_);
                pka.cascade_end(*j, cscd);
            }

            // free the ion buffer
            ion_queue_.free_ion(j);

            // CascadeComplete event
            // Calc NRT values (using j1 - at initial pos!)
            pka.cascade_complete(
                    j1, par_.nrt_calculation == NRT_average ? target_->cell(j1.cellid()) : nullptr);
            ionEvent(Event::CascadeComplete, j1, &pka);

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
            for (int i = 0; i < utally_.size(); ++i) {
                *(utally_[i]) += *(ution_[i]);
                dutally_[i]->addSquared(*(ution_[i]));
            }
        }

        // clear the tally scores
        tion_.clear();
        for (int i = 0; i < utally_.size(); ++i)
            ution_[i]->clear();

        // update thread counter
        thread_ion_counter_++;

    } // ion loop

    if (cscd)
        delete cscd;

    return 0;
}

int mccore::transport(ion *i, abstract_cascade *cscd)
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
            if (cscd) {
                cscd->push_interstitial(*i);
            } else if (damage_stream_.is_open()) {
                damage_ev.interstitial(*i);
                damage_stream_.write(&damage_ev);
            }
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

            // a vacancy has been created
            if (cscd) {
                cscd->push_vacancy(j1);
            } else if (damage_stream_.is_open()) {
                damage_ev.vacancy(j1);
                damage_stream_.write(&damage_ev);
            }

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
    if (utally_.size()) {
        for (int i = 0; i < utally_.size(); ++i) {
            *(utally_[i]) += *(other.utally_[i]);
            other.utally_[i]->clear();
            *(dutally_[i]) += *(other.dutally_[i]);
            other.dutally_[i]->clear();
        }
    }
}

void mccore::mergeEvents(mccore &other)
{
    pka_stream_.merge(other.pka_stream_);
    other.pka_stream_.clear();
    exit_stream_.merge(other.exit_stream_);
    other.exit_stream_.clear();
    damage_stream_.merge(other.damage_stream_);
    other.damage_stream_.clear();
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
    utally_.push_back(new user_tally(p));
    dutally_.push_back(new user_tally(p));
    ution_.push_back(new user_tally(p));
}
