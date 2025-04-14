#ifndef DEDX_CALC_H
#define DEDX_CALC_H

#include "dedx.h"
#include "arrays.h"
#include "random_vars.h"
#include "ion.h"

class ion;
class material;
class mccore;

/**
 * @brief The dedx_calc class is used for electronic energy loss and straggling calculations
 *
 * The class stores pre-computed interpolation tables for all ion/material combinations
 * present in a given simulation. These tables are generated initially with a call to init(const
 * mccore& s).
 *
 * During the simulation, init(const ion* i, const material* m) should be
 * called each time a new ion/material combination arises, to preload the
 * required tables for max efficiency.
 *
 * Then, the energy loss of the ion for a given flight path is effected by calling
 * operator() on the dedx_calc object.
 *
 *
 * \ingroup Core
 * \sa dedx
 */
class dedx_calc
{
public:
    /**
     * @brief Determines electronic energy loss calculation mode
     */
    enum eloss_calculation_t {
        EnergyLossOff = 0, /**< Electronic energy loss disabled */
        EnergyLoss = 1, /**< Only electronic energy loss is calculated */
        EnergyLossAndStraggling = 2, /**< Both energy loss & straggling are calculated */
        InvalidEnergyLoss = -1
    };

    dedx_calc();
    dedx_calc(const dedx_calc &other);
    ~dedx_calc();

    eloss_calculation_t type() const { return type_; }

    // dedx interpolators
    ArrayND<dedx_interp *> dedx() const { return dedx_; }
    ArrayND<straggling_interp *> de_strag() const { return de_strag_; }

    /**
     * @brief Initialize the object for a given simulation \a s
     *
     * Computes the interpolation tables for all ion/material
     * combinations required by the given simulation.
     *
     * Should be called once after the projectile and all target materials
     * have been defined in \a s and before starting the transport
     * simulation.
     *
     * @param s a reference to an \ref mccore object
     * @return 0 if succesfull
     */
    int init(const mccore &s);

    /**
     * @brief Preload interpolation tables for a given ion/material combination
     *
     * This function should be called each time a new ion/material combination
     * is needed during the simulation. This could be, e.g., when a new recoil
     * is created, when an ion crosses the boundary between two materials.
     *
     * @param i pointer to an \ref ion object
     * @param m pointer to a \ref material object
     * @return 0 if succesfull
     */
    int preload(const ion *i, const material *m);

    /**
     * @brief Calculate electronic energy loss and straggling of the moving ion
     *
     * The specific stopping & straggling values are obtained by
     * calling dedx_interp() on the respective interpolator object.
     *
     * Tables follow the corteo indexing scheme, see \ref dedx_index.
     *
     * @param rng random number generator (used for straggling calc)
     * @param E ion energy [eV]
     * @param fp flight path [nm]
     * @param sqrtfp sqrt of fp/(atomic radius) (used for straggling)
     * @param stopping_tbl pointer to stopping interpolator
     * @param straggling_tbl pointer to straggling interpolator
     * @return the energy loss [eV]
     *
     * \sa dedx
     */

    /**
     * @brief Calculate and subtract electronic energy loss and straggling of a moving ion
     *
     * The calculation is based on interpolation tables preloaded for a specific
     * ion/material combination by a previous
     * call ton init(const ion* i, const material* m).
     *
     * @param i pointer to an \ref ion object
     * @param fp the ion's flight path [nm]
     * @param sqrtfp sqrt of fp/(atomic radius) (used for straggling)
     * @param rng random number generator (used for straggling)
     * @return  the total energy loss [eV]
     */
    void operator()(ion *i, float fp, float sqrtfp, random_vars &rng) const
    {
        if (type_ == EnergyLossOff)
            return;
        float E = i->erg();
        float de_stopping = fp * (*stopping_interp_)(E);

        if (type_ == EnergyLossAndStraggling) {
            dedx_index ie(E);
            float de_straggling = straggling_interp_->data()[ie] * rng.normal() * sqrtfp;

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
            de_stopping = 0.99 * E;

        i->de_ioniz(de_stopping);
    }

    void operator()(ion *i, float fp) const
    {
        if (type_ == EnergyLossOff)
            return;
        float E = i->erg();
        float de_stopping = fp * (*stopping_interp_)(E);

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
            de_stopping = 0.99 * E;

        i->de_ioniz(de_stopping);
    }

protected:
    eloss_calculation_t type_;

    // Electronic Stopping & Straggling Tables
    ArrayND<dedx_interp *> dedx_; // stopping data (atoms x materials)
    ArrayND<straggling_interp *> de_strag_; // straggling data (atoms x materials)

    const dedx_interp *stopping_interp_;
    const straggling_interp *straggling_interp_;
};

#endif // DEDX_CALC_H
