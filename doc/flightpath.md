# Flight Path Selection {#flightpath}

Here we discuss briefly the techniques of flight path selection in Monte-Carlo ion transport simulations. For a more detailed discussion of the specific implementation in OpenTRIM see \subpage flightpathdetails.

In MC particle transport simulations, the distance to the next collision is typically sampled from the exponential distribution, \f$p(x) = N\sigma_0 e^{-N\sigma_0 x}=\ell^{-1}e^{-x/\ell}\f$, where \f$N\f$ is the atomic density of scattering centers, \f$\sigma_0\f$ the total cross-section and \f$\ell = (N\sigma_0)^{-1}\f$ denotes the mean free path (mfp).

There is a difficulty in employing this type of flight path selection for charged particles interacting via the Coulomb and screened Coulomb interactions, because the total cross-section diverges due to the long range of the potentials. Thus, a particle can in-principle have infinitely many interactions along its trajectory and this is a problem for computer simulation. There are mainly two ways to circumvent this problem:
- Set a fixed flight path, typically of the order of the interatomic distance 
- Set a cutoff to the interaction, either w.r.t. the scattering angle or the recoil energy or both. Scattering events below the cutoff will be ignored. 

In both cases the conditions are selected so that the results of the calculation are not affected significantly.

Mendenhall & Weller (2005) set a lower cutoff for the recoil energy, \f$T_c\f$, which corresponds to the lowest energy transfer of interest in the problem under study. They suggest a value in the range of 1 - 10 eV for ion penetration in solids. 

\f$T_c\f$ corresponds to a lower bound \f$\theta_c\f$ for the center-of-mass scattering angle, which can be obtained from 
\f[
T_c = T_m \sin^2(\theta_c/2)
\f]
For a given reduced energy \f$\epsilon\f$ of the particle, we can find the maximum impact parameter, \f$p_{max}\f$, corresponding to \f$\theta_c\f$ and thus define the effective total cross-section
\f[
\sigma_0 = \pi\, p_{max}^2(\epsilon, \theta_c) = 
\int_0^{p_{max}}{2\pi\, p\, dp} =
\int_{\theta_c}^{\pi}{d\sigma(\epsilon,\theta)}
\f]

As the particle slows down, the cross-section increases and eventually \f$p_{max}\f$ becomes larger than the interatomic distance. To avoid this, a hard upper bound is set on \f$p_{max}\f$ at about half the nearest-neighbour distance in the solid. This effectively truncates the potential at this distance.    

The main steps of the Mendenhall-Weller algorithm are as follows

> **Mendenhall-Weller (MHW) Algorithm:**
> - For a given incoming ion energy \f$E\f$ find the values of \f$p_{max}\f$, \f$\sigma_0\f$ and \f$\ell\f$ corresponding to the recoil energy cutoff \f$T_c\f$
> - Generate a random number sample \f$u \in (0,1)\f$
> - If \f$p_{max} < L/2\f$, where \f$L\f$ is the interatomic distance, 
>    - Set \f$p = p_{max} \sqrt{-\log u}\f$
>    - Propagate the particle by \f$\ell\f$
>    - If \f$p<p_{max}\f$ generate a scattering event  
> - If \f$p_{max} \geq L/2\f$ then 
>    - Set \f$p = (L/2)\sqrt{u}\f$
>    - Propagate particle by \f$\sim L\f$ 
>    - Generate a scattering event

This is essentially very similar to the algorithm employed by the popular SRIM code.

To save computation time, the values of \f$p_{max}\f$, \f$\sigma_0\f$ and \f$\ell\f$ can be pre-calculated on a coarse energy grid.

The main characteristics of this algorithm are:
1. For \f$p_{max} < L/2\f$, the path length is fixed and the impact parameter is sampled from \f$e^{-N\ell\pi p^2}p\, dp\f$. This is more efficient than the standard procedure, which requires random sampling of both the path and the impact parameter. 

2. For \f$p_{max} \geq L/2\f$, there is an absolute upper bound to \f$p_{max}\f$ at half the interatomic distance and \f$p\f$ is sampled from \f$p\, dp\f$. This is implemented in both SRIM and MHW2005. MHW justify it as follows:

> ... impact parameters larger than half the lattice spacing do not occur, since then one is closer to the adjacent atom.

while Biersack1980 and the SRIM manual (Ch.7) write:

> This procedure maintains the atomic density in the target without correlating the lateral positions of successive target atoms (neglection of lattice structure).

We believe that these comments are not entirely correct. The ion can in principle have collisions with \f$p_{max} > L/2\f$. There is nothing in the employed binary-collision physics that prevent it. And we do not see how this would change the atomic density or introduce correlations.

The point is that, as the ion's energy drops below about 1 keV, the simple picture of binary collisions breaks down since the ion interacts collectively with the solid. To describe this process we would have to use a completely different simulation technique called molecular dynamics (MD). From this viewpoint, the truncation \f$p_{max}<L/2\f$ means essentially that, as the physics that we employ to simulate the process becomes out of context, for the sake of computing efficiency, we dispense with the details and make a crude approximation to quickly finish the ion track.

On the other hand, some of the effects we are interested in, e.g., the generation of defects by atomic displacements, occur in this low energy region. Thus, maybe it is beneficial to make a more detailed simulation, albeit within the physics that we employ.

Thus, OpenTRIM employs also a more "standard" algorithm for path and impact parameter selection, termed "FullMC", that is less focused on computing efficiency. It turns out the penalty is not so severe. The main steps are shown below:

> **FullMC path selection algorithm**
> - For a given incoming ion of energy \f$E\f$ find the values of \f$p_{max}\f$, \f$\sigma_0\f$ and \f$\ell\f$ 
> - Additionally, compute a maximum path-length \f$\Delta x_{max}\f$, such that the relative electronic energy loss \f$\Delta E_e/E = (1/E)\int_0^{\Delta x_{max}} {(dE/dx)_e dx}\f$ is below a small value \f$\delta \sim 0.01-0.05\f$. 
> - Take a random sample \f$u_{1} \in (0,1)\f$
> - If \f$u_1 < e^{-\Delta x_{max}/\ell}\f$, reject the scattering event and propagate the particle by \f$\Delta x_{max}\f$
> - Otherwise:
>   - compute the path to the next collision \f$\Delta x = -\ell\,\log u_1\f$ 
>   - compute the impact parameter as \f$p = p_{max}\sqrt{u_2}\f$, where \f$u_{2} \in (0,1)\f$ is a 2nd random sample 
>   - Propagate the particle by \f$\Delta x\f$ and make a collision

See \ref flightpathdetails for a more detailed description of the employed algorithms.


