\page physics Short Physics Introduction

- \subpage basic-model-assump "Basic modelling assumptions"
- \subpage kmc-algorithm "Kinetic Monte-Carlo algorithm"
- \subpage flightpath "Flight Path Selection"
- \subpage damage-events "Damage Events"
- \subpage energy-partition "Energy partition"

\page basic-model-assump Basic modelling assumptions

The simulation is based on the following models and approximations:

- **Binary Collision Approximation (BCA)** 

  This means that the projectile traveling through the target collides with only one target atom at a time.
  Find more information in this [Wikipedia article](https://en.wikipedia.org/wiki/Binary_collision_approximation).

- **Random positions of target atoms**
  
  The crystalline structure of the target is neglected and the positions of target atoms are considered random.

- **Continuous slowing down approximation**
  
  Between nuclear collisions, the projectile looses energy continuously along its path due to its interaction with the target electrons. This is expressed by the electronic stopping power \f$dE/dx\f$.
  \sa \ref dedx

- **Screened Coulomb interactions**

  The interaction between projectile and target atoms is described by a screened Coulomb potential. 
  The scattering is considered elastic and it is approximated by classical kinematics. 
  \sa \ref XS, https://ir2-lab.github.io/screened_coulomb/

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
   This is implemented in \ref scattering_calc::scatter().
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






  
