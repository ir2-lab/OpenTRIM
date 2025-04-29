# Screened Coulomb scattering {#screened-coulomb}

## Screened Coulomb potential definition

The interaction of projectile ions (atomic number \f$Z_1\f$, mass \f$m_1\f$) with target atoms \f$(Z_2,m_2)\f$ is described by the screened Coulomb potential 
$$
V(r) = \frac{Z_1 Z_2 e^2}{r} \Phi(r/a)
$$
where \f$\Phi\f$ is the screening function and \f$a\f$ the screening length
$$
a \approx \frac{0.8853}{Z_1^{0.23} + Z_2^{0.23}} a_0
$$
with \f$a_0 \approx 0.0529\f$ nm the Bohr radius.

The function \f$ \Phi(x) \f$ satisfies
$$
\Phi(0) = 1, \quad \Phi(\infty)\to 0
$$

Different forms of the screening function have been proposed. The most widely used one is a sum of exponentials
$$
\Phi(x) = \sum_i {A_i \, e^{-b_ix}}, \quad \sum_i{A_i}=1
$$
OpenTRIM has several definitions for screening functions of this type and the user can select the preferred one by setting \ref _Simulation_screening_type "`Simulation.screening_type`" in the \ref json_config "configuration options".

## Scattering angle

The scattering of the incoming projectile ions of energy \f$E\f$ is elastic, and can be adequately described by classical kinematics.

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

## Cross-section and stopping power

The differential cross-section in the center-of-mass system is given by

\f[
  \frac{d\sigma}{d\Omega} = \frac{p}{\sin(\theta)}\left| \frac{dp}{d\theta}\right| = a^2 \frac{s}{\sin(\theta)}\left| \frac{ds}{d\theta}\right|
\f]

where \f$ d\Omega = 2 \pi \sin\theta d\theta \f$ is the element of solid angle. To calculate numerically the cross-section we need to
- invert the function \f$ \theta = \theta(\epsilon,s) \f$ to obtain  \f$ s = s(\epsilon,\theta) \f$
- evaluate numerically the derivative \f$ dp / d\theta = a^{-1} ds/d\theta\f$ 

Using the expression for the recoil energy \f$ T\f$ of the struck atom
\f[
  T = \gamma E \sin^2 \theta/2, \quad \gamma = 4m_1 m_2 / (m_1 + m_2)^2
\f]
and noting that
\f[
  \Omega(\theta) = 2\pi (1-\cos\theta) = 4\pi\sin^2\theta/2
\f]
we obtain the differential cross-section with respect to recoil energy: 
\f[
  \frac{d\sigma}{dT} = \frac{4\pi}{\gamma \, E} \frac{d\sigma}{d\Omega} 
\f]
and the nuclear stopping cross-section
\f[
  S_n(E) = \int_0^{\gamma \, E}{T\,d\sigma(E,T)} = 
  \frac{\gamma E}{4\pi} \int_0^{4\pi}{\Omega \frac{d\sigma}{d\Omega}\,d\Omega} =
  \gamma E\, \langle \sigma\cdot\Omega \rangle.
\f]
The stopping power is \f$ -dE/dx = N\,S_n(E) \f$, where \f$ N \f$ denotes the atomic density.

The *reduced* stopping cross-section is
\f[
  s_n(\epsilon) = \frac{\epsilon}{\pi a^2 \gamma E} S_n(E) = 
  \frac{\epsilon}{\pi a^2}\, \langle \sigma\Omega \rangle.
\f]

## Analytical results for the un-screened Coulomb potential

For the un-screened Coulomb potential the analytical expressions for the above quantities are (\f$ a=1\f$):

- Distance of closest approach
\f[
  x_0 = \frac{1}{2\epsilon} + \sqrt{\frac{1}{(2\epsilon)^2} + s^2}
\f]

- The closest approach for head-on collisions (s=0)
\f[
  b_0 = \frac{a}{\epsilon} = \frac{Z_1 Z_2 e^2}{E_{CM}}
\f]

- Scattering angle
\f[
  \sin^2 (\theta/2) = \frac{1}{1 + 4 \epsilon^2 s^2}
\f]

- Differential cross-section
\f[
  \frac{d\sigma}{d\Omega} = \frac{a^2}{16 \epsilon^2}\, \frac{1}{\sin^4 (\theta/2)} = \frac{b_0^2}{16} \frac{1}{\sin^4 (\theta/2)}
\f]
\f[
  \frac{d\sigma}{dT} = = \frac{\pi b_0^2}{4} \frac{\gamma E}{T^2}
\f]

The stopping cross-section diverges in this case. However, setting a mininum cutoff \f$ T_m \f$ in the recoil energy it is obtained that
\f[
  \sigma_0(T_m) = \frac{\pi b_0^2}{4} \left( \frac{\gamma E}{T} - 1\right)
\f]
\f[
  \langle T \rangle = T_m \log (\gamma E / T_m)
\f]

## OpenTRIM implementation

OpenTRIM includes a \ref XS "library of C++ classes" that can be utilized to perform the above calculations. 

Additionally, a set of \ref xs_corteo "pre-computed scattering tables" are provided for different screening functions. These are employed in the OpenTRIM simulations but can also be used independently as external libraries. 

\sa XS, xs_corteo
