# Energy partition {#energy-partition}

The energy of the projectile ion and subsequent recoils is deposited to the target as:

- Ionization energy - \f$E_{Ioniz}\f$ \n
  This is due to the interaction of moving ions with the electron cloud. It is essentially equal to the stopping power integrated over the ion track.

- Stored energy of lattice defects - \f$E_{Stored}\f$ \n
  When the recoil energy is above the displacement threshold, the target atom is knocked out of its lattice position and a stable vacancy-interstitial pair, or Frenkel pair, is created.
  The Frenkel defect is associated with a formation energy, $E_l$, which is subtracted from the kinetic energy of recoil atoms. This energy remains stored in the lattice and can be released when the defect recombines. 

- Lattice vibrations - \f$E_{Lattice}\f$ \n
  This is due to subthreshold collisions where the recoil energy is released to the lattice as collective vibrations (or *phonons*).

- Lost energy - \f$E_{Lost}\f$\n
due to ions or recoils escaping the simulation volume. 

The sum of these four components is exactly equal to the initial projectile energy:

$$
E_0 = E_{Ioniz} + E_{Stored} + E_{Lattice} + E_{Lost}
$$

The sum \f$E_{Lattice} + E_{Stored}\f$ corresponds to SRIM *"Phonons"*. 

The following relation holds between \f$E_{Stored}\f$ and defect numbers:

$$
   E_{Stored} = \frac{1}{2}\sum_{i}{(V_i + I_i)E_{l,i}}
$$

where the summation is over the different target atomic species.





