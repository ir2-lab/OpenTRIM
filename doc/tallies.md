# Tallies & events {#tallies}

The results of an OpenTRIM simulation are stored in the \ref out_file "HDF5 output archive" in two distinct forms:

- **Tallies** (`/tally` and `/user_tally`): histograms of scored quantities, accumulated over all simulated ion histories and reported as mean values per source ion, together with their statistical uncertainty.
- **Event tables** (`/events`): raw, event-by-event listings of selected Monte-Carlo events, with one row per event.

Tallies are the compact, statistically meaningful output and are always present. Event tables are optional, potentially very large, and intended for post-processing that cannot be expressed as a fixed histogram.

## Monte-Carlo tallies {#tallies-mc}

A tally in the Monte-Carlo method is a score table where the occurrences of certain events are counted (tallied). In the end, we divide the score by the total number of trials to obtain the probability of an event. In the present case of ion simulation, a trial is equivalent to a complete ion history.

Each element in the tally tables is a mean value over all histories. Specifically, if \f$x_i\f$ is the contribution to the table entry \f$x\f$ from the \f$i\f$-th ion history, then the tally table will contain the mean value
\f[
\bar{x} = \frac{1}{N_h} \sum_i {x_i}
\f]

For each tally table, a second table of equal dimensions is stored, which holds the standard error of the mean (SEM). This has the same name plus the ending `_sem`. E.g., for the table `/tally/damage_events/Vacancies` there is also `/tally/damage_events/Vacancies_sem`. The SEM of a tally table entry, \f$\sigma_{\bar{x}}\f$, is calculated as
\f[
\sigma_{\bar{x}}^2 = \frac{1}{N_h(N_h-1)} \sum_i { (x_i - \bar{x})^2 }=
\frac{\bar{{x^2}} - \bar{x}^2}{N_h-1}
\f]

The number of histories \f$N_h\f$ is stored in `/run_info/total_ion_count`. Multiplying a tally by \f$N_h\f$ recovers the absolute accumulated score.

Scores are accumulated internally in double precision and, in multi-threaded runs, each thread keeps its own tally which is merged into the global one under a mutex. The per-history contributions \f$x_i\f$ are therefore independent of the number of threads, and so are \f$\bar{x}\f$ and \f$\sigma_{\bar{x}}\f$ up to floating-point round-off. See \ref reproducibility for details.

@note The SEM is only meaningful for \f$N_h > 1\f$; for a single history all `_sem` tables are zero.

@note Ratios of tallies, e.g. the mean PKA energy `Pka_energy`/`Pka`, are legitimate estimators but their uncertainty is **not** the ratio of the stored SEMs, because numerator and denominator are correlated. A conservative estimate can be obtained with the standard formula for the error of a ratio of correlated variables, or by splitting the run into independent batches.

## Standard tallies {#tallies-standard}

OpenTRIM stores 17 standard tally tables for various quantities of interest, plus a table of totals. They are saved under the path `/tally` and are divided in 4 groups:

| Group                     | Tables | Documented in |
| ------------------------- | ------ | ------------- |
| `/tally/damage_events`    | 4      | \ref damage-events "Damage events" |
| `/tally/energy_deposition`| 4      | \ref energy-partition "Energy partition" |
| `/tally/pka_damage`       | 6      | \ref damage-events "Damage events" |
| `/tally/ion_stat`         | 3      | below |

The complete list of tables is the following:

| Path                                   | Unit   | Description |
| -------------------------------------- | ------ | ----------- |
| `damage_events/Vacancies`              | –      | Vacancies created by displacement events |
| `damage_events/Implantations`          | –      | Stopped recoils (interstitials) and stopped beam ions (implanted atoms) |
| `damage_events/Replacements`           | –      | Replacement events |
| `damage_events/Recombinations`         | –      | Frenkel pairs removed by intra-cascade recombination |
| `energy_deposition/Electronic`         | eV     | Electronic energy loss |
| `energy_deposition/Nuclear`            | eV     | Sub-threshold nuclear energy loss |
| `energy_deposition/Stored`             | eV     | Energy stored in lattice defects (Frenkel pairs) |
| `energy_deposition/Lost`               | eV     | Kinetic energy carried out of the simulation volume by escaping ions |
| `pka_damage/Pka`                       | –      | Primary knock-on atoms (PKAs) |
| `pka_damage/Pka_energy`                | eV     | PKA recoil energy |
| `pka_damage/Tdam`                      | eV     | Damage energy |
| `pka_damage/Tdam_LSS`                  | eV     | Damage energy estimated by the LSS approximation |
| `pka_damage/Vnrt`                      | –      | Vacancies per the NRT model using `Tdam` |
| `pka_damage/Vnrt_LSS`                  | –      | Vacancies per the NRT model using `Tdam_LSS` |
| `ion_stat/Collisions`                  | –      | Number of ion collisions |
| `ion_stat/Flight_path`                 | nm     | Total ion flight path |
| `ion_stat/Lost`                        | –      | Number of ions that exited the simulation volume |

@note There are two tables named `Lost`: `energy_deposition/Lost` is an **energy** in eV, while `ion_stat/Lost` is a **count** of escaping ions.

All the above tables store information per atom type and per simulation cell. They have dimensions \f$[N_{at} \times N_{x} \times N_{y} \times N_{z}]\f$, where \f$N_{at}\f$ counts all the different atoms present in the problem including the projectile and \f$N_{x,y,z}\f$ are the number of cells along the 3 axes.

### Atom and cell indexing {#tallies-indexing}

Each table is a 4-dimensional array \f$A_k[i, i_x, i_y, i_z]\f$: the score of the \f$k\f$-th quantity generated by the atom with id \f$i\f$ in the cell \f$(i_x, i_y, i_z)\f$. E.g., a vacancy of the atom with id=2 created in cell (10,0,0) adds +1 to \f$A_{V}[2,10,0,0]\f$. The three spatial indices are independent axes of the dataset, so a depth profile, a slice or a projection is obtained by indexing or summing along them directly.

#### Atom id {#tallies-indexing-atom}

The atom id is **not** defined by the atomic species alone: the same chemical element appearing in two different materials counts as two different atoms. E.g., we may have Fe projectiles (id=0) impinging on a target containing Fe<sub>2</sub>O<sub>3</sub> (Fe with id=1, O with id=2) and bulk Fe (Fe with id=3). Beam ions always have id=0 and target atoms have consecutive ids starting from 1. The atom labels, in the order of their ids, are listed in `/target/atoms/label`, in the form `[symbol] in [material]`, or `[symbol] ion` for the projectile.

For some tables the projectile (\f$i=0\f$) does not have a meaningful contribution; e.g., vacancies are always assigned to one of the target atoms, never to the projectile. In this case the 0-th slice of the respective table is all zeros. The same is true of every table in a `"CascadesOnly"` simulation, where the source ions are drawn from the target atoms and nothing is ever scored under id 0.

#### Cell geometry {#tallies-indexing-cell}

The cell boundaries along the three axes are given in `/target/grid/X`, `/target/grid/Y` and `/target/grid/Z`, which have \f$N_x+1\f$, \f$N_y+1\f$ and \f$N_z+1\f$ values, respectively. The centre coordinates of every cell are also stored directly, as `/target/grid/cell_xyz`, an array of size \f$[3 \times N_c]\f$ with \f$N_c = N_x N_y N_z\f$.

`cell_xyz` is indexed by the **flat cell id**, which OpenTRIM forms in row-major (C) order,
\f[
j = (i_x\, N_y + i_y)\, N_z + i_z
\f]
i.e. the natural flattening of the last three dimensions of a tally table. This same id is what the `cid` column of the \ref tallies-events-exit "exit event table" contains, and what \ref grid3D::cellid() returns internally. Tally data does not need it — index the 4-D array instead — but it is needed to attach a position to a cell id coming from elsewhere.

@note Tallies are scored **per cell, not per unit volume**. To obtain densities, divide by the cell volume, which is uniform and equal to \f$\Delta x\, \Delta y\, \Delta z\f$ from the grid arrays.

### Where scoring happens {#tallies-scoring}

The standard \ref tally object is updated at the following \ref Event "events":

| Event              | Scored quantities |
| ------------------ | ----------------- |
| `BoundaryCrossing` | `Collisions`, `Flight_path`, `Nuclear`, `Electronic` accumulated since the last event, credited to the cell the ion is leaving |
| `Replacement`      | `Replacements` +1, plus the accumulated path, collision and energy-loss counters |
| `Interstitial`     | `Implantations` +1, `Stored` += \f$E_l/2\f$ (recoils only), plus the accumulated counters; the ion's remaining kinetic energy goes to `Nuclear` |
| `Vacancy`          | `Vacancies` +1, `Stored` += \f$E_l/2\f$ |
| `IonExit`          | `ion_stat/Lost` +1, `energy_deposition/Lost` += remaining kinetic energy, plus the accumulated counters; for recoils the unpaired \f$E_l/2\f$ is released to `Nuclear` |
| `CascadeComplete`  | `Pka` +1, `Pka_energy`, `Tdam`, `Tdam_LSS`, `Vnrt`, `Vnrt_LSS`, `Recombinations` |

Because path, collision and energy-loss counters are flushed at cell boundaries and at the end of a track, these quantities are attributed to the cell in which they were actually accumulated.

`Vacancy` and `Interstitial` events are not emitted at the moment the defect is created: defects are queued and released at the end of the PKA cascade, after the optional \ref damage-events "intra-cascade recombination" step. Recombined pairs are therefore never scored in `Vacancies` or `Implantations`; they appear only in `Recombinations`.

@note In an `"IonsOnly"` simulation recoils are not followed. `Pka` and `Pka_energy` are still scored, but `Tdam` and `Vnrt` are zero, and only the LSS estimates `Tdam_LSS` and `Vnrt_LSS` are meaningful. The `damage_events` tables contain only the beam ion implantations.

### The table of totals {#tallies-totals}

A table stored in `/tally/totals/data` holds the totals of all standard tallies summed over all cells. It has dimensions \f$[N_{tally} \times N_{at}]\f$ with \f$N_{tally} = 18\f$, i.e., each element is the total score of a given quantity for a given atom. The accompanying `/tally/totals/data_sem` holds the corresponding SEM.

The names labelling the first dimension are stored as \f$N_{tally}\f$ strings in `/tally/totals/column_names`:

    Histories, Vacancies, Implantations, Replacements, Recombinations,
    Electronic, Nuclear, Stored, Lost, Pka, Pka_energy, Tdam, Tdam_LSS,
    Vnrt, Vnrt_LSS, Flight_path, Collisions, Lost

Row 0 (`Histories`) is a bookkeeping row: element (0,0) equals 1, i.e., one history per history, and the rest of the row is zero. Rows 1 to 17 are the cell sums of the corresponding tally tables, in the order given above. Note that the order of the rows is the order of the \ref tally::tally_t enum, which differs from the alphabetical grouping used for the dataset paths.

\see tally, tally::tally_t, Event

## User tallies {#tallies-user}

In addition to the standard tables, OpenTRIM lets the user define arbitrary tallies which score a chosen simulation event into a multi-dimensional histogram of chosen binning variables. This covers quantities the standard tables do not provide, such as angular or radial distributions, energy spectra of escaping ions, or PKA energy spectra resolved by species.

User tallies are defined in the `UserTally` section of the \ref json_config "JSON configuration", which is an array of tally definitions. Each definition requires:

- \ref _UserTally_0_id "id" – a unique name for the tally; it becomes the HDF5 group name, so a short name without spaces is preferable.
- \ref _UserTally_0_description "description" – an optional one-line explanation, stored alongside the data.
- \ref _UserTally_0_event "event" – the simulation event that triggers a score.
- \ref _UserTally_0_coordinate_system "coordinate_system" – an optional local frame in which positions and directions are expressed.
- \ref _UserTally_0_bins "bins" – the bin edges of one or more binning variables.

Each time the selected event occurs, the tally evaluates all of its binning variables for the ion that caused the event, locates the corresponding bin and adds +1 to it. As with the standard tallies, the accumulated counts are divided by the number of histories on output, so the stored values are **events per source ion**, and a `data_sem` table gives the SEM.

### Trigger events {#tallies-user-events}

| `event`            | Triggered when |
| ------------------ | -------------- |
| `IonStop`          | The ion stops inside the simulation volume (energy below \ref _Transport_min_energy "min_energy" or captured in a replacement) |
| `IonExit`          | The ion exits the simulation volume |
| `Vacancy`          | A lattice vacancy is created |
| `Replacement`      | A replacement event occurs |
| `CascadeComplete`  | A PKA cascade is completed |
| `BoundaryCrossing` | An ion crosses an internal cell boundary |

Each user tally scores exactly one event type.

@note `IonStop` fires for every ion that comes to rest, including those that are subsequently removed by intra-cascade recombination. `Vacancy` fires at the end of the cascade, after recombination. The two therefore give the number of Frenkel pairs before and after recombination — see the `Nfp_0` / `Nfp_1` pair in the \ref Fe_50kV_Cascades "50keV Fe in Fe cascades" example.

@note For `CascadeComplete` the ion whose properties are binned is the PKA at its *initial* position, and the energy variable `E` is the PKA recoil energy rather than a current kinetic energy.

### Binning variables {#tallies-user-bins}

| JSON key    | Name in output | Quantity | Unit |
| ----------- | -------------- | -------- | ---- |
| `x`         | `x`            | Position vector x component | nm |
| `y`         | `y`            | Position vector y component | nm |
| `z`         | `z`            | Position vector z component | nm |
| `r`         | `r`            | Radial distance \f$r=\sqrt{x^2+y^2+z^2}\f$ | nm |
| `rho`       | `rho`          | Cylindrical radial distance \f$\rho=\sqrt{x^2+y^2}\f$ | nm |
| `cosTheta`  | `costheta`     | Cosine of the polar angle, \f$\cos\theta = z/r\f$ | – |
| `nx`        | `nx`           | x-axis direction cosine | – |
| `ny`        | `ny`           | y-axis direction cosine | – |
| `nz`        | `nz`           | z-axis direction cosine | – |
| `E`         | `E`            | Ion kinetic energy, or PKA recoil energy for `CascadeComplete` | eV |
| `Tdam`      | `Tdam`         | Cascade damage energy (`CascadeComplete` only) | eV |
| `V`         | `V`            | Number of vacancies generated in the cascade (`CascadeComplete` only) | – |
| `atom_id`   | `atom_id`      | Atomic species id (0 = beam ion) | – |
| `recoil_id` | `recoil_id`    | Recoil generation id (0 = beam ion, 1 = PKA, >1 = higher order) | – |

`Tdam` and `V` are only defined for the `CascadeComplete` event; with any other event they evaluate to 0 and the tally will only score in the bin containing 0, if any.

The positions `x`, `y`, `z`, `r`, `rho`, `cosTheta` and the direction cosines `nx`, `ny`, `nz` refer to the tally's own coordinate system (see below).

### Binning rules {#tallies-user-binrules}

- Bin edges must be given as a **monotonically increasing** array of at least 2 values. A variable with fewer than 2 edges is ignored, i.e., it does not create a tally dimension. At least one variable must define valid edges, otherwise the configuration is rejected.
- \f$n+1\f$ edges define \f$n\f$ bins. A value \f$v\f$ scores in bin \f$i\f$ if \f$v \in [b_i, b_{i+1})\f$.
- A value outside the range of the edges is **discarded**. There are no underflow/overflow bins.
- If several variables are binned, the event is scored only if **all** of them fall inside their respective ranges.
- Integer-valued variables (`atom_id`, `recoil_id`) need edges that bracket each integer value. For a target with two atomic species, edges `[0, 1, 2, 3]` give three bins counting beam ions, species 1 and species 2 respectively; edges `[1, 2]` select species 1 alone.

### Dimension ordering {#tallies-user-dims}

The `data` array has one dimension per binning variable with valid edges. **The order of the dimensions is fixed by OpenTRIM and is independent of the order in which the variables appear in the JSON config.** It is:

    x, y, z, r, rho, cosTheta, nx, ny, nz, E, Tdam, V, atom_id, recoil_id

`V` is a special case: it expands to \f$N_{at}-1\f$ dimensions, one per target atomic species in order of increasing atom id, all sharing the same bin edges. A tally binning `V` in a two-species target therefore gets two `V` dimensions.

The authoritative description of the layout is always written into the output file: `bin_names[j]` and `bin_descriptions[j]` name the variable of dimension `j`, and the dataset `bins/j` holds its \f$N_j+1\f$ edges. Post-processing code should use these rather than reconstructing the order from the config.

### Coordinate system {#tallies-user-cs}

Each user tally can define its own reference frame, which is useful for tallies centred on the beam spot or on a feature of the target. It is specified by three vectors expressed in simulation coordinates:

- \ref _UserTally_0_coordinate_system_origin "origin" – the origin of the local frame [nm]
- \ref _UserTally_0_coordinate_system_zaxis "zaxis" – a vector parallel to the local z-axis
- \ref _UserTally_0_coordinate_system_xzvector "xzvector" – a vector lying on the local xz-plane

`zaxis` and `xzvector` must not be parallel. With the default values (`origin` = [0,0,0], `zaxis` = [0,0,1], `xzvector` = [1,0,1]) the tally frame coincides with the simulation frame.

Ion positions are translated and rotated into this frame, directions are rotated only, before the binning variables are evaluated. The three vectors are stored in the output under `coordinate_system/`.

### Output layout {#tallies-user-output}

Each user tally is written to its own group `/user_tally/<id>`:

| Dataset                        | Size | Contents |
| ------------------------------ | ---- | -------- |
| `description`                  | scalar | The user supplied description |
| `event`                        | scalar | Name of the trigger event |
| `event_description`            | scalar | One-line description of the trigger event |
| `bin_names`                    | \f$[N_b]\f$ | Variable name of each dimension |
| `bin_descriptions`             | \f$[N_b]\f$ | Description of each dimension |
| `bins/0`, `bins/1`, …          | \f$[N_j+1]\f$ | Bin edges of dimension \f$j\f$ |
| `data`                         | \f$[N_1,\ldots,N_{N_b}]\f$ | Mean counts per source ion |
| `data_sem`                     | \f$[N_1,\ldots,N_{N_b}]\f$ | SEM of `data` |
| `coordinate_system/origin`     | [3] | Frame origin |
| `coordinate_system/zaxis`      | [3] | Frame z-axis |
| `coordinate_system/xzvector`   | [3] | Vector on the frame xz-plane |

### Example {#tallies-user-example}

The following tally, taken from the \ref 270keV_He_on_C "270keV He on C" example, records the angular distribution of beam ions transmitted through a thin carbon foil:

```javascript
"UserTally": [
    {
        "id": "AngularDistribution",
        "description": "Angular distribution of He ions exiting the foil",
        "event": "IonExit",
        "bins": {
            "nx": [0.980, 0.981, 0.982, /* ... */ 1.000],
            "x": [440, 450],
            "atom_id": [0, 1]
        }
    }
]
```

The `x` and `atom_id` bins act as filters. The target is 440 nm thick along x, so the single bin \f$[440, 450)\f$ selects ions leaving through the far face, and the single bin `atom_id` \f$\in [0,1)\f$ selects atom id 0, i.e. the beam ions. Only the 20 `nx` bins carry the distribution.

The resulting `data` array has shape \f$[1, 20, 1]\f$, following the canonical order `x`, `nx`, `atom_id` — note that this differs from the order written in the config.

Further worked examples are given in the `examples/python/usertally.ipynb` notebook and in the \ref Fe_50kV_Cascades "50keV Fe in Fe cascades" configuration.

\see user_tally, user_tally::parameters, user_tally::bin_var_t

## Event tables {#tallies-events}

Event tables record individual Monte-Carlo events instead of accumulating them into a histogram. They allow arbitrary post-processing — correlations, custom binning, spatial reconstruction of cascades — at the cost of a much larger output file.

Three event tables are available, each enabled by its own option in the `Output` section of the \ref json_config "JSON configuration":

| Option | Group | Contents |
| ------ | ----- | -------- |
| \ref _Output_store_pka_events "store_pka_events"       | `/events/pka`    | One row per PKA cascade |
| \ref _Output_store_exit_events "store_exit_events"     | `/events/exit`   | One row per ion leaving the simulation volume |
| \ref _Output_store_damage_events "store_damage_events" | `/events/damage` | One row per vacancy or interstitial created |

All three are disabled by default. A group is absent from the output file if its option is off.

### Common structure {#tallies-events-structure}

Each group contains three datasets:

| Dataset               | Size | Contents |
| --------------------- | ---- | -------- |
| `event_data`          | \f$[N_{ev} \times N_{cols}]\f$ | The event data, one row per event |
| `column_names`        | \f$[N_{cols}]\f$ | Short name of each column |
| `column_descriptions` | \f$[N_{cols}]\f$ | Description and unit of each column |

`event_data` is a 32-bit float dataset, stored chunked and deflate-compressed. Unlike tallies, the values are **raw, un-normalized** data: no division by the number of histories takes place.

@note All columns, including the history id and the various integer ids, are stored as `float32`. Integer ids are therefore represented exactly only up to \f$2^{24}\f$ = 16 777 216. For runs with more histories than that, the `hid` column loses resolution.

During the run, events are appended to temporary files in the system temporary directory and are transferred to the HDF5 archive only when the output file is written. Enough free space must be available there for the complete event stream. In multi-threaded runs, the per-thread streams are merged in order of the history id, so the rows of `event_data` are ordered by history, and within a history in the order the events occurred. This ordering is independent of the number of threads.

Event tables are read back when a simulation is continued from an existing output file, so the new file contains the events of both runs.

The column layout of each table is described below; `column_names` and `column_descriptions` in the file are always the authoritative reference, since the number of PKA columns depends on the target.

### PKA events {#tallies-events-pka}

`/events/pka` has \f$7 + 4\,N_t\f$ columns, where \f$N_t = N_{at}-1\f$ is the number of target atomic species. A row is written when a PKA cascade is complete, so the defect counts refer to the whole cascade.

| Column | Name | Description |
| ------ | ---- | ----------- |
| 0 | `hid` | History id of the source ion that generated the PKA |
| 1 | `pid` | Atom id of the PKA species |
| 2–4 | `x`, `y`, `z` | Position where the PKA was created [nm] |
| 5 | `E` | PKA recoil energy [eV] |
| 6 | `Tdam` | Damage energy of the cascade [eV] |
| 7 … | `V1` … `V`\f$N_t\f$ | Vacancies of each target species generated in the cascade |
| … | `I1` … `I`\f$N_t\f$ | Interstitials of each target species |
| … | `ICR1` … `ICR`\f$N_t\f$ | Intra-cascade recombinations of each target species |
| … | `Corr. ICR1` … | Correlated intra-cascade recombinations of each target species |

The `ICR` and `Corr. ICR` columns are zero unless \ref _Simulation_intra_cascade_recombination "intra_cascade_recombination" is enabled. The `V` and `I` counts are the numbers surviving recombination.

`E` is the recoil energy \f$T\f$ transferred in the collision, before subtraction of the lattice binding energy \f$E_l\f$. In a `"CascadesOnly"` simulation, where the source ions are themselves the PKAs, `E` is the source ion energy after subtraction of \f$E_l\f$.

The cell id is not stored; it can be recovered from the position and the grid arrays.

\see pka_buffer

### Exit events {#tallies-events-exit}

`/events/exit` has 10 columns and one row for every ion — beam ion or recoil — that leaves the simulation volume through an external boundary.

| Column | Name | Description |
| ------ | ---- | ----------- |
| 0 | `hid` | History id |
| 1 | `iid` | Atom id of the exiting ion |
| 2 | `cid` | Id of the cell the ion occupied before exiting |
| 3 | `E` | Kinetic energy at the moment of exit [eV] |
| 4–6 | `x`, `y`, `z` | Position on the boundary [nm] |
| 7–9 | `nx`, `ny`, `nz` | Direction cosines at exit |

This table is the natural input for transmission, backscattering and sputtering analysis. The same events are tallied in `/tally/ion_stat/Lost` and `/tally/energy_deposition/Lost`.

\see exit_buffer

### Damage events {#tallies-events-damage}

`/events/damage` has 7 columns and one row for every defect that survives to be tallied. It gives the full spatial distribution of the generated damage.

| Column | Name | Description |
| ------ | ---- | ----------- |
| 0 | `hid` | History id |
| 1 | `rid` | Recoil generation id: 0 for beam ions, 1 for PKAs, >1 for higher-order recoils |
| 2 | `iid` | Atom id of the interstitial species, or of the atom that occupied the vacated site |
| 3 | `did` | Defect type: 0 = vacancy, 1 = interstitial |
| 4–6 | `x`, `y`, `z` | Position of the defect [nm] |

Rows are written when the cascade completes, after intra-cascade recombination; recombined pairs do not appear. Within a cascade the interstitials are written first, followed by the vacancies.

\see damage_event_buffer

## Reading the output {#tallies-reading}

Any HDF5-capable tool can read the archive. With Python and `h5py`:

```python
import h5py
import numpy as np

with h5py.File('out.h5', 'r') as f:
    nh = f['/run_info/total_ion_count'][()]
    labels = [s.decode() for s in f['/target/atoms/label'][()]]

    # vacancy depth profile of atom id 1, per ion.
    # the table is 4-D [atom, ix, iy, iz], so just index the spatial axes
    vac = f['/tally/damage_events/Vacancies']
    v = vac[1, :, 0, 0]
    dv = f['/tally/damage_events/Vacancies_sem'][1, :, 0, 0]

    # cell centres, stored as [3, Nc] and indexed by the flat cell id
    nat, nx, ny, nz = vac.shape
    xyz = f['/target/grid/cell_xyz'][()].reshape(3, nx, ny, nz)
    xc = xyz[0, :, 0, 0]

    # a user tally with its bin edges
    g = f['/user_tally/AngularDistribution']
    names = [s.decode() for s in g['bin_names'][()]]
    edges = [g[f'bins/{j}'][()] for j in range(len(names))]
    data = g['data'][()]

    # PKA events as a named table
    ev = f['/events/pka']
    cols = [s.decode() for s in ev['column_names'][()]]
    pka = ev['event_data'][()]
    Tdam = pka[:, cols.index('Tdam')]
```

More complete examples are available as Jupyter notebooks under `examples/python/` and as Octave scripts under `examples/octave/`.

\see tally, user_tally, event_stream, event_buffer, Event, out_file
