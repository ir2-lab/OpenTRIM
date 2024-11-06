#include "ion.h"


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

// assuming the object was cloned from
// the projectile
void ion::init_recoil(const atom* a, double T)
{
    // mark current pos and cell
    // as initial for this track
    pos0_ = pos_;
    cellid0_ = cellid_;
    prev_cellid_ = -1;
    // set the energy & atom
    erg_ = erg0_ = T;
    atom_ = a;
    // increase recoil generation
    recoil_id_++;
}

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
 * @param s the distance to propagate the ion [nm]
 * @return the type of boundary crossing (none, internal (cell change), external (ion left simulation))
 */

// #pragma GCC push_options
// #pragma GCC optimize("O0")


BoundaryCrossing ion::propagate(float& s)
{
    vector3 x = pos_ + s*dir_;  // calc new ion position
    if (grid_->contains_with_bc(x)) { // is the ion still inside the target ?
        if (!grid_->contains(icell_,x)) { // does the ion exit the cell ?
            // propagate to the boundary
            x = pos_;
            s = bring2boundary(grid_->box(icell_), x, dir_);
            grid_->apply_bc(x); 
            ivector3 ix = grid_->pos2cell(x);
            path_ += s;
            pos_ = x;
            if (ix != icell_) {
                icell_ = ix;
                prev_cellid_ = cellid_;
                cellid_ = grid_->cellid(icell_);
                return BoundaryCrossing::Internal;
            } else {
                /* This is a rare case where although grid_->contains(icell_,x)
                 * returned false in the end the particle does not change cell.
                 * This can happen if the following 2 conditions hold simultaneously:
                 *   A) x_i = (pos + s*dir)_i == L1, where L1 is the boundary of the simulation
                 *      volume along the i-th direction
                 *   B) We have periodic boundary conditions along the i-th direction
                 *
                 * Thus, the call to grid_->contains_with_bc(x) returns true, due to the periodic BCs,
                 * BUT the call to grid_->contains(icell_,x) returns false because the ion is exactly at
                 * the boundary and this is considered "outside the cell" (condition= "x0 <= x < x1").
                 * Note that contains(icell_,x) does not check for periodic BCs.
                 */
                return BoundaryCrossing::InternalPBC;
            }
        } else { // we remain in the cell
            path_ += s;
            pos_ = x;
            return BoundaryCrossing::None;
        }
    } else { // ion is bound to exit simulation
        // 1. Reduce s to just cross the boundary

        // propagate to the boundary
        // @ToDo more debugging needed here
        x = pos_;
        s = bring2boundary(grid_->box(icell_), x, dir_);

        //s = distance2boundary(grid_->box(icell_),pos_,dir_);
        //x = pos_ + s*dir_;

        grid_->apply_bc(x);
        // 2. still exiting ?
        if (!grid_->contains_with_bc(x)) {
            pos_ = x;
            path_ += s;
            prev_cellid_ = cellid_;
            cellid_ = -1;
            return BoundaryCrossing::External;
        } else { // ion is still in target. just change the cell
            // get new pos and cell
            pos_ = x;
            path_ += s;
            prev_cellid_ = cellid_;
            icell_ = grid_->pos2cell(x);
            cellid_ = grid_->cellid(icell_);
            return BoundaryCrossing::Internal;
        }
    }
}

// #pragma GCC pop_options






