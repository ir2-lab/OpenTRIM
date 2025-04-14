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

1. Sample the distance to the next collision and the impact parameter of that collision. This is a very important part of the whole algorithm and is discussed in more detail in \ref flightpath.
2. Propagate the particle to the collision site. If a cell boundary is reached before collision then the particle is transferred to the new cell and the algorithm is restarted. If the boundary is an external boundary of the simulation volume, the ion exits and the current history ends. This step is implemented in the \ref ion::propagate() function.
3. Calculate the electronic stopping and straggling for the flight path and subtract from the ion energy. For details see \ref dedx.
4. Select collision partner. An atom is selected randomly, taking into account the target composition.
5. Calculate scattering angle and recoil energy. This is implemented in \ref xs_lab::scatter().
6. If the energy of the recoiling target atom is above the displacement threshold, then a recoil ion is produced and stored in the \ref ion_queue "ion queue" to be processed later.
7. Repeat the sequence from step 2 until either
  - the ion exits the simulation volume, or,
  - the ion energy goes below the cutoff (\ref mccore::parameters::min_energy \f$\sim 1\f$ eV), whereupon it stops and remains implanted in the target

The above algorithm is encapsulated in the function mccore::transport().

### Complete simulation algorithm

To run a complete Monte-Carlo history, the following steps are taken:

1. Generate a projectile ion in the simulation volume, with energy, position and direction as specified by OpenTRIM's \ref json_config configuration options
2. Transport the ion through the simulation volume until it stops or until it exits
3. Transport all secondary and higher order recoils until the \ref ion_queue "ion queue" is empty 

The above is implemented in the function mccore::run().

\page flightpath Flight Path Selection

In MC particle transport simulations, the distance to the next collision is typically sampled from the exponential distribution, \f$p(x) = N\sigma_0 e^{-N\sigma_0 x}=\ell^{-1}e^{-x/\ell}\f$, where \f$N\f$ is the atomic density of scattering centers, \f$\sigma_0\f$ the total cross-section and \f$\ell = (N\sigma_0)^{-1}\f$ denotes the mean free path (mfp).

There is a difficulty in employing this type of flight path selection for the Coulomb and screened Coulomb interactions because the total cross-section diverges. This can be circumvented by setting a lower cutoff, either w.r.t. the scattering angle or the recoil energy or some other scattering parameter. Scattering events below the cutoff are ignored. The cutoff value is selected so that the result of the calculation is not affected significantly.

Mendenhall & Weller 2005 set such a lower cutoff for the recoil energy, \f$T_c\f$, which corresponds to the lowest recoil energy of interest in the problem under study. M&W suggest a value in the range of 1 - 10 eV for ion penetration in solids. 

\f$T_c\f$ corresponds to a lower bound \f$\theta_c\f$ for the center-of-mass scattering angle, which can be obtained from 
$$
T_c = T_m \sin^2(\theta_c/2)
$$
For a given reduced energy \f$\epsilon\f$ of the particle, we can find the maximum impact parameter, \f$p_{max}\f$, corresponding to \f$\theta_c\f$ and thus define the effective total cross-section
$$
\sigma_0 = \pi\, p_{max}^2(\epsilon) = 
\int_0^{p_{max}}{2\pi\, p\, dp} =
\int_{\theta_c}^{\pi}{d\sigma(\epsilon,\theta)}
$$

The main steps of the Mendenhall-Weller algorithm are as follows

> **Mendenhall-Weller (MHW) Algorithm:**
> - For a given particle energy \f$E\f$ find the values of \f$p_{max}\f$, \f$\sigma_0\f$ and \f$\ell\f$ corresponding to the recoil energy cutoff \f$T_c\f$
> - Take a random number sample \f$u \in (0,1)\f$
> - If \f$p_{max} < L\f$, where \f$L\f$ is approx. half the interatomic distance, 
>    - \f$p = p_{max} \sqrt{-\log u}\f$
>    - If \f$p>p_{max}\f$ reject the scattering event 
>    - Propagate particle by \f$\ell\f$
> - if \f$p_{max} \geq L\f$ then 
>    - \f$p = \sqrt{u /(\pi N L)}\f$
>    - Propagate particle by \f$L\f$ 

This is essentially the same algorithm as employed by SRIM.

The 2nd part of the algorithm, for \f$p_{max} \geq L\f$, ensures that impact parameters larger than \f$L\f$ do not occur. This is accepted in both SRIM and M&W2005. M&W justify it as follows:

> ... impact parameters larger than half the lattice spacing do not occur, since then one is closer to the adjacent atom.

while Biersack1980 and the SRIM manual (Ch.7) write:

> This
> procedure maintains the atomic density in the target
> without correlating the lateral positions of successive
> target atoms (neglection of lattice structure).

We believe that these propositions are not justified and one can assume a flight path smaller than the interatomic distance without any problems.

We propose to employ a more "standard" procedure for path selection similar to what is done for neutron or photon transport simulations. It is specified as "FullMC" path selection:

> **FullMC path selection algorithm**
> - For a given particle energy \f$E\f$ we have pre-computed \f$p_{max}\f$, \f$\sigma_0=\pi p_{max}^2\f$ and \f$\ell = (N\sigma_0)^{-1}\f$
> - Additionally, we have pre-computed values of maximum path-length \f$\Delta x_{max}\f$, 
> such that the relative electronic energy loss \f$\Delta E/E = (1/E)\int_0^{\Delta x_{max}} {(dE/dx)dx} \leq \delta \sim 0.01-0.05\f$. The value  
> \f$u_c = e^{-\Delta x_{max}/\ell}\f$, which corresponds to the probability that \f$\Delta x > \Delta x_{max}\f$, is also precomputed.
> - Take a random sample \f$u_{1} \in (0,1)\f$
> - If \f$u_1 < u_c\f$, reject the scattering event and propagate the particle by \f$\Delta x_{max}\f$
> - Otherwise, compute the Poisson distributed path to the next collision \f$\Delta x = -\ell\,\log u_1\f$ 
> - Take a 2nd random sample \f$u_{2} \in (0,1)\f$ and compute the impact parameter as
> \f$p = p_{max}\sqrt{u_2}\f$
> - Propagate the particle by \f$\Delta x\f$

Although FullMC needs 2 random numbers instead of 1 in the MHW/SRIM algorithm, the penalty is not so high since all outcomes are valid (in M&W algorithm some events are discarted). 

An advantage of this method is that the average mfp of the simulated ions is exactly \f$\ell\f$, which in the MW algorithm is not true. This permits us to preset a value for the mfp. This can be useful, e.g., when simulating thin targets (setting \f$\ell \ll d\f$ ensures scattering in the target)

### Parameters for adjusting flight path selection

A number of different criteria can be used in parallel for setting \f$p_{max}\f$, \f$\sigma_0\f$ and \f$\ell\f$.

The corresponding options are in the **Transport** section of the \ref json_config "JSON configuration options".

- **Recoil energy lower cutoff** \n 
  As explained above, a lower cutoff \f$T_c\f$ in the recoil energy results in a maximum impact parameter \f$p_{max} = p(E,T_c)\f$ and \f$\ell = 1/\sqrt{\pi N p_{max}^2}\f$.

  This can be set by changing the value of **Transport.min_recoil_energy** (in eV). The default is 1eV, equal to **Transport.min_energy**.

  This parameter is used in both MHW and IPP flight path selection schemes.

- **Upper bound due to electronic stopping** \n 
  \f$\Delta E / E = (1/E)\int_0^{\Delta x}{(dE/dx)dx} < \delta\f$ or \f$\Delta x < \Delta x_{max}(E)\f$. Typically, a \f$\delta\f$ of 0.01 - 0.05 is employed.

  This is set by **Transport.max_rel_eloss**. The default is 0.05.

- **Upper bound for the mfp** \n 
  \f$\ell < \ell_{max}\f$, where \f$\ell_{max}\f$ is set for geometric reasons

  This is set by **Transport.max_mfp**. The default is a huge value (option de-activated).

Tables of \f$p_{max}\f$, \f$\sigma_0\f$, \f$\ell\f$ and \f$\Delta x_{max}\f$ as a function of incident energy are calculated before starting the main simulation.

The relevant algorithms are implemented in the class \ref flight_path_calc.

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


  
