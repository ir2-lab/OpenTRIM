\page screened-coulomb Screened Coulomb scattering

### Screened Coulomb potential definition

The general form of the screened Coulomb potential is

$$
V(r) = \frac{Z_1 Z_2 e^2}{r} \Phi(r/a)
$$

where \f$\Phi\f$ is the screening function and \f$a\f$ the screening length. The function \f$ \Phi(x) \f$ satisfies

$$
\Phi(0) = 1, \quad \Phi(\infty)\to 0
$$

Different forms of the screening function have been proposed. The most widely used one is a sum of exponentials

$$
\Phi(x) = \sum_i {A_i \, e^{-b_ix}}, \quad \sum_i{A_i}=1
$$

OpenTRIM has several definitions for screening functions of this type and the user can select the preferred one by setting `Simulation.screening_type` in the \ref json_config "configuration options".

The scattering of the incoming projectile ion from target nuclei is elastic, and it can be adequately described by classical scattering theory.

The scattering angle in the center-of-mass system is given by

$$
\theta = \pi - 2 s \int_{x_0}^\infty {x^{-2}F^{-1/2}(x)\,dx}
$$

where

$$
F(x) = 1 - \frac{\Phi(x)}{x\,\epsilon} - \frac{s^2}{x^2}
$$

and 
- \f$  \epsilon = E_{CM} a/Z_1 Z_2 e^2 \f$, with \f$ Z_1,Z_2 \f$ the atomic number of projectile and target atom, respectively, and \f$ E_{CM} \f$ the kinetic energy in the center-of-mass system.
- \f$ x = r/a \f$
- \f$ s = p/a \f$, where \f$ p \f$ is the impact parameter
- \f$ x_0 \f$ is the distance of closest approach, which satisfies
\f$ F(x_0)=0 \f$.

The integral can be evaluated numerically by quadrature.
Employing the Gauss–Chebyshev scheme (https://dlmf.nist.gov/3.5#v) it is obtained that
 
 \f[
   \theta = \pi - \frac{2\,s}{x_0} \frac{\pi}{N}
   \sum_{j=0}^{N/2-1}{H\left[ \cos\left( \frac{\pi}{N}\,j + \frac{\pi}{2N} \right) \right]}
 \f]

 where

 \f[
 H(u) = \sqrt{\frac{1-u^2}{F(x_0/u)}}
 \f]

As this is a computationally costly operation, the scattering integrals are typically pre-calculated and tabulated
for use in Monte-Carlo codes. 

### Cross-section and stopping power

The differential cross-section in the center-of-mass system is given by

\f[
  \frac{d\sigma}{d\Omega} = \frac{p}{\sin(\theta)}\left| \frac{dp}{d\theta}\right|
\f]

where \f$ d\Omega = 2 \pi \sin\theta d\theta \f$ is the element of solid angle.

Using the expression for the recoil energy \f$ T\f$ of the struck nucleus
\f[
  T = \gamma E \sin^2 \theta/2, \quad \gamma = 4m_1 m_2 / (m_1 + m_2)^2
\f]


Reduced stopping power

Calculate the reduced stopping power

\f[
S_n(\epsilon) = \frac{\epsilon}{\pi\, a^2 T_m N} \frac{dE}{dx} =
\frac{4 \epsilon}{a^2 T_m^2} \int_0^{T_m}{T\,\sigma(T) dT}
\f]

where \f$ \sigma \f$ is the reduced cross-section.

The integral is evaluated numerically.

Optionally, the function calculates the stopping power
for scattering angles below `theta_max`. In this case the
upper limit in the integral is replaced with

\f[
T(\theta_m) = T_m \, \sin^2 \theta_m/2
\f]

OpenTRIM includes a \ref XS "C++ library" that can be utilized to perform the above calculations. 
Additionally, a set of \ref xs_corteo "pre-computed scattering tables" are provided for different screening functions. These are employed in the OpenTRIM simulations but can also be used independently as external libraries. 

\sa XS, xs_corteo
