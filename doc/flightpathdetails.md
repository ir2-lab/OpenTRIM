# OpenTRIM flight path selection {#flightpathdetails}

OpenTRIM has 2 different flight path sampling algorithms. 

The preferred mode can be set by the option \ref _Transport_flight_path_type "Transport.flight_path_type":
- \ref const_fp "\"Constant\"" - Constant flight path
- \ref var_fp "\"Variable\"" - Variable flight path

Each type of sampling can be adjusted by a set of parameters, which are described below.

The relevant algorithms are implemented in the class \ref flight_path_calc. A short description is also given in the following paragraphs.

## Constant flight path {#const_fp}

This is the simplest mode of flight path selection. 

The flight path between collisions is fixed to the constant value, \f$\ell_0\f$.

\f$\ell_0 \f$ is set by the \ref _Transport_flight_path_const "Transport.flight_path_const" option.
- It is given in units of the atomic radius \f$R_{at}\f$ of the material. 
- Default value: \f$ \ell_0 = 1 R_{at} \f$ 

The impact parameter is sampled uniformly on a circle of radius \f$p_{max} = (N\pi\ell_0)^{-1/2}\f$, thus, \f$p = p_{max} \sqrt{u}\f$ with \f$u \in (0,1)\f$.

This type of flight path selection is analogous to SRIM's "Monolayer" mode.

## Variable flight path {#var_fp}

### Parameters

- \ref _Transport_min_recoil_energy "Transport.min_recoil_energy" \n 
  This defines the minimum recoil energy, \f$T_c\f$, that can occur in a scattering event. 
  Interactions with \f$T<T_c\f$ are ignored.  
  - Default value: 1 eV.

- \ref _Transport_min_scattering_angle "Transport.min_scattering_angle" \n 
  Defines the minimum lab-system ion scattering angle, \f$\Theta_c\f$, that can occur in a scattering event. 
  Events with \f$\Theta < \Theta_c\f$ are ignored.    
  - Default value: 1 eV. \n

- \ref _Transport_max_rel_eloss "Transport.max_rel_eloss" \n 
  Defines the maximum relative ion energy loss, \f$(\Delta E / E)_{max} \f$, that can occur per flight path due to electronic stopping. \n
  This restricts the flight path \f$\Delta x\f$ so that 
  \f[
  \Delta E / E = (1/E)\int_0^{\Delta x}{(dE/dx)dx} \approx \Delta x\, N\, S_e(E)/E < (\Delta E / E)_{max} 
  \f] 
  or
  \f[
  \Delta x < \Delta x_{max}(E) = (\Delta E/E)_{max} \left[ N S_e(E) \right]^{-1}
  \f]
  - Default: \f$(\Delta E / E)_{max}=0.05\f$.\n

- \ref _Transport_mfp_range "Transport.mfp_range" \n 
  Defines the allowed range for the ion mean free path \f$\ell\f$. \n
  It is given in units of \f$R_{at}\f$.
  - Default: \f$ \ell / R_{at} \in [1, \, \infty)\f$

### Implementation

Before starting the simulation, the values of \f$p_{max}(E)\f$ and \f$\ell(E)=(N\pi p_{max}^2)^{-1}\f$ that are compatible with \f$T_c\f$ and \f$\Theta_c\f$ are computed on a coarse energy grid.

Limits can be optionally set on \f$\ell(E)\f$ (option \ref _Transport_mfp_range "Transport.mfp_range"). \n
The requirement that \f$\ell(E)\f$ remains within the specified range sets also the limits of \f$p_{max}\f$ 

\f[
(N\pi \ell_{max})^{-1/2} < p_{max} < (N\pi \ell_{min})^{-1/2}
\f]

The default \f$\ell(E)\f$ range actually sets a lower bound at \f$R_{at}\f$, i.e., roughly at the interatomic distance. This is the most anticipated condition for most users and is compatible with SRIM. 
It can be easily lifted by setting \f$\ell_{min}\f$ to a smaller value or 0.

The value of maximum allowed flight path \f$\Delta x_{max}(E)\f$ due to \ref _Transport_max_rel_eloss "Transport.max_rel_eloss" is also pre-computed on the same coarse energy grid as \f$p_{max}\f$ and \f$\ell\f$.

Finally, the path selection process comprises the following steps:

>
> **OpenTRIM variable flight path sampling**
> - Based on the ion energy \f$E\f$, select the most appropriate values of \f$p_{max}\f$, \f$\ell\f$ and \f$\Delta x_{max}\f$ from the pre-computed tables.
> - Take a uniform random sample \f$ u_1 \in (0,1) \f$
> - If \f$u_1 < e^{-\Delta x_{max}/\ell}\f$   
>   - reject the scattering event
>   - set the flight path equal to \f$\Delta x_{max}(E)\f$
> - otherwise
>   - set the flight path \f$\Delta x = -\ell \log u_1\f$
>   - sample the impact parameter \f$p = p_{max} \sqrt{u_2}\f$, where \f$ u_2 \in (0,1) \f$ a uniform random value
>


