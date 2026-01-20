#include "ion.h"
#include "target.h"

ion::ion()
    : pos_(0.f, 0.f, 0.f),
      pos0_(0.f, 0.f, 0.f),
      dir_(0.f, 0.f, 1.f),
      erg_(1.0),
      erg0_(1.0),
      t_(0.0),
      t0_(0.0),
      icell_(),
      cellid_(-1),
      prev_cellid_(-1),
      ion_id_(0),
      recoil_id_(0),
      atom_(nullptr),
      grid_(nullptr),
      ncoll_(0),
      path_(0),
      ioniz_(0),
      phonon_(0),
      recoil_(0)
{
}

int ion::setPos(const vector3 &x)
{
    pos_ = pos0_ = x;
    assert(x.allFinite());
    assert(grid_->contains(x));
    icell_ = grid_->pos2cell(x);
    assert(!grid3D::isNull(icell_));
    cellid_ = cellid0_ = grid_->cellid(icell_);
    prev_cellid_ = -1;
    return 0;
}

/// Set the atomic species of the ion
void ion::setAtom(const atom *a)
{
    atom_ = a;
    s_erg_to_t_ = S_ERG_TO_TIME_CONST * std::sqrt(atom_->M());
}

// assuming the object was cloned from
// the projectile
void ion::init_recoil(const atom *a, double T)
{
    // mark current pos and cell
    // as initial for this track
    pos0_ = pos_;
    cellid0_ = cellid_;
    prev_cellid_ = -1;
    // set the energy & atom
    erg_ = erg0_ = T;
    setAtom(a);
    // set starting time to the projectile time
    t0_ = t_;
    // increase recoil generation
    recoil_id_++;
}

// // For debuging though issues in propagate
// #pragma GCC push_options
// #pragma GCC optimize("O0")

/**
 * @brief Propagate the ion for a distance s [nm] taking care of boundary crossings
 *
 * The function first checks if the new ion position would be within the
 * simulation volume (taking periodic bc into account).
 *
 * If not, then the ion is propagated to just beyond its cell boundary and the
 * function checks again
 * if indeed the ion escapes the simulation. If yes then the ion exits. Otherwise, it
 * just changes cell.
 *
 * If the ion remains inside the simulation volume, the function checks
 * if it is in the same cell.
 *
 * If not, then the ion is propagated to just beyond its cell boundary,
 * the new cell is found
 * and the index vector and cell id are updated.
 *
 * In all cases of boundary crossing, s is adjusted to the minimum travel needed to just
 * overcome the boundary.
 *
 * @param fp the distance to propagate the ion [nm]
 * @return the type of boundary crossing (none, internal (cell change), external (ion left
 * simulation))
 */
BoundaryCrossing ion::propagate(float &fp)
{
    vector3 x = pos_ + fp * dir_; // calc new ion position
    if (grid_->contains_with_bc(x)) { // is the ion still inside the target ?
        if (grid_->contains(icell_, x)) { // is the ion still inside the cell ?
            // yes we remain in the cell
            path_ += fp;
            t_ += fp / std::sqrt(erg_) * s_erg_to_t_;
            pos_ = x;
            return BoundaryCrossing::None;
        } else { // ion exits the cell
            // propagate to the boundary
            x = pos_;
            fp = grid_->bring2boundary(icell_, x, dir_);
            assert(finite(fp));
            grid_->apply_bc(x);
            ivector3 ix = grid_->pos2cell(x);
            path_ += fp;
            t_ += fp / std::sqrt(erg_) * s_erg_to_t_;
            pos_ = x;
            if (ix != icell_) { // we crossed an internal cell boundary
                icell_ = ix;
                prev_cellid_ = cellid_;
                cellid_ = grid_->cellid(icell_);
                return BoundaryCrossing::Internal;
            } else {
                /* We crossed an internal boundary but did not change cell !!
                 *
                 * This is a rare case where although grid_->contains(icell_,x)
                 * returned false in the end the particle does not change cell.
                 * This can happen if the following 3 conditions hold simultaneously:
                 *   A) Particle reaches a boundary along the i-th direction
                 *   B) We have periodic boundary conditions along the i-th direction
                 *   C) There is only 1 cell along the i-th direction at this point, thus
                 *      the particel does not change cell
                 *
                 * Thus, the call to grid_->contains_with_bc(x) returns true, due to the periodic
                 * BCs, BUT the call to grid_->contains(icell_,x) returns false because the ion is
                 * exactly at the boundary and this is considered "outside the cell" (condition= "x0
                 * <= x < x1"). Note that contains(icell_,x) does not check for periodic BCs.
                 */
                return BoundaryCrossing::InternalPBC;
            }
        }
    } else { // ion is bound to exit simulation
        // 1. Reduce s to just cross the boundary
        // @ToDo more debugging needed here
        x = pos_;
        fp = grid_->bring2boundary(icell_, x, dir_);
        assert(finite(fp));
        path_ += fp;
        t_ += fp / std::sqrt(erg_) * s_erg_to_t_;
        grid_->apply_bc(x);
        // 2. still exiting ?
        if (!grid_->contains_with_bc(x)) {
            pos_ = x;
            prev_cellid_ = cellid_;
            cellid_ = -1;
            return BoundaryCrossing::External;
        } else { // ion is still in target. just change the cell
            // get new pos and cell
            pos_ = x;
            prev_cellid_ = cellid_;
            icell_ = grid_->pos2cell(x);
            cellid_ = grid_->cellid(icell_);
            return BoundaryCrossing::Internal;
        }
    }
}

float ion::move(float s)
{
    float fp(s);
    vector3 x = pos_ + fp * dir_; // calc new ion position
    if (grid_->contains_with_bc(x)) { // is the ion still inside the target ?
        if (!grid_->contains(icell_, x)) { // does the ion exit the cell ?
            // propagate to the boundary
            x = pos_;
            fp = grid_->bring2boundary(icell_, x, dir_);
            assert(finite(fp));
            grid_->apply_bc(x);
            ivector3 ix = grid_->pos2cell(x);
            pos_ = x;
            if (ix != icell_) {
                icell_ = ix;
                prev_cellid_ = cellid_;
                cellid_ = grid_->cellid(icell_);
            }
        } else { // we remain in the cell
            pos_ = x;
        }
    } else { // ion is bound to exit simulation
        // do nothing !!
        fp = 0;
    }
    return fp;
}

// #pragma GCC pop_options
