# OpenTRIM + Intra-cascade recombination (ICR)

4/2/2025

## Implementation

- A Frenkel pair is unstable and thus recombines if the I-V distance is below a capture radius $R_c$
- The I-V capture radius $R_c$ is introduced as a parameter/property of each atomic species in a target material
- In the ion transport simulation, ICR is implemented as follows: 
  - All I & V generated in a PKA cascade are stored in a time ordered list 
  - After the cascade finishes, the list is traversed from the start and each defect may recombine with the closest preceding anti-defect within its capture radius.
  
The OCTAVE script `run_all.m` runs a number of `opentrim` tests of various experimental options regarding inter-cascade recombination. It can be executed on the command line or in OCTAVE's environment.

The scripts `icr1.m`, `icr2.m` and `icr3.m` analyze the output and create the figures below.

## Test case: 2 MeV Fe on Fe

- For Fe a typical value for $R_c$ is 0.80 nm = $2.8 a_0$. It has been reported that with this value similar defect production is obtained for MD and BCA

In the following, electronic energy loss is switched off in order to observe only the effects of nuclear scattering.

![](icr1.png)

From the above figure it is seen that
- Without ICR there is good agreement with the NRT model. This is independent of the value of $E_d$. 
- With ICR and $E_d=40$:
  - For $T_d \gtrsim 100\, E_d$ the number of generated defects falls to $\sim 35\%$ of the NRT, in agreement with arc-dpa / MD
  - At low $T_d\lesssim 5\, E_d$, $N_d < 1$, i.e., most of the defects generated at low $T_d$ recombine. This contradicts the basic assumption of damage models that recoils with $E_d<T_d<2 E_d/0.8$ result in the production of one stable FP 
- Lowering $E_d$ to 10 eV:
  - Results in much higher recombination. A high $T_d$, only about 15% of defects survive. 

## Disabling of correlated recombination

To address the behavior of $N_d$ at low damage energy we tried disabling the recombination of I-V from the same FP, i.e., of correlated defects.

This results in the following behavior

![](icr2.png)

For $T_d\lesssim 100$ eV the number of defects is indeed 1. 

However, for slightly higher energies, $N_d$ goes initially below 1 and then starts to grow again. This is unphysical.

The number of defects at $T_d>1$ keV is slightly higher. This is due to the non-recombination of correlated I-Vs. They correspond to about 10% of the total number of recombinations at high $T_d$.

## Generating FPs with initial separation between I-V

Another idea was to generate FPs with an initial separation between V and I equal to $R_c$.

Thus, in a collision with recoil energy $T>E_d$ the recoiling ion is positioned initially at a distance $R_c$ from the point of impact, where the vacancy is created. The initial kinetic energy of the recoiling atom is $T - E_{FP} - R_c\,S_e(T)$, where $E_{FP}$ is the FP formation energy (or lattice binding energy) and $S_e$ the electronic stopping power. 

Another variation of this is to produce the FP with inital distance $R_c$ and subtract $E_d$ from the kinetic energy of the recoiling ion. Thus, if the recoil is generated with initial energy $T=E_d$ then the recoilling atom is implanted at a distance $R_c$ with zero kinetic energy.

The following graph shows the effect of these options on recombination.

![](icr3.png)

It is seen that setting the recoil at a distance $R_c$ makes the low energy cascades stable and thus $N_d\approx 1$ in agreement with the NRT model.  

However, at high $T_d$ there is much less recombination than predicted by arc-dpa ($\sim 30\%$ of NRT). The remaining stable defects are roughly at 75% of NRT.

Switching on the 2nd option, i.e., to subtract $E_d$ from the recoil energy, reduced the stable defects to $\sim 60\%$.

It is difficult to reconcile these results with the those of the previous paragraph, which show that correlated recombinations represent only 10% of the total recombinations happening in a high energy cascade. One would anticipate that creating I-V pairs at initial distance equal to $R_c$ would primarily affect only those correlated recombinations. However, the above figure proves that this is not correct. The matter needs more study.
