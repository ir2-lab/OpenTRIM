# Damage Events {#damage-events}

## Damage parameters

Damage generation in OpenTRIM is modelled based on a set of energy parameters, as is done in SRIM and other Monte-Carlo and BCA codes. They are the **displacement energy** \f$E_d\f$, the **lattice binding energy** \f$E_l\f$ and the **recombination energy** \f$E_r\f$. Their meaning becomes apparent when we consider the sequence of steps that leads to the generation of atomic displacements.

\f$E_d\f$, \f$E_l\f$ and \f$E_r\f$ are characteristic of the material and have different values for each atomic species in the given material.

In the OpenTRIM JSON configuration, their values are set by parameters with the mnemonic codes \ref _Target_materials_0_composition_0_Ed "Ed", \ref _Target_materials_0_composition_0_El "El" and \ref _Target_materials_0_composition_0_Er "Er", respectively, for each element in the material composition definition. For example, for Fe the definition would be:
```javascript

"materials": [
            {
                "id": "Iron",
                "density": 7.8658,
                "composition": [
                    {
                        "element": {
                            "symbol": "Fe"
                        },
                        "X": 1,
                        "Ed": 40,
                        "El": 3,
                        "Er": 40
                    }
                ]
            }
        ]
```
See the examples in the \ref json_config section for more details.

## The displacement event

A displacement damage event occurs as follows:

- An incoming ion A with kinetic energy \f$E\f$ collides with a target atom B and transfers to it a recoil energy \f$T\f$. A emerges from the collision with energy \f$E' = E - T\f$.
- If \f$T\f$ is at or above B's displacement threshold, \f$T \geq E_d^B\f$: 
  - B is displaced from its lattice site and a lattice vacancy is created at the position of atom B. 
  - B starts moving as a new recoil ion with kinetic energy \f$T - E_l^B\f$, where \f$E_l^B\f$ is the lattice binding energy of atom B.
  - if \f$E' < E_r^B\f$ and A is the of the same atomic species as B, then A replaces B in its lattice site. This constitutes a *replacement event*: the recoil B is still emitted and followed, but **no vacancy and no interstitial are recorded**, since the site is immediately re-occupied. The track of A ends here and its remaining energy \f$E'\f$ is deposited as sub-threshold nuclear energy loss ("Phonons" in SRIM terminology).
  - Otherwise, A continues its track with energy \f$E'\f$
- If \f$T < E_d^B\f$, B is not displaced and \f$T\f$ is deposited as sub-threshold nuclear energy loss.

When an atom is displaced, a vacancy-interstitial pair, or Frenkel pair, is created. This defect has a certain formation energy, which is a characteristic of the material and of the species of the interstitial. This is essentially the lattice binding energy, \f$E_l\f$, which is termed this way for compatibility with the terminology of previous codes.
In principle, the Frenkel pair formation energy is the sum of the energies needed to form the vacancy and the interstitial, which need not be equal. However, for simplicity, OpenTRIM assigns a formation energy of \f$E_l/2\f$ to both the vacancy and the interstitial atom.

The value of \f$E_r\f$ is typically set equal to \f$E_d\f$.

Recoils are generated only if the simulation follows cascades, i.e., when \ref _Simulation_simulation_type "/Simulation/simulation_type" is `"FullCascade"` or `"CascadesOnly"`. With `"IonsOnly"` the recoil energy \f$T\f$ is deposited on the spot and only the projectile track is simulated.

## End of an ion track: interstitials and implantations

An ion track is terminated when the ion's kinetic energy drops below the cutoff \ref _Transport_min_energy "/Transport/min_energy" (default 1 eV), or when the ion is captured in a replacement event, or when it leaves the simulation volume.

An ion stopping inside the simulation volume comes to rest at an interstitial position:

- If it is a target recoil, it is recorded as an **interstitial**, completing the Frenkel pair whose vacancy was recorded at the recoil's original site.
- If it is a beam ion, it is recorded as an **implanted** atom.

Both are scored in the same tally table, `Implantations`, since they are physically the same kind of event: a foreign atom at rest in an interstitial site.

If instead the ion escapes through an external boundary of the simulation volume, no interstitial is recorded. For a recoil this means the vacancy it left behind remains unpaired, and the half Frenkel pair energy \f$E_l/2\f$ that had been assigned to the interstitial is released as nuclear energy loss. See \ref energy-partition "Energy partition" for the full energy bookkeeping.

## Intra-cascade recombination

Optionally, OpenTRIM can let the Frenkel pairs generated within a single PKA cascade recombine before they are tallied. This is enabled by the option \ref _Simulation_intra_cascade_recombination "/Simulation/intra_cascade_recombination" (disabled by default) and is still an **experimental** feature.

When active, at the end of each PKA cascade the vacancies and interstitials of that cascade are matched up: an interstitial recombines with a vacancy if

- the two are of the same atomic species, and
- their mutual distance \f$R\f$ is below the recombination radius \f$R_c\f$ of that species, set by \ref _Target_materials_0_composition_0_Rc "Rc" in the target options.

Recombined pairs are removed from the vacancy and interstitial counts and scored instead in the `Recombinations` table. 

## Damage event tallies

The damage events described above are scored per atomic species and per simulation cell in the `/tally/damage_events` group of the \ref out_file "HDF5 output archive":

| Table            | Description                                                             |
| ---------------- | ----------------------------------------------------------------------- |
| `Vacancies`      | Vacancies created by displacement events                                |
| `Implantations`  | Stopped recoils (interstitials) and stopped beam ions (implanted atoms) |
| `Replacements`   | Replacement events                                                      |
| `Recombinations` | Frenkel pairs removed by intra-cascade recombination                    |

All tallies are accumulated over the simulated histories and reported per source ion, together with their statistical uncertainty. See \ref tallies for details.

## Primary knock-on atoms (PKAs)

PKAs are the atoms of the target that are first hit by an ion of the external beam, obtaining energy above the displacement threshold and thus generating a recoil cascade. In a `"CascadesOnly"` simulation there is no beam and the source ions themselves are the PKAs.

OpenTRIM pays special attention to PKA events and keeps separate records of the damage and energy deposition within each PKA cascade. These are reported in the `/tally/pka_damage` group of the \ref out_file "HDF5 output archive":

| Table        | Description                                                                    |
| ------------ | ------------------------------------------------------------------------------ |
| `Pka`        | Number of PKAs                                                                 |
| `Pka_energy` | PKA recoil energy \f$T\f$ [eV], before subtraction of \f$E_l\f$                |
| `Tdam`       | Damage energy, i.e., the part of the PKA energy imparted to atomic motion [eV] |
| `Tdam_LSS`   | Damage energy estimated by the LSS approximation [eV]                          |
| `Vnrt`       | Vacancies per the NRT model using `Tdam`                                       |
| `Vnrt_LSS`   | Vacancies per the NRT model using `Tdam_LSS`                                   |

Since these are tallies, the stored values are sums over all PKAs divided by the number of source ions. Thus, `Pka` gives the number of PKAs per ion, while the mean PKA energy is obtained as the ratio `Pka_energy`/`Pka`, and similarly for the other quantities.

The damage energy \f$T_{dam}\f$ is obtained directly from the Monte-Carlo simulation as the PKA recoil energy minus all electronic energy losses incurred in the cascade:

\f[
T_{dam} = T - \sum_{\rm cascade}{\Delta E_e}
\f]

where the sum runs over the PKA and all of its secondary recoils. It thus includes both the energy stored in the generated defects and the sub-threshold nuclear energy loss.

## NRT implementation

To aid in comparisons to standard damage calculations or to neutron irradiation, OpenTRIM employs the standard Norgett-Robinson-Torrens (NRT) model to give an independent estimation of the generated defects. The number of vacancies produced by a recoil of damage energy \f$T_{dam}\f$ is

$$
\nu_{NRT}(T_{dam}) = \begin{cases}
0, & T_{dam} < E_d \\
1, & E_d \leq T_{dam} < 2.5\,E_d \\
\frac{0.8\, T_{dam}}{2\, E_d}, & T_{dam} \geq 2.5\,E_d
\end{cases}
$$

The model is applied to two different estimates of the damage energy:

- The damage energy of the PKA cascade as found by the Monte-Carlo simulation. This is reported in the tally table `/tally/pka_damage/Vnrt`.
- The damage energy estimated by the LSS partition. This is reported in `/tally/pka_damage/Vnrt_LSS` and can be used for comparison to other codes, e.g., SRIM.

The LSS damage energy is obtained from the analytical partition function of Lindhard et al., in the form given by Robinson,

\f[
T_{dam}^{LSS} = \frac{T}{1 + k_d\, g(\varepsilon)}, \qquad
g(\varepsilon) = \varepsilon + 3.4008\,\varepsilon^{1/6} + 0.40244\,\varepsilon^{3/4}
\f]

with the reduced energy \f$\varepsilon = 0.01014\, Z^{-7/3}\, T\f$ and \f$k_d = 0.1334\, Z^{2/3}\, M^{-1/2}\f$, where \f$T\f$ is the PKA recoil energy in [eV] and \f$Z\f$, \f$M\f$ the atomic number and mass of the recoil atom. Note that this is the self-ion approximation, i.e., the recoil is assumed to move in a target of its own species.

In multi-elemental targets, the application of NRT is not well defined. The option \ref _Simulation_nrt_calculation "/Simulation/nrt_calculation" selects among two alternatives:

- **"NRT_element"** (default): the model is implemented using the parameters (\f$E_d\f$, \f$Z\f$, \f$M\f$) of the specific PKA recoil atom.
- **"NRT_average"**: the model is implemented using material average values, with the effective displacement threshold defined according to Ghoniem and Chou JNM1988,
  \f[
  \bar{E}_d^{-1} = \sum_i {X_i\, E_{d,i}^{-1}}
  \f]
  where \f$X_i\f$ is the atomic fraction of species \f$i\f$. The mean \f$Z\f$ and \f$M\f$ of the material are used in the LSS partition.
