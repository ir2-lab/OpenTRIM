# Flight Path Selection {#flightpath}

Here we discuss briefly the techniques of flight path selection in Monte-Carlo ion transport simulations. For a more detailed discussion of the specific implementation in OpenTRIM see \subpage flightpathdetails.

In particle transport simulations, the distance to the next collision is typically sampled from the exponential distribution, \f$p(x) = N\sigma_0 e^{-N\sigma_0 x}=\ell^{-1}e^{-x/\ell}\f$, where \f$N\f$ is the volume density of scattering centers, \f$\sigma_0\f$ the total cross-section and \f$\ell = (N\sigma_0)^{-1}\f$ denotes the mean free path (mfp). It is assumed that the scattering centers are randomly distributed inside the simulation volume.

There is a difficulty in employing this type of flight path selection for charged particles interacting via the Coulomb and screened Coulomb interactions, because the total cross-section diverges due to the long range of the potentials. Thus, a particle can in-principle have infinitely many interactions along its trajectory and this is a problem for computer simulation. There are mainly two ways to circumvent this problem:
- Set a fixed flight path, small enough so that the particle has a sufficiently high number of collisions   
- Set a cutoff to the interaction, either w.r.t. the scattering angle or the recoil energy or both. Scattering events below the cutoff are ignored. The mean free path is variable, i.e., it depends on the projectile energy. 

In both cases the conditions are selected so that the results of the calculation are not affected significantly.

## Fixed flight path

In this case, the flight path between collisions is a constant, \f$\ell_0\f$, typically of the order of the interatomic distance, e.g., \f$\ell_0 \sim R_{at}\f$, where \f$R_{at} = (4\pi N/3)^{-1/3}\f$ is the atomic radius. Thus, a simulated particle has numerous collisions as it traverses though matter - of the order of \f$10^8\f$ per cm of flight path. 

The corresponding effective total cross-section is \f$\sigma_0 = (N\ell_0)^{-1} = (N R_{at})^{-1} \sim 1.33 \pi R_{at}^2 \f$, i.e., approximately equal to a circular area of radius \f$R_{at}\f$. Thus, the interaction potential is effectively truncated at a distance \f$r\sim R_{at}\f$. This is considered an adequate approximation, as the ion will at any time interact with the target atom closest to its vicinity, which is within \f$r<R_{at}\f$, and not with more distant atoms with \f$r>R_{at}\f$. 

The simulation proceeds as follows:

>
> **Fixed Flight Path algorithm**
> - Propagate the projectile by a distance \f$\ell_0\sim R_{at}\f$ along its trajectory. 
> - Sample the impact parameter \f$p\f$ uniformly on a circle of radius \f$p_{max} =  (N\pi\ell_0)^{-1/2}\f$. \n
>   This can be achieved by setting \f$p=p_{max}\sqrt{u}\f$, where \f$u\f$ is a random value distributed uniformly in \f$[0,1)\f$.
> - Calculate the scattering angle \f$\theta=\theta(E,p)\f$ and recoil energy \f$T=T(E,\theta)\f$
> - Adjust the projectile energy and direction
> - Repeat
>

The fixed flight path method achieves a rather accurate description of the ion trajectory if a sufficiently small \f$\ell_0\f$ is employed. 

The SRIM user manual suggests the use of this method for most accurate results. This is called "monolayer mode" in the terminology of this program. 

However, the efficiency of the method is rather low, as a high number of low energy scattering events are included, which do not contribute significantly to the final outcome. 

## Variable flight path

To improve the efficiency of the simulation without sacrificing accuracy, one can selectively reject scattering events of low importance. 

This can be achieved by setting, e.g., a recoil energy cutoff, \f$T_c\f$, corresponding to the lowest energy transfer of interest in the problem under study. Mendenhall & Weller (2005) suggest a value in the range of 1 - 10 eV for ion penetration in solids. 

\f$T_c\f$ corresponds to a lower bound \f$\theta_c\f$ for the center-of-mass scattering angle, which can be obtained from 
\f[
T_c = \gamma E \sin^2(\theta_c/2)\quad \Rightarrow \quad \theta_c = 2 \arcsin\sqrt{T_c/\gamma E},
\f]
where \f$\gamma = 4m_1 m_2 /(m_1+m_2)^2\f$. The corresponding upper bound to the impact parameter, \f$p_{max}\f$, can be found by solving 
\f[
\theta_c = \theta(E,p_{max}) 
\f]
where \f$\theta(E,p)\f$ is the scattering integral of the potential. Having obtained \f$p_{max}\f$, we can now define the effective total cross-section \f$\sigma_0=\pi p_{max}^2\f$ and the mean free path \f$\ell=(N\sigma_0)^{-1}\f$.

This procedure can be extended to include a lower cut-off for the lab system scattering angle \f$\Theta\f$. Noting that for small scattering angles 
\f[
  \Theta \approx \theta / (1 + m_1/m_2)\quad \Rightarrow \quad  T=\gamma E \sin^2(\theta/2) \approx (m_1/m_2) E \Theta^2,
\f]
we can define a combined recoil energy cut-off, \f$T'_c\f$ 
\f[
T'_c = \min \left\{ T_c, \; (m_1/m_2) E \, \Theta_c^2 \right\}.
\f]
Thus, we reject scattering events only when both the recoil energy and the scattering angle are below their respective cut-offs: 
\f[
  T<T_c \; \land \; \Theta<\Theta_c
\f]

Having defined an effective total cross-section, we can now employ the standard algorithm for sampling the flight path and impact parameter:
>
> **Variable Flight Path algorithm**
> - For given projectile energy \f$E\f$ and recoil cut-off \f$T'_c\f$ calculate the maximum impact parameter \f$p_{max}=p_{max}(E,T'_c)\f$ and the mean free path \f$\ell = (N\pi p_{max}^2)^{-1/2}\f$
> - Sample the flight path \f$x\f$ to the next collision from \f$\ell^{-1}e^{-x / \ell}\f$. \n
  This is achieved by setting \f$x=-\ell \log u_1\f$, where \f$u_1\f$ is a random number uniformly distributed in \f$(0,1)\f$.
> - Sample the impact parameter uniformly on a circle of radius \f$p_{max}\f$:     
  \f$p = p_{max}\sqrt{u_2}\f$, where \f$u_2\in(0,1)\f$ is again a uniform random number.
> - Calculate the scattering angle \f$\theta=\theta(E,p)\f$ and recoil energy \f$T=T(E,\theta)\f$
> - Adjust the projectile energy and direction
> - Repeat
>

This procedure may seem more complicated than the fixed flight path algorithm, however it brings a huge gain in efficiency, since many low importance events are rejected. This is most evident at high projectile energy and light ions. 

The values of the energy dependent \f$p_{max}\f$ and \f$\ell\f$ are not needed
with high accuracy. Thus, they can be precomputed on a coarse energy grid.

As the particle slows down, the cross-section increases and eventually \f$\ell\f$ becomes comparable to the interatomic distance. In this limit, one can assume that the random nature of the flight path is unimportant and the trajectory of the ion can be simulated to sufficient accuracy by the constant flight path algorithm. 

## Other algorithms

A variation of the variable flight path algorithm is the following, which is suggested by many authors (Biersack1980, SRIM book, MHW2005):

> **Modified Variable Flight Path algorithm**
> - Calculate \f$p_{max}=p_{max}(E,T'_c)\f$ and \f$\ell = (N\pi p_{max}^2)^{-1/2}\f$ as above
> - Take a uniform random sample \f$u\in [0,1)\f$
> - If \f$p_{max} < L\f$,  where \f$L\sim R_{at}\f$
>    - Set \f$p = p_{max} \sqrt{-\log u}\f$
>    - Propagate the particle by \f$\ell\f$
>    - If \f$p<p_{max}\f$ generate a scattering event  
> - If \f$p_{max} \geq L\f$ then 
>    - Set \f$p = L\sqrt{u}\f$
>    - Propagate particle by \f$(N\pi L^2)^{-1/2}\f$ 
>    - Generate a scattering event

The main characteristics of this algorithm are:
- For small \f$p_{max} < L\f$, the path length is \f$\ell\f$ and the impact parameter is sampled from \f$e^{-N\ell\pi p^2}p\, dp\f$. This is more efficient than the standard algorithm, which requires random sampling of both the path and the impact parameter. 

- For \f$p_{max} \geq L\f$, the algorithm reverts to the constant flight path mode

Despite the higher efficiency, there are also some drawbacks:
- There is a discontinuous change in behaviour at \f$p_{max}=L\f$, as also noted in the SRIM book.
- for \f$p_{max}>L\f$ the resulting average distance between collisions, \f$\langle x\rangle\f$, is larger than the mean free path \f$\ell\f$ because some scattering events are rejected. It can be seen that \f$\langle x\rangle = (1-e^{-1})^{-1}\ell \approx 1.6 \ell\f$. Thus, the simulated projectile trajectory is not consistent with the anticipated mean free path.

For these reasons, the above algorithm has not been included in OpenTRIM.

Another point that has been discussed by several authors is the treatment of the low energy region.

It is generally accepted that, as the projectile energy drops below about 1 keV, the simple picture of binary collisions breaks down since the ion interacts collectively with the solid. To describe this process we would have to use a completely different simulation technique called molecular dynamics (MD). From this viewpoint, the truncation of the potential at \f$r \leq R_{at}\f$ in the constant flight path mode means essentially that, as the physics that we employ to simulate the process becomes out of context, for the sake of computing efficiency, we dispense with the details and make a crude approximation to quickly finish the ion track.

On the other hand, some of the effects we are interested in, e.g., the generation of defects by atomic displacements, occur in this low energy region. Some authors have included the effect of low energy collisions by extending to \f$p>R_{at}\f$, as, e.g., described in Eckstein1991.

In OpenTRIM this is possible within the variable flight path mode simply by lowering the recoil cut-off \f$T_c\f$ and lifting the lower bound to \f$\ell\f$. 

See \ref flightpathdetails for a more detailed description of the fligh path algorithms as employed in OpenTRIM.