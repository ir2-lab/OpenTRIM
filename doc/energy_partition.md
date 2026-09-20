# Energy partition {#energy-partition}

An energetic ion moving inside a target material loses energy through its interactions with the target electrons and nuclei.

Energy transfer to the electrons, called electronic energy loss or electronic stopping, \f$\Delta E_e\f$, is treated as a continuous process and evaluated in OpenTRIM from interpolation tables of the stopping power as a function of the ion's kinetic energy.

The interaction with the nuclei is treated explicitly, as a sequence of binary collisions, within the BCA and screened Coulomb potential approximations.

If the recoil energy \f$T\f$ imparted to a target nucleus is above the displacement threshold \f$E_d\f$, the atom is displaced from its lattice site and becomes a moving ion, which loses energy by exactly the same processes. The potential energy needed to remove the atom from its lattice site and place it at an interstitial position, or, equivalently, the formation energy of the created Frenkel pair, is the lattice binding energy \f$E_l\f$. This energy is subtracted from the kinetic energy of the recoiling ion and remains stored in the lattice. See \ref damage-events "Damage events" for more details.

If, on the other hand, \f$T\f$ is below \f$E_d\f$, or whenever the kinetic energy of a moving ion drops below the absolute minimum energy \f$E_{min}\f$ (\ref _Transport_min_energy "Transport.min_energy"), the recoil or ion history is not followed any further and the remaining energy is added to the sub-threshold nuclear energy loss, \f$\Delta E_n\f$.

OpenTRIM records the energy deposited by the above processes per simulation cell and atom type in the standard tally `/tally/energy_deposition`, which has the following components:

- Electronic energy loss - \f$\Delta E_{e}\f$ (table `Electronic`) \n
  The energy transferred to the target electrons by the projectile and by all recoils, i.e., the electronic stopping power integrated along all ion tracks.

- Nuclear energy loss - \f$\Delta E_{n}\f$ (table `Nuclear`) \n
  The sub-threshold nuclear energy loss accumulated over the projectile and all target atom recoils. At the end of each source ion history no moving ions are left in the simulation volume; thus, all energy given to the nuclei has ended up either below threshold or in defect generation. \f$\Delta E_{n}\f$ therefore corresponds to the total energy transferred to the target nuclei as kinetic energy and *not* used for defect generation.

- Stored energy of lattice defects - \f$\Delta E_{Stored}\f$ (table `Stored`) \n
  The potential energy stored in the generated lattice defects.

- Lost energy - \f$E_{Lost}\f$ (table `Lost`) \n
  The kinetic energy carried out of the simulation volume by escaping ions and recoils.

The four components account for the whole of the initial projectile energy \f$E_0\f$:

$$
E_0 = \Delta E_{e} + \Delta E_{n} + \Delta E_{Stored} + E_{Lost}
$$

This balance holds exactly for each simulated ion history.

The sum \f$\Delta E_{n} + \Delta E_{Stored}\f$ corresponds to SRIM's *"Phonons"*.

The stored energy is related to the number of defects by

$$
   \Delta E_{Stored} = \frac{1}{2}\sum_{i}{(V_i + I_i)E_{l,i}}
$$

where \f$V_i\f$ and \f$I_i\f$ are the numbers of vacancies and interstitials, respectively, and the summation is over the different target atomic species. The factor \f$1/2\f$ arises because OpenTRIM assigns half of the Frenkel pair formation energy, \f$E_{l,i}/2\f$, to each of the two defects.
