\page physics Short Physics Introduction

- \subpage basic-model-assump "Basic modelling assumptions"
- \subpage screened-coulomb "Screened Coulomb scattering"
- \subpage kmc-algorithm "Kinetic Monte-Carlo algorithm"
- \subpage flightpath "Flight Path Selection"
- \subpage energy-partition "Energy partition"
- \subpage damage-events "Damage Events"

\page basic-model-assump Basic modelling assumptions

The simulation is based on the following models and approximations:

- **Binary Collision Approximation (BCA)** 

  This means that the projectile traveling through the target collides with only one target atom at a time.
  Find more information in this [Wikipedia article](https://en.wikipedia.org/wiki/Binary_collision_approximation).

- **Random positions of target atoms**
  
  The crystalline structure of the target is neglected and the positions of target atoms are considered random.

- **Continuous slowing down approximation**
  
  Between nuclear collisions, the projectile looses energy continuously along its path due to its interaction with the target electrons. This is expressed by the electronic stopping power \f$dE/dx\f$.

- **Screened Coulomb interactions**

  The interaction between projectile and target atoms is described by a screened Coulomb potential. 
  The scattering is considered elastic and it is approximated by classical kinematics. For more information on scattering calculations see the \ref screened-coulomb.

\page kmc-algorithm Kinetic Monte-Carlo algorithm

### Ion transport algorithm

The simulation of ion transport typically proceeds in the following manner:

1. Sample the distance to the next collision and the impact parameter of that collision. \n
   This is a very important part of the whole algorithm and is discussed in more detail in \ref flightpath.
2. Propagate the particle to the collision site. \n
   If a cell boundary is reached before collision then the particle is transferred to the new cell and the algorithm is restarted. If the boundary is an external boundary of the simulation volume, the ion exits and the current history ends. This step is implemented in the \ref ion::propagate() function.
3. Calculate the electronic stopping and straggling for the flight path and adjust the ion energy. \n
   For details see \ref dedx.
4. Select collision partner. \n
   An atom is selected randomly, taking into account the target composition.
5. Calculate scattering angle and recoil energy. \n
   This is implemented in \ref xs_lab::scatter().
6. If the energy transfer is above the displacement threshold, generate a recoil ion and store in the \ref ion_queue "ion queue" to be processed later.
7. Repeat the sequence from step 2 until either
  - the ion exits the simulation volume, or,
  - the ion energy goes below the cutoff (\ref _Transport_min_energy "Transport.min_energy" \f$\sim 1\f$ eV), whereupon it stops and remains implanted in the target

The above algorithm is encapsulated in the function mccore::transport().

### Complete simulation algorithm

To run a complete Monte-Carlo history, the following steps are taken:

1. Generate a projectile ion in the simulation volume, with energy, position and direction as specified by OpenTRIM's \ref json_config configuration options
2. Transport the ion through the simulation volume until it stops or until it exits
3. Transport all secondary and higher order recoils until the \ref ion_queue "ion queue" is empty 

The above is implemented in the function mccore::run().



\page energy-partition Energy partition

The energy of the projectile ion and subsequent recoils is deposited to the target as:

- Ionization energy of the target electrons - \f$E_{Ioniz}\f$ \n
  This is due to the electromagnetic interaction of the moving ion with the electron cloud. It is essentially equal to the stopping power integrated over the ion track.

- Stored energy of lattice defects - \f$E_{Stored}\f$ \n
  When the recoil energy is above the displacement threshold, the target atom is knocked out of its lattice position and a stable vacancy-interstitial pair, or Frenkel pair, is created.
  The Frenkel defect is associated with a formation energy, which is subtracted from the kinetic energy of the recoil. This energy remains stored in the lattice and can be released when the defect recombines.
  The Frenkel pair formation energy is the \ref atom::parameters::El parameter. It is assumed that one-half of El is due to the vacancy and the other half due to the interstitial. 

- Lattice vibrations - \f$E_{Lattice}\f$ \n
  This is due to the recoil energy of subthreshold collisions, which is released to the lattice as collective vibrations (or phonons).

- Lost energy - \f$E_{Lost}\f$\n
due to ions or recoils escaping the simulation volume. 

The sum of these four components is exactly equal to the initial projectile energy:

$$
E_0 = E_{Ioniz} + E_{Stored} + E_{Lattice} + E_{Lost}
$$

The sum \f$E_{Lattice} + E_{Stored}\f$ is what was called "Phonons" in SRIM. For compatibility, the value of \f$E_{Lattice} + E_{Stored}\f$ is also provided with the label "Phonons" in the \ref out_file "output file".

The following relation holds between \f$E_{Stored}\f$ and defect numbers:

$$
   E_{Stored} = \frac{1}{2}\sum_{i}{(V_i + I_i)E_{l,i}}
$$

where the summation is over the different target atomic species.

### Implementation in the Monte-Carlo code

Energy deposition from each simulated particle and the generation of target defects is registered (in tally scores) when the particle changes cell and at the end of the particle's history, in the following conditions:
- when the particle exits the simulation volume,
- when it stops and becomes implanted in the target,
- in a replacement event, where the ion replaces the recoil in the lattice 

Initially, a projectile ion is created with energy \f$E_0\f$, as specified by ion_beam::parameters::ionE0.

When a recoil is created after a collision with recoil energy \f$T\f$, it starts of with kinetic energy \f$T - E_l\f$, as \f$E_l\f$ is consumed in the formation of the vacancy-interstitial pair.

In the course of its passage through the target, the ion or recoil looses energy due to electronic stopping and nuclear collisions. If collisions are sub-threshold, the recoil energy goes directly to the lattice. Otherwise, it is transferred to the new recoil. The code keeps track of the current ion energy and the total amount of energy lost in each of these processes.

When the ion crosses a cell boundary, the total energy lost to ionization or nuclear collisions in the cell is added to the corresponding tally score tables.

When the ion stops and becomes implanted then
- Its kinetic energy is added to the lattice vibrational energy
- \f$E_l/2\f$ is deposited as stored energy at the starting position of the ion track (corresponding to the vacancy formation energy)
- \f$E_l/2\f$ is deposited as stored energy at the end position of the ion track (corresponding to the interstitial formation energy)

In a replacement event
- The ion kinetic energy is added to the lattice vibrational energy
- \f$E_l/2\f$, where \f$E_l\f$ refers to the recoiling atom lattice energy, is subtracted from the stored energy, since a vacancy is not created 
- If the moving ion is itself the result of a recoil event, then
  - \f$E_l/2\f$, where \f$E_l\f$ refers to the ion's lattice energy, is deposited as stored energy at the starting position of the ion track (corresponding to the vacancy formation energy)
  - \f$E_l/2\f$, where \f$E_l\f$ refers to the ion's lattice energy, is deposited as lattice vibrational energy at the end position of the ion track, corresponding to the interstitial formation energy that is now released

When the ion exits the simulation volume
- Its kinetic energy is added to the "lost" energy tally
- \f$E_l/2\f$ is deposited as stored energy at the starting position of the ion track (corresponding to the vacancy formation energy)
- \f$E_l/2\f$ is added to the lattice vibrational energy, as it corresponds to the interstitial that is finally not created

\page damage-events Damage Events

@todo write documentation on damage event handling


  
