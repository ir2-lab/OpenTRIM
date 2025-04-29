# Damage Events {#damage-events}

Damage generation is modelled based on energy levels, as is done in SRIM and other Monte-Carlo and BCA codes.

The relevant energy parameters are:

- **The displacement energy** \f$E_d\f$ \n
  This is a property of the atoms making up the target materials. When a target atom
  receives recoil energy higher than \f$E_d\f$ it is displaced from its atomic position. \n
  
- **The lattice binding energy** \f$E_l\f$ \n
  When an atom is displaced a vacancy-intersitial pair, or Frenkel pair, is created. This defect has a certain formation energy, which is a characteristic of the material. This is called \f$E_l\f$ in OpenTRIM for compatibility with previous codes.
  In principle, the Frenkel pair formation energy is the sum of the energies needed to form the vacancy and the interstitial, which need not be equal. However, for simplicity, OpenTRIM assigns a formation energy of \f$E_l/2\f$ to both the vacancy and the interstitial atom.

- **The recombination energy** \f$E_r\f$ \n
  When the kinetic energy of an ion is below \f$E_r\f$, then it can be captured by a nearby vacancy of the same atomic species. When this happens during a collision it is called a *replacement* (see below). Otherwise, it leads to recombination within a recoil cascade, which is an experimental feature of OpenTRIM still under development. \n
  Typically \f$E_r\f$ is set equal to \f$E_d\f$. 


\f$E_d\f$, \f$E_l\f$ and \f$E_r\f$ are set by parameters with the same mnemonic in the \ref json_config "JSON config". They appear for each element in the material composition definition. See the examples in \ref json_config section.

A damage event occurs as follows:

- An incoming ion A with kinetic energy \f$E\f$ collides with a target atom B and transfers to it an energy \f$T\f$.
- If \f$T > E_d^B\f$, then B starts moving with kinetic energy \f$T-E_l^B\f$ leaving behind a lattice vacancy.
- Otherwise, \f$T\f$ is deposited as lattice excitation ("Phonons" in SRIM terminology)
- A emerges from the collision with energy \f$E-T\f$
- If A is the same atomic species as B and \f$E-T<E_r^B\f$, then A replaces B in the lattice. This constitutes a replacement event. \f$E-T\f$ is deposited as lattice excitation.
- Otherwise, A continues its track.

## Primary knock-on atoms (PKAs)

PKAs are the atoms of the target that are first hit by the an ion of the external beam, obtaining energy above the displacement threshold and thus generating a recoil cascade.

OpenTRIM pays special attention to PKA events and keeps separate records of the damage and energy deposition within the PKA cascade. These are reported in the special section `/tally/pka_damage` of the \ref out_file "HDF5 output archive".

Quantities like the number of PKAs per ion, average PKA energy, average damage energy, etc, are tallied and reported separately.

## NRT implementation

To aid in comparisons to standard damage calculations or to neutron irradiation, OpenTRIM employs the standard NRT model to give an independent estimation of the generated defects.

The NRT damage is calculated based on:

- The damage energy of the PKA cascade as it is found by the Monte-Carlo simulation. This is reported in the tally table `/tally/pka_damage/Vnrt` 
- The damage energy estimated by the LSS partition. This is reported in `/tally/pka_damage/Vnrt_LSS` and can be used for comparison to other codes, e.g., SRIM

In multi-elemental targets, the application of NRT is not well defined. The option \ref _Simulation_nrt_calculation "/Simulation/nrt_calculation" selects among two alternatives:
- **"NRT_element"**: the model is implemented using the parameters of the specific PKA recoil atom
- **"NRT_average"**: the model is implemented using an effective displacement threshold defined according to Ghoniem and Chou JNM1988