# OpenTRIM flight path selection {#flightpathdetails}

OpenTRIM has 3 different flight path sampling algorithms, which cover all the above techniques. The preferred mode for a given simulation is selected by the option \ref _Transport_flight_path_type "Transport.flight_path_type":
- **"Constant"** - Constant flight path
- **"MHW"** - Mendenhall-Weber / SRIM-like algorithm
- **"FullMC"** - "Full" Monte-Carlo algorithm (see text)

Each type of sampling can be adjusted by a set of parameters, which are described below.

The relevant algorithms are implemented in the class \ref flight_path_calc. A short description of the details is also given in the following paragraphs.

## Parameters for adjusting flight path selection

- \ref _Transport_flight_path_const "Transport.flight_path_const" \n
  This is the flight path, \f$\Delta x\f$, to be used in the **Constant** mode. \n
  It is given in units of the atomic radius \f$R_{at}\f$ of the material. The default is 
  \f[
  \Delta x = (4/3) R_{at}
  \f]. 
  This particular value is selected so that the corresponding \f$p_{max} = (N\pi \Delta x)^{-1/2} = R_{at}\f$, i.e., the potential is truncated at the atomic radius.

- \ref _Transport_min_recoil_energy "Transport.min_recoil_energy" \n 
  This defines the lower cutoff recoil energy, \f$T_c\f$. The default is 1 eV.\n
  It is used in both MHW and FullMC modes.

- \ref _Transport_min_scattering_angle "Transport.min_scattering_angle" \n 
  This defines the lower cutoff in the lab-system scattering angle, \f$\Theta_c\f$. The default is \f$2^\circ\f$.\n
  It is used in both MHW and FullMC modes.

- \ref _Transport_max_rel_eloss "Transport.max_rel_eloss" \n 
  Defines the maximum relative electronic energy loss, \f$\delta\f$, per flight path. \n
  This restricts the flight path \f$\Delta x\f$ so that 
  \f[
  \Delta E / E = (1/E)\int_0^{\Delta x}{(dE/dx)dx} < \delta
  \f] 
  The default is \f$\delta=0.05\f$.\n
  It is used in both MHW and FullMC modes.

- \ref _Transport_mfp_range "Transport.mfp_range" \n 
  Defines the allowed range for the ion mean free path \f$\ell\f$. \n
  It is given in units of \f$R_{at}\f$ and the default range is 
  \f[\ell \in [(4/3) R_{at}, \, \infty)\f]
  It is used only in the FullMC mode.

## Constant flight path

This is the simplest mode of flight path selection. The flight path is fixed to a constant value, \f$\Delta x\f$, and no sampling is performed.

\f$\Delta x\f$ can be set with the \ref _Transport_flight_path_const "Transport.flight_path_const" option.

This type of flight path selection is analogous to SRIM's "Monolayer" mode.

## Mendenhal-Weber (MHW) flight path selection

OpenTRIM implements the MHW flight path algorithm, which is very similar to SRIM, extending it with 2 additional components:
-  the effective total cross-section is obtained by setting a lower cutoff not only on the recoil energy but also on the scattering angle. This ensures that the ion trajectory is accurately simulated. 
-  an upper bound to the flight path is set by requiring that the fractional electronic energy loss along the path remains below a certain level.  

Parameters:
- \f$T_c\f$: recoil energy cutoff
- \f$\Theta_c\f$: Lab system scattering angle cutoff
- \f$(\Delta E_e/E)_{max}\f$: Upper bound for the relative electronic energy loss

### Combined recoil energy and scattering angle cutoff
Scattering events with both \f$T < T_c \f$ and \f$ \Theta < \Theta_c \f$ are neglected.

The 2 conditions can be combined either in \f$T\f$- or in \f$\theta\f$-space, where \f$\theta\f$ is the center-of-mass scattering angle. 

Noting that for small scattering angles 
\f[ \Theta \approx \theta / (1 + m_1/m_2),\f]  
and
\f]
T=\gamma E \sin^2(\theta/2) \approx (m_1/m_2) E \Theta^2,
\f]
the combined condition for the recoil energy is
\f[
T < T_c(E) = \min \left\{ T_c, \; (m_1/m_2) E \, \Theta_c^2 \right\}
\f]

The same condition expressed in scattering angle space is
\f[
\theta < \theta_c(E) = \min \left\{ 2\sqrt{\frac{T_c}{\gamma E}}, \; \Theta_c \left( 1+\frac{m_2}{m_1}\right) \right\}
\f]

For a multielemental target, we calculate the condition for each target element and find the minimum
\f[
\theta_c(E) = \min_i \left\{ \theta_{c,i}(E) \right\}
\f]
where $i$ indexes the elements in the material.
 
Having defined the energy dependent cutoff scattering angle \f$\theta_c(E)\f$ we can now obtain the corresponding maximum impact parameter  
\f[
p_{max} = p(E,\theta_c(E))
\f]
by numerically inverting the scattering angle vs. \f$p\f$ function, \f$\theta = \theta(E,p)\f$. 

In a multi-elemental target, this is generalized to
\f[
p_{max}^2 = \sum_i{X_i \, p_i^2(E,\theta_c(E))}
\f]
where \f$X_i\f$ is the atomic fraction of the \f$i\f$-th element and \f$p_i(E,\theta)\f$ denotes the function for the specific projectile/target element combination. 

### Upper bound to the relative electronic energy loss

The electronic energy loss for a flight path \f$\Delta x\f$ is
\f[
\Delta E_e = \Delta x\, N S_e(E)
\f]

To keep the relative energy loss below \f$(\Delta E_e/E)_{max}\f$ we set the maximum path length equal to
\f[
\Delta x_{max} = (\Delta E_e/E)_{max} \left[ N S_e(E) \right]^{-1}
\f]

\f$p_{max}\f$ must be modified accordingly:
\f[
p_{max} = \max \left\{ (N\pi \Delta x_{max})^{-1/2}, \; p_{max} \right\}
\f]

### Truncating the potential

The impact parameter is truncated at \f$R_{at}\f$:
\f[
p_{max} = \min \left\{ R_{at}, \; p(E,\theta_c(E)) \right\}
\f]
This effectively means that there is no interaction for distances \f$r > R_{at}\f$.

### Long flight paths

When simulating high energy ions with long flight-paths, the resulting damage profile would become un-physical as there will be regions where the particles fly over without creating any defects.

SRIM's solution for this, as stated in the manual (p.7-8) is:

> The free flight path is not a constant length, for once the maximum jump distance is estimated, this length is reduced by a random number (\f$0<u<1\f$) to distribute the probability of the collision to somewhere randomly within the jump length.

For compatibility, this is adopted also in OpenTRIM's implementation of this algorithm. 
\f$\Delta x\f$ is always multiplied by a random value if it exceeds \f$R_{at}\f$. \todo A more optimized solution must be found, where \f$\Delta x\f$ is varied only if it is above some threshold.

It is noted that setting equal probability for collision within the flight path is in principle not statistically correct, as the probability of scattering increases as \f$1-e^{-x/\ell}\f$ for \f$0<x< \Delta x\f$.

### Implementation

Before starting the simulation, pre-compute a table of \f$p_{max}(E)\f$ on a coarse energy grid.

Then, to select a flight path for an ion with energy \f$E\f$ do the following:
- Take a uniform random variate \f$ u_1 \in (0,1) \f$
- If \f$p_{max}(E) < R_{at}\f$
   
   - if \f$u_1 < e^{-1}\f$ reject the scattering event and set \f$\Delta x = (N\pi p_{max}^2)\f$
     otherwise
     - \f$p = p_{max}(E) \sqrt{-\log u}\f$
     - \f$\Delta x = (N\pi p_{max}^2)^{-1} \cdot u_2\f$ with a random value \f$ u_2 \in (0,1) \f$

   otherwise

   - \f$p=R_{at} \sqrt{u}\f$
   - \f$\Delta x = 1.1547\, R_{at}\f$


## FullMC flight path selection

OpenTRIM implements a "full Monte-Carlo algorithm", without the simplifications employed in MHW & SRIM. Thus, the flight path is first sampled from the exponential distribution \f$e^{-x/\ell}\f$ and the impact parameter is sampled uniformly within 
the circle of area \f$\sigma_0 = \pi p_{max}^2\f$.

This requires 2 random generator calls. In practice, this does not severely hinder the efficiency even on a PC of average capacity. The advent of modern, fast random generator algorithms helps a lot in this direction.

The advantages of the FullMC flight path selection strategy are:
- Correct spatial distribution of scattering events
- Statistically sound implementation of path cutoff due to electronic energy loss \f$\Delta E_e\f$. If the sampled flight path leads to \f$\Delta E_e\f$ above the threshold, the scattering is rejected and the particle path is truncated.

### Parameters:
- \f$T_c\f$: recoil energy cutoff
- \f$\Theta_c\f$: Lab system scattering angle cutoff
- \f$(\Delta E_e/E)_{max}\f$: Upper bound for the relative electronic energy loss
- Allowed mean free path (mfp) range \f$ [\ell_{min}, \ell_{max}] \f$ 

### Maximum impact parameter & mean free path
 
The maximum impact parameter is calculated exactly as in the MHW case. However, no truncation is performed by default. Limits are imposed by the optional mfp range. The requirement that the mfp, \f$\ell = (N\pi p_{max}^2)^{-1}\f$, remains within the specified range results in 

\f[
p_{max} = \min \left\{ (N\pi \ell_{min})^{-1/2}, \; p_{max} \right\}
\f]

\f[
p_{max} = \max \left\{ (N\pi \ell_{max})^{-1/2}, \; p_{max} \right\}
\f]

The default mfp range actually employs the conventional truncation of the potential, since this is the most anticipated condition for most users. It can be easily lifted by setting \f$\ell_{min}=0\f$

### Implementation

Before starting the simulation, pre-compute a table of \f$p_{max}(E)\f$, \f$\ell(E)\f$, \f$\Delta x_{max}(E)\f$ on a coarse energy grid.

To select a flight path for an ion with energy \f$E\f$ do the following:

- Take a uniform random variate \f$ u_1 \in (0,1) \f$
- If \f$u_1 < e^{-\Delta x_{max}/\ell}\f$
   
   - reject the scattering event
   - set the flight path equal to \f$\Delta x_{max}(E)\f$

   otherwise

   - set the flight path \f$\Delta x = -\ell \log u_1\f$
   - sample the impact parameter \f$p = p_{max} \sqrt{u_2}\f$, with \f$ u_2 \in (0,1) \f$ a random value

## Sampling impact parameter and azimuthal scattering angle in one go

To implement the random ion scattering we typically first sample the impact parameter, \f$p\f$, which defines the polar scattering angle \f$\theta\f$, and then the azimuthal scattering angle \f$\phi\f$ uniformly in \f$[0, 2\pi)\f$.

This requires 2 random numbers, one for each variable. Nevertheless, it is more efficient to sample \f$\phi\f$ using 2 random numbers, to avoid calling trigonometric functions. Thus, the required number of random numbers is actually 3.

A typical such algorithm for sampling \f$\phi\f$, employed also in OpenTRIM, is:

> - get 2 uniform random values: \f$x \in [-1, 1)\f$ and \f$y \in [0, 1)\f$
> - compute \f$x^2\f$, \f$y^2\f$ and \f$r^2 = x^2 + y^2\f$
> - repeat until \f$0 < r^2 \leq 1\f$
> - return \f$\left(\frac{x^2-y^2}{r^2},\frac{2 x y}{r^2}\right)\f$, which is equivalent to \f$(\cos\phi, \sin\phi)\f$, \f$\phi \in [0, 2\pi).\f$

This essentially computes a random 2D unit vector \f$\mathbf{n}\f$, which is exactly what we need for the scattering calculation.

However, we can use the random norm \f$r\f$ from the above algorithm as a sample for the impact parameter, \f$p = p_{max} \cdot r.\f$ In this manner, we essentially sample the random position of the scattering atom on a circular disk of radius \f$p_{max}\f$ perpendicular to the ion's direction. Thus, the number of random number generator calls becomes again 2.

This is actually a small improvement of the order of 1-5% in computation speed.