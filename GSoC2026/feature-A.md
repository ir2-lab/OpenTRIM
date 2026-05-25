# Feature A - Python Bindings for OpenTRIM
## GSoC 2026 · Revised Technical Design


## Table of Contents

1. Purpose
2. Design Philosophy
3. Three Python Classes
4. Python Enums and Classes
5. opentrim.Config - Complete Reference
6. opentrim.Driver - Complete Reference
7. opentrim.Info - Complete Reference
8. Threading Model - Mode A & Mode B
9. Data Flow: Config → Simulation → Results
10. Complete Example
11. Build System
12. File Organization
13. PR Plan
14. Library Modifications (Minimal)
15. Code Introspection & Autocomplete
16. Testing & Documentation
17. Future Work

---

## 1. Purpose

Allow users to configure, run and analyze OpenTRIM simulations from Python - scripts, Jupyter notebooks, or interactive sessions - without touching JSON files or the CLI.

```
Before:  Edit config.json → opentrim -f config.json → h5py.File("out.h5") → matplotlib
After:   config = opentrim.Config()  →  sim.run()  →  info["tally"]["damage_events"]["Vacancies"]
```

---

## 2. Design Philosophy

Design principles for the binding:

| Principle | What it means in practice |
|-----------|--------------------------|
| **No direct array access** | All data goes through `mcinfo`, which normalizes per-ion, computes SEM, and returns copies. |
| **All data through `mcinfo`** | One access path: `opentrim.Info`. No parallel "fast paths" or raw tally views. |
| **Strict, typed Config** | Config is typed Python structs, not free-form dicts. Enums are Python enums - autocomplete and typo checking work out of the box. |
| **JSON is internal** | Typed structs are the primary interface. JSON methods (`to_json`/`from_json`) exist for interop and debug, but the typical workflow requires no JSON. |
| **Three classes, nothing more** | `Config`, `Driver`, `Info` - that's it. |
| **`mccore` access is future work** | Raw tally counts, variance arrays, event streams, RNG state write (`set_rng_state`) - all deferred. Read-only `rng_state` is available now via `info["run_info"]["rng_state"]` (goes through `mcinfo`, not direct `mccore`). |

---

## 3. Three Python Classes

```
┌─────────────────────────────────────────────────────────┐
│  import opentrim                                        │
│                                                         │
│  opentrim.Config    →  configure the simulation         │
│  opentrim.Driver    →  run the simulation               │
│  opentrim.Info      →  read the results                 │
└─────────────────────────────────────────────────────────┘
```

Wrapping map:

| Python class | C++ class | Header |
|---|---|---|
| `opentrim.Config` | `mcconfig` | `source/include/mcdriver.h` |
| `opentrim.Driver` | `mcdriver` | `source/include/mcdriver.h` |
| `opentrim.Info` | `mcinfo` | `source/include/mcinfo.h` |

---

## 4. Python Enums and Classes

All enums are exposed at module level: `opentrim.SimulationType.FullCascade`, not `opentrim.mccore.simulation_type_t.FullCascade`. `opentrim.Element` is also defined here — it wraps `element_t` and is used in both `Config.IonBeam.ion` and `Atom.element`.

### `opentrim.SimulationType`
Wraps `mccore::simulation_type_t`

```python
# Values bound from C++ - do not assume integer ordering
opentrim.SimulationType.FullCascade   # Full damage cascade (ions + recoils)
opentrim.SimulationType.IonsOnly      # Only primary ions, no cascade
opentrim.SimulationType.CascadesOnly  # Only recoil cascades, no primary ions
```

### `opentrim.ScreeningType`
Wraps `mccore::screening_t`

```python
opentrim.ScreeningType.NoScreening  # Unscreened Coulomb potential
opentrim.ScreeningType.Bohr
opentrim.ScreeningType.KrC          # Krypton-Carbon
opentrim.ScreeningType.Moliere
opentrim.ScreeningType.ZBL          # Ziegler-Biersack-Littmark (most common)
opentrim.ScreeningType.ZBL_MAGIC    # ZBL with analytic approximation
```

> `mccore::None` → `NoScreening` (Python keyword clash):
> ```cpp
> py::enum_<mccore::screening_t>(m, "ScreeningType")
>     .value("NoScreening", mccore::None)   // "None" is reserved - renamed
>     .value("Bohr",        mccore::Bohr)
>     .value("KrC",         mccore::KrC)
>     .value("Moliere",     mccore::Moliere)
>     .value("ZBL",         mccore::ZBL)
>     .value("ZBL_MAGIC",   mccore::ZBL_MAGIC);
> ```

### `opentrim.Stopping`
Wraps `dedx_calc::electronic_stopping_t`

```python
opentrim.Stopping.Off    # No electronic stopping
opentrim.Stopping.SRIM96
opentrim.Stopping.SRIM13  # Default - SRIM 2013 tables
opentrim.Stopping.DPASS   # DPASS parametrization v.21.06
```

> `options_spec.json` calls this `"DPASS22"` but the C++ enum and `parse_json.cpp:179`
> use `"DPASS"`. Python name follows the C++ enum: `opentrim.Stopping.DPASS`.

### `opentrim.Straggling`
Wraps `dedx_calc::electronic_straggling_t`

```python
opentrim.Straggling.Off   # No straggling (default)
opentrim.Straggling.Bohr
opentrim.Straggling.Chu
opentrim.Straggling.Yang
```

### `opentrim.NRT_Impl`
Wraps `mccore::nrt_calculation_t`

```python
opentrim.NRT_Impl.NRT_element  # Per-element Ed (default)
opentrim.NRT_Impl.NRT_average  # Average Ed over all target atoms
```

### `opentrim.FlightPath`
Wraps `flight_path_calc::flight_path_type_t`

```python
opentrim.FlightPath.Constant  # Constant mean free path (default)
opentrim.FlightPath.Variable  # Energy-dependent mean free path
```

### `opentrim.Distribution`
Wraps `ion_beam::distribution_t`

```python
opentrim.Distribution.SingleValue  # Monoenergetic / point source (default)
opentrim.Distribution.Uniform
opentrim.Distribution.Gaussian
```

### `opentrim.Geometry`
Wraps `ion_beam::geometry_t`

```python
opentrim.Geometry.Surface  # Ions start at target surface (default)
opentrim.Geometry.Volume   # Ions start inside target volume
```

### `opentrim.Element`
Wraps `element_t` (`source/include/ion.h`)

```python
opentrim.Element("He")   # from symbol → auto-sets Z=2, M=mean atomic mass
opentrim.Element(2)      # from atomic number → auto-sets symbol="He", M

e = opentrim.Element("Fe")
e.symbol        # "Fe"
e.atomic_number # 26
e.atomic_mass   # 55.845

# Used in both IonBeam and Target atoms:
config.IonBeam.ion    = opentrim.Element("He")
atom.element          = opentrim.Element("Fe")
```

---

### `opentrim.Event`
Wraps `enum class Event : int32_t` (`source/include/tally.h`)

```python
# Common values for configuring user tallies
opentrim.Event.IonStop           # Default - record when ion stops
opentrim.Event.IonExit           # Ion exits target boundary
opentrim.Event.Vacancy           # Vacancy creation event
opentrim.Event.Replacement       # Replacement collision
opentrim.Event.CascadeComplete   # End of a damage cascade
opentrim.Event.BoundaryCrossing  # Ion crosses a region boundary

# Also bound (internal / less common)
opentrim.Event.NewSourceIon      # New primary ion started
opentrim.Event.NewRecoil         # New recoil ion created
opentrim.Event.Scattering        # Every scattering event
opentrim.Event.NewFlightPath     # Start of a new flight path
opentrim.Event.NEvent            # Sentinel (total count)
opentrim.Event.Invalid
```

---

## 5. `opentrim.Config` - Complete Reference

Wraps `mcconfig` (defined in `source/include/mcdriver.h`).

### Construction

```python
config = opentrim.Config()          # all options at default values
config = opentrim.Config("sim.json") # load from JSON file
```

### Dual Access - Dict and Attribute Style

```python
# Attribute style (recommended - autocomplete and type hints work in IDEs)
config.Run.max_no_ions = 10000
config.Simulation.electronic_stopping = opentrim.Stopping.SRIM13

# Dict style (same typed structs - useful for programmatic/loop access)
config["Run"]["max_no_ions"] = 10000
config["Simulation"]["electronic_stopping"] = opentrim.Stopping.SRIM13
```

### Methods

```python
config.validate()   # check for errors; raises ValueError with description
```

> ```python
> config.to_json()                       # → str   full config as JSON string
> config = opentrim.Config.from_json(s)  # classmethod: construct from JSON string
> opentrim.Config.options_spec()         # → str   JSON spec of all options (types, ranges, docs)
> ```
> `from_json` constructs a new Config from a JSON string - does not modify an existing one.
> Prefer the constructor for files: `opentrim.Config("sim.json")`. JSON is otherwise
> handled internally (e.g., during `sim.save()` / `sim.load()`).

### Sub-structs

#### `config.Simulation`
Wraps `mccore::parameters`

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `simulation_type` | `opentrim.SimulationType` | `FullCascade` | What to simulate |
| `screening_type` | `opentrim.ScreeningType` | `ZBL` | Nuclear scattering potential |
| `electronic_stopping` | `opentrim.Stopping` | `SRIM13` | Electronic stopping model |
| `electronic_straggling` | `opentrim.Straggling` | `Off` | Electronic straggling |
| `nrt_calculation` | `opentrim.NRT_Impl` | `NRT_element` | NRT damage calculation |
| `intra_cascade_recombination` | `bool` | `False` | Recombination within cascade |
| `time_ordered_cascades` | `bool` | `True` | Time-ordered cascade tracking (experimental) |
| `correlated_recombination` | `bool` | `True` | Correlated recombination (experimental) |
| `move_recoil` | `bool` | `False` | Move recoil below Ed (experimental) |
| `recoil_sub_ed` | `bool` | `False` | Follow recoils below Ed (experimental) |

```python
config.Simulation.electronic_stopping = opentrim.Stopping.SRIM13
config.Simulation.simulation_type     = opentrim.SimulationType.FullCascade
config.Simulation.nrt_calculation     = opentrim.NRT_Impl.NRT_element
```

#### `config.Transport`
Wraps `mccore::transport_options`

| Field | Type | Default | Unit | Description |
|-------|------|---------|------|-------------|
| `flight_path_type` | `opentrim.FlightPath` | `Constant` | - | Flight path sampling |
| `flight_path_const` | `float` | `1.0` | atomic radii | Constant MFP value |
| `min_energy` | `float` | `1.0` | eV | Stop tracking ion below this energy |
| `min_recoil_energy` | `float` | `1.0` | eV | Stop tracking recoil below this |
| `min_scattering_angle` | `float` | `2.0` | degrees | Skip collisions below this angle |
| `max_rel_eloss` | `float` | `0.05` | dE/E | Max relative energy loss per step |
| `mfp_range` | `[float, float]` | `[1.0, 1e30]` | atomic radii | MFP limits |

```python
config.Transport.min_energy    = 5.0    # eV
config.Transport.flight_path_type = opentrim.FlightPath.Variable
```

> Note: unit is atomic radius (as per `flight_path.h:24–29`). `mccore.h:128` incorrectly states `[nm]` and will be corrected.

#### `config.IonBeam`
Wraps `ion_beam::parameters`

```python
config.IonBeam.ion = opentrim.Element("He")   # auto-sets Z=2, M=4.003
# or explicitly:
config.IonBeam.ion = opentrim.Element(2)      # from atomic number

config.IonBeam.energy_distribution.type   = opentrim.Distribution.SingleValue
config.IonBeam.energy_distribution.center = 2e6   # eV - 2 MeV beam
config.IonBeam.energy_distribution.fwhm   = 0.0   # eV

config.IonBeam.spatial_distribution.geometry = opentrim.Geometry.Surface
config.IonBeam.spatial_distribution.type     = opentrim.Distribution.SingleValue

config.IonBeam.angular_distribution.type     = opentrim.Distribution.SingleValue
config.IonBeam.angular_distribution.center   = [1.0, 0.0, 0.0]  # beam along x
```

#### `config.Target`
Wraps `target::target_desc_t`

```python
config.Target.size       = [100.0, 100.0, 100.0]   # nm
config.Target.origin     = [0.0, 0.0, 0.0]          # nm
config.Target.cell_count = [200, 1, 1]               # grid cells
config.Target.periodic_bc = [0, 1, 1]               # non-periodic in x, periodic in y,z

# Add a material
mat = opentrim.Material()
mat.id      = "Iron"
mat.density = 7.874  # g/cm³
atom = opentrim.Atom()
atom.element = opentrim.Element("Fe")   # auto-sets Z=26, M=55.845
atom.X  = 1.0    # 100% Fe
atom.Ed = 40.0   # eV - displacement threshold
atom.El = 3.0    # eV - lattice binding energy
atom.Es = 4.28   # eV - surface binding energy
atom.Er = 40.0   # eV - replacement threshold
atom.Rc = 0.946  # nm  - recombination radius
mat.composition.append(atom)
config.Target.materials.append(mat)

# Define a region
region = opentrim.Region()
region.id          = "FeLayer"
region.material_id = "Iron"
region.origin      = [0.0, 0.0, 0.0]
region.size        = [100.0, 100.0, 100.0]
config.Target.regions.append(region)
```

#### `config.Run`
Wraps `mcconfig::run_options`

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `max_no_ions` | `int` | `100` | Number of ions to simulate |
| `max_cpu_time` | `int` | `0` | Max wall time in seconds (0 = unlimited) |
| `threads` | `int` | `1` | Worker threads (≤ 0 = auto-detect: half of `hardware_concurrency()` if >3 cores, else 1) |
| `seed` | `int` | `123456789` | RNG seed for reproducibility |

```python
config.Run.max_no_ions = 50000
config.Run.threads     = 8
config.Run.seed        = 42
```

#### `config.Output`
Wraps `mcconfig::output_options`

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `title` | `str` | `"Ion Simulation"` | Simulation title |
| `outfilename` | `str` | `"out"` | Output HDF5 filename (no extension) |
| `storage_interval` | `int` | `1000` | HDF5 write interval in ms |
| `store_exit_events` | `bool` | `False` | Save per-ion exit events |
| `store_pka_events` | `bool` | `False` | Save PKA events |
| `store_damage_events` | `bool` | `False` | Save damage event positions |
| `store_dedx` | `bool` | `True` | Save electronic stopping tables |

```python
config.Output.outfilename = "Fe_2MeV_He"
config.Output.store_damage_events = True
```

#### `config.UserTally`
List of user-defined tallies. Each element wraps `user_tally::parameters`.

```python
tally = opentrim.UserTally()
tally.id          = "depth_profile"
tally.description = "Vacancy depth profile"
tally.event       = opentrim.Event.Vacancy
tally.bins.x      = list(range(0, 101))  # 0–100 nm in 1 nm steps

config.UserTally.append(tally)
```

---

## 6. `opentrim.Driver` - Complete Reference

Wraps `mcdriver` (defined in `source/include/mcdriver.h`).

### Construction and Initialization

```python
sim = opentrim.Driver()     # create driver (no config yet)
sim.init(config)            # initialize from a Config object
```

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `max_ions` | `int` (read/write) | Max ions to simulate - update before or between runs |
| `max_cpu_time` | `int` (read/write) | Max wall time in seconds (0 = unlimited) |

```python
sim.max_ions = 100000   # update limit before calling run()
```

> Note: `max_ions` maps to `config.Run.max_no_ions` and is read once at `exec()` start (`mcdriver.cpp:120`). Changes take effect on the next `run()` call.

### Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `init(config)` | `None` | Initialize driver from Config |
| `config()` | `opentrim.Config` | Return copy of the current config |
| `run()` | `None` | **Mode A**: non-blocking - returns immediately |
| `run(callback, interval_ms)` | `None` | **Mode B**: blocking with callback every `interval_ms` |
| `is_running()` | `bool` | True if simulation is currently active (see note below) |
| `ion_count()` | `int` | Ions simulated so far - binding delegates to `mccore::ion_count()` via `getSim()` |
| `abort()` | `None` | Signal simulation to stop gracefully |
| `wait()` | `None` | Block Python until simulation completes (see note below) |
| `reset()` | `None` | Abort and clear all simulation state |
| `save(filename)` | `None` | Save to HDF5 file (identical to CLI output) |
| `load(filename)` | `None` | Load from HDF5 file (can continue simulation) |

> Note: `mcdriver::is_running()` checks `thread_pool_.size() > 0` (`mcdriver.h:335`).
> `thread_pool_` is populated at `mcdriver.cpp:178`, after some setup. If Python calls
> `is_running()` before that line runs, it returns `false` incorrectly. The binding must
> maintain its own `std::atomic<bool> exec_running_`, set before launching `exec_thread_`
> and cleared after it finishes - see Section 8.

> Note: `mcdriver::wait()` joins the internal worker pool - already cleared by the time
> `exec()` returns. For Mode A, `sim.wait()` must join `exec_thread_` (the binding-side
> thread running `exec()`), not `mcdriver::wait()`. See Section 8.

---

## 7. `opentrim.Info` - Complete Reference

Wraps `mcinfo` (defined in `source/include/mcinfo.h`).

All simulation results go through `mcinfo`. It normalizes raw tally counts per ion,
computes SEM, and returns numpy arrays. No direct C++ array access is exposed.

### Construction

```python
info = opentrim.Info(sim)   # sim is an opentrim.Driver
```

`Info` holds a reference to the `Driver` it was constructed from. The binding must
keep the `Driver` alive as long as the `Info` object exists - implemented via Python
reference counting (pass `sim` as a `py::object` kept-alive argument).

> Note: `mcinfo` stores a raw `const mcdriver*` internally, not a `shared_ptr`.
> `mcinfo.h` may need to be extended during development to support shared ownership.

> Note: Accessing `Info` while a simulation is running in Mode A may lead to data
> races. Prefer reading results only after `sim.wait()` until thread safety is confirmed
> during implementation.

### Interface

```python
info.keys()                # → list[str]  - available keys at this level
info["key"]                # → sub-Info (if group) or numpy array / tuple / str
info.type                  # → str: "group", "string", "json", "real64", "real32", "uint64", "tally_score"
info.description           # → str   node description from mcinfo_spec.json
repr(info)                 # → human-readable tree summary
opentrim.Info.info_spec()  # → str   JSON spec of the full info tree structure
```

> Note: `mcinfo::operator[]` uses `nlohmann::ordered_map::operator[]` (see `mcinfo.h:42`)
> which silently inserts a default-constructed node for missing keys. The binding must use
> `children().at(key)` instead, converting `std::out_of_range` to `py::key_error`:
>
> ```cpp
> .def("__getitem__", [](const mcinfo& node, const std::string& key) -> const mcinfo& {
>     try { return node.children().at(key); }
>     catch (const std::out_of_range&) { throw py::key_error(key); }
> }, py::return_value_policy::reference_internal)
>
> .def("keys", [](const mcinfo& node) {
>     std::vector<std::string> k;
>     for (auto& [name, _] : node.children())
>         k.push_back(name);
>     return k;
> })
> ```

### Tree Structure

Runtime keys come from `mcinfo.cpp` (the spec file disagrees in a few places). Use the runtime keys; see the mismatch table below.

```python
# ─── run_info ────────────────────────────────────────────────────────
info["run_info"]["title"]              # → str        simulation title (config.Output.title)
info["run_info"]["total_ion_count"]    # → uint64[1]  total ions simulated
info["run_info"]["json_config"]        # → str        full config serialized as JSON
info["run_info"]["run_history"]        # → str        JSON run timing history

info["run_info"]["version_info"]       # → group
info["run_info"]["version_info"]["name"]              # → str   code name
info["run_info"]["version_info"]["version"]           # → str   code version
info["run_info"]["version_info"]["git_tag"]           # → str   git tag
info["run_info"]["version_info"]["compiler"]          # → str   compiler id
info["run_info"]["version_info"]["compiler_version"]  # → str   compiler version
info["run_info"]["version_info"]["build_system"]      # → str   build system
info["run_info"]["version_info"]["build_time"]        # → str   build timestamp

info["run_info"]["rng_state"]          # → numpy uint64[N]   RNG state (N = rngState().size())

# ─── ion_beam ────────────────────────────────────────────────────────
info["ion_beam"]["E0"]                 # → float32[1]   mean beam energy (eV)
info["ion_beam"]["Z"]                  # → float32[1]   atomic number (float, not int)
info["ion_beam"]["M"]                  # → float32[1]   atomic mass (amu)
info["ion_beam"]["dir0"]               # → float32[3]   mean direction (from angular_distribution.center)
info["ion_beam"]["x0"]                 # → float32[3]   mean position (from spatial_distribution.center)

# ─── target ──────────────────────────────────────────────────────────
# Note: runtime key is "target" (lowercase) - mcinfo_spec.json shows "Target" (wrong)

info["target"]["grid"]["X"]            # → float32[Nx+1]   cell edges (nm)
info["target"]["grid"]["Y"]            # → float32[Ny+1]
info["target"]["grid"]["Z"]            # → float32[Nz+1]
info["target"]["grid"]["cell_xyz"]     # → float32[3, Ncells]   cell centers

info["target"]["atoms"]["label"]       # → list[str]   e.g. ["Fe in Iron"] - for plot legends
info["target"]["atoms"]["symbol"]      # → list[str]   element symbols
info["target"]["atoms"]["Z"]           # → float32[Natoms]   atomic number (float, not int)
info["target"]["atoms"]["M"]           # → float32[Natoms]   atomic mass (amu)
info["target"]["atoms"]["Ed"]          # → float32[Natoms]   displacement threshold (eV)
info["target"]["atoms"]["El"]          # → float32[Natoms]   lattice binding energy (eV)
info["target"]["atoms"]["Es"]          # → float32[Natoms]   surface binding energy (eV)
info["target"]["atoms"]["Er"]          # → float32[Natoms]   replacement threshold (eV)
info["target"]["atoms"]["Rc"]          # → float32[Natoms]   recombination radius (nm)

info["target"]["materials"]["name"]            # → list[str]
info["target"]["materials"]["atomic_density"]  # → float32[Nmats]   (atoms/nm³)
info["target"]["materials"]["mass_density"]    # → float32[Nmats]   (g/cm³)
info["target"]["materials"]["atomic_radius"]   # → float32[Nmats]   (nm)

# dedx and flight_path - only present when config.Output.store_dedx is True
info["target"]["dedx"]["erg"]          # → float32[Nerg]                  energy grid (eV)
info["target"]["dedx"]["eloss"]        # → float32[Nions, Nmats, Nerg]    stopping power
info["target"]["dedx"]["strag"]        # → float32[Nions, Nmats, Nerg]    straggling

info["target"]["flight_path"]["erg"]   # → float32[Nerg]                  energy grid (eV)
info["target"]["flight_path"]["mfp"]   # → float32[Nions, Nmats, Nerg]    mean free path
info["target"]["flight_path"]["ipmax"] # → float32[Nions, Nmats, Nerg]    max impact parameter
info["target"]["flight_path"]["fpmax"] # → float32[Nions, Nmats, Nerg]    max flight path

# ─── tally ───────────────────────────────────────────────────────────
# tally_score nodes return (values, sem) - normalized per ion, shape [Natoms, Nx, Ny, Nz]

tally = info["tally"]

v,  dv  = tally["damage_events"]["Vacancies"]
i,  di  = tally["damage_events"]["Implantations"]
r,  dr  = tally["damage_events"]["Replacements"]
rc, drc = tally["damage_events"]["Recombinations"]

e,  de  = tally["energy_deposition"]["Ionization"]
l,  dl  = tally["energy_deposition"]["Lattice"]
s,  ds  = tally["energy_deposition"]["Stored"]
lo, dlo = tally["energy_deposition"]["Lost"]

p,  dp  = tally["pka_damage"]["Pka"]
pe, dpe = tally["pka_damage"]["Pka_energy"]
t,  dt  = tally["pka_damage"]["Tdam"]
tl, dtl = tally["pka_damage"]["Tdam_LSS"]
vn, dvn = tally["pka_damage"]["Vnrt"]
vl, dvl = tally["pka_damage"]["Vnrt_LSS"]

fp, dfp = tally["ion_stat"]["Flight_path"]
c,  dc  = tally["ion_stat"]["Collisions"]
lc, dlc = tally["ion_stat"]["Lost"]         # ions lost/exited (distinct from energy_deposition.Lost)

data, sem = tally["totals"]["data"]          # → tally_score, shape [18, Natoms]
tally["totals"]["column_names"]              # → list[str]   names of the 18 tally columns

# ─── user_tally ──────────────────────────────────────────────────────
# Sub-keys are ut->id() strings (e.g. "depth_profile"). Empty group if no UserTally defined.
data, sem = info["user_tally"]["depth_profile"]["data"]          # → tally_score tuple
info["user_tally"]["depth_profile"]["decription"]                # → str   ("decription" - typo in C++, real key)
info["user_tally"]["depth_profile"]["event"]                     # → str
info["user_tally"]["depth_profile"]["event_decription"]          # → str   ("event_decription" - typo in C++, real key)
info["user_tally"]["depth_profile"]["bin_names"]                 # → list[str]
info["user_tally"]["depth_profile"]["bin_descriptions"]          # → list[str]
info["user_tally"]["depth_profile"]["coordinate_system"]["zaxis"]    # → float32[3]
info["user_tally"]["depth_profile"]["coordinate_system"]["xzvector"] # → float32[3]
info["user_tally"]["depth_profile"]["coordinate_system"]["origin"]   # → float32[3]
info["user_tally"]["depth_profile"]["bins"]["0"]                 # → float32[Nbins+1]   bin edges dim 0
info["user_tally"]["depth_profile"]["bins"]["1"]                 # → float32[Nbins+1]   bin edges dim 1
```

> | mcinfo_spec.json | mcinfo.cpp (runtime key) |
> |---|---|
> | `"Target"` | `"target"` - spec uses title-case for UI labels |
> | `"dEdx"` | `"eloss"` |
> | `"data_sem"` | does not exist - SEM is returned via `mcinfo::get()` as a tuple |
> | `"events"` group | not built in mcinfo.cpp |

### Note on Tally Data

All tally values are normalized per ion and returned as `(values, sem)` tuples.
`mcinfo` allocates flat buffers, computes results at query time, and returns copies. Python does not hold references into C++ memory.

---

## 8. Threading Model - Mode A & Mode B

`mcdriver::exec()` is a blocking C++ call - it spawns worker threads, waits for them,
merges results, then returns. The two run modes differ in how the binding handles this.

### Mode A - Non-blocking

```python
sim.run()              # returns immediately - exec() runs on a background std::thread
print(sim.ion_count()) # Python is free; ion_count() is an atomic counter
sim.wait()             # block until exec() completes
```

The binding launches `exec()` on a `std::thread` (`exec_thread_`) and returns immediately.
`sim.wait()` joins `exec_thread_` - not `mcdriver::wait()`, which only joins the internal
worker pool and is a no-op once `exec()` has already returned.

> Note: `mcdriver::is_running()` checks `thread_pool_.size() > 0` (see `mcdriver.h:335`).
> `thread_pool_` is populated at `mcdriver.cpp:178`, after some setup. If Python calls
> `is_running()` before that line runs, it returns `false` incorrectly. The binding must
> maintain its own `std::atomic<bool> exec_running_`, set before launching `exec_thread_`
> and cleared after it finishes:
>
> ```cpp
> exec_running_ = true;
> exec_thread_ = std::thread([this]{
>     driver_.exec();
>     exec_running_ = false;
> });
> // sim.is_running() checks exec_running_, not thread_pool_
> ```

### Mode B - Blocking with Python Callback

```python
def update_plot():
    v, dv = info["tally"]["damage_events"]["Vacancies"]
    plt.cla()
    plt.errorbar(x[:-1], v[0,:,0,0], yerr=dv[0,:,0,0])
    plt.draw(); plt.pause(0.01)

sim.run(update_plot, 1000)   # blocks Python REPL; callback every 1000 ms
update_plot()                # final update after sim finishes
```

`exec()` runs on the calling thread. The GIL is released before calling `exec()` so
Python is not fully blocked, and re-acquired inside the trampoline before each callback.
Ctrl+C raises `KeyboardInterrupt` at the next callback boundary, aborting the run:

```cpp
// Static trampoline - bridges raw C function pointer to Python callable
static void py_trampoline(const mcdriver&, void* p) {
    py::gil_scoped_acquire gil;
    (*static_cast<py::function*>(p))();
}

// Mode B binding - no changes to mcdriver.h
// Note: interval is quantized to 100 ms ticks (mcdriver.cpp:97-98); minimum effective interval is 100 ms
.def("run", [](mcdriver& d, py::function cb, size_t interval_ms) {
    py::gil_scoped_release release;
    d.exec(py_trampoline, interval_ms, &cb);
})
```

---


## 9. Data Flow: Config → Simulation → Results

```
opentrim.Config  (Python struct objects)
    │
    │  C++ struct fields copied directly - no JSON serialization at this stage
    │  Enums: Python enum value → C++ enum value (integer mapping)
    │
    ▼
mcconfig  (C++ struct, validated by mcconfig::validate())
    │
    ▼
mcdriver::init(const mcconfig&)
    │
    ▼
mcdriver::exec()
    ├── spawns N mccore worker threads (each: mccore::run() on its own clone)
    ├── each clone accumulates its own tally independently
    └── post-join: mergeTallies(), mergeEvents() - clones merged into main sim
    │
    ▼
opentrim.Info(sim)  - wraps mcinfo(const mcdriver*)
    │
    │  mcinfo::get(x, dx, dim):
    │    - gets raw tally ArrayNDd from mccore
    │    - allocates flat std::vector<double> buffers for x (mean) and dx (SEM)
    │    - normalizes: x[i] = A[i] / N,  dx[i] = sqrt((ΣA²[i]/N - x[i]²) / (N-1))
    │    - returns copies, not pointers into C++ internal arrays
    │
    ▼
numpy arrays  (data, sem)
    │
    ▼
matplotlib / analysis
```

---

## 10. Complete Example

```python
import opentrim
import numpy as np
import matplotlib.pyplot as plt

# ── 1. Configure ───────────────────────────────────────────────────────────
config = opentrim.Config()

# Ion beam: 2 MeV He²⁺
config.IonBeam.ion = opentrim.Element("He")
config.IonBeam.energy_distribution.center = 2e6  # eV

# Target: 100 nm Fe slab, 200 depth bins
mat = opentrim.Material()
mat.id = "Fe"; mat.density = 7.874
atom = opentrim.Atom()
atom.element = opentrim.Element("Fe")
atom.X  = 1.0                       # 100% Fe
atom.Ed = 40.0; atom.El = 3.0; atom.Es = 4.28; atom.Er = 40.0; atom.Rc = 0.946
mat.composition.append(atom)
config.Target.materials.append(mat)

region = opentrim.Region()
region.id = "slab"; region.material_id = "Fe"
region.origin = [0, 0, 0]; region.size = [100, 100, 100]
config.Target.regions.append(region)

config.Target.cell_count = [200, 1, 1]
config.Target.size = [100.0, 100.0, 100.0]

# Physics
config.Simulation.electronic_stopping = opentrim.Stopping.SRIM13
config.Simulation.simulation_type     = opentrim.SimulationType.FullCascade

# Run
config.Run.max_no_ions = 10000
config.Run.threads = 4
config.Run.seed = 42

config.validate()  # raises ValueError if anything is wrong

# ── 2. Run ─────────────────────────────────────────────────────────────────
sim = opentrim.Driver()
sim.init(config)

# Option A: non-blocking
sim.run()                                           # returns immediately
print(f"Ions so far: {sim.ion_count()}")
sim.wait()                                          # block until done
print(f"Done. Total ions: {sim.ion_count()}")

# ── 3. Read Results ────────────────────────────────────────────────────────
info = opentrim.Info(sim)

# Grid (x-axis edges in nm)
x_edges = info["target"]["grid"]["X"]   # numpy float[201]
x_centers = 0.5 * (x_edges[:-1] + x_edges[1:])  # 200 cell centers

# Vacancies with error bars
v, dv = info["tally"]["damage_events"]["Vacancies"]  # shape [1, 200, 1, 1]
v_depth  = v[0, :, 0, 0]
dv_depth = dv[0, :, 0, 0]

# ── 4. Plot ────────────────────────────────────────────────────────────────
plt.figure(figsize=(8, 4))
plt.errorbar(x_centers, v_depth, yerr=dv_depth, fmt='-', capsize=2)
plt.xlabel("Depth (nm)")
plt.ylabel("Vacancies per ion")
plt.title(f"He 2 MeV → Fe  (N={sim.ion_count():,} ions)")
plt.tight_layout()
plt.show()

# ── 5. Save / Load ─────────────────────────────────────────────────────────
sim.save("Fe_2MeV_He.h5")   # same HDF5 format as CLI output

sim2 = opentrim.Driver()
sim2.load("Fe_2MeV_He.h5")   # reload - continues from saved state
N = sim2.ion_count()
sim2.max_ions = N * 2         # double the ion count
sim2.run()
sim2.wait()

# ── 6. Live Plot (Mode B) ──────────────────────────────────────────────────
sim3 = opentrim.Driver()
sim3.init(config)
info3 = opentrim.Info(sim3)
x3 = info3["target"]["grid"]["X"]

def live_update():
    v3, dv3 = info3["tally"]["damage_events"]["Vacancies"]
    plt.cla()
    plt.errorbar(0.5*(x3[:-1]+x3[1:]), v3[0,:,0,0], yerr=dv3[0,:,0,0])
    plt.xlabel("Depth (nm)"); plt.ylabel("Vacancies/ion")
    plt.draw(); plt.pause(0.01)

sim3.run(live_update, 2000)   # blocks; callback every 2 seconds
live_update()                 # final update after sim finishes
```

---

## 11. Build System

### Dependencies

| Library | Version | Source | Role |
|---------|---------|--------|------|
| `pybind11` | v3.0.4 | `github.com/pybind/pybind11` | Binding engine - add to `cmake/Externals.cmake` |
| `nlohmann_json` | v3.11.3 | already in `cmake/Externals.cmake` | Config serialization (HDF5 save/load) - reused, not re-fetched |
| `HighFive` | v3.3.0 | already in `cmake/Externals.cmake` | HDF5 read/write - reused |

> Note: `pybind11_json` is not used - Config fields are bound directly as typed struct attributes.

### `cmake/Externals.cmake` - Addition

```cmake
# Append to existing cmake/Externals.cmake
FetchContent_Declare(external_pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v3.0.4
    GIT_SUBMODULES_RECURSE FALSE
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(external_pybind11)
```

### `pyproject.toml` - New file at repo root

```toml
[build-system]
requires      = ["scikit-build-core>=0.11", "pybind11>=3.0"]
build-backend = "scikit_build_core.build"

[project]
name             = "opentrim"
dynamic          = ["version"]   # scikit-build-core reads this from CMakeLists.txt project() version
requires-python  = ">=3.9"
description      = "Python bindings for the OpenTRIM Monte Carlo ion transport simulator"
dependencies     = ["numpy>=1.21"]

[project.optional-dependencies]
jupyter = ["ipython", "matplotlib"]
```

### `CMakeLists.txt` - Root level addition

```cmake
# In root CMakeLists.txt, alongside add_subdirectory(source):
option(OPENTRIM_BUILD_PYTHON "Build Python bindings" ON)
if(OPENTRIM_BUILD_PYTHON)
    add_subdirectory(bindings/python)
endif()
```

### `bindings/python/CMakeLists.txt` - New file

```cmake
# pybind11 is available via FetchContent in cmake/Externals.cmake - no find_package needed
pybind11_add_module(_opentrim_core MODULE
    bindings.cpp          # module entry: py::module_ m
    config_bind.cpp       # opentrim.Config + all sub-structs
    enums_bind.cpp        # all Python enums
    driver_bind.cpp       # opentrim.Driver + GIL/threading
    info_bind.cpp         # opentrim.Info + mcinfo tree
)

target_link_libraries(_opentrim_core PRIVATE
    opentrim                    # libopentrim.so (no Qt)
    nlohmann_json::nlohmann_json  # reused, not re-fetched
)

target_include_directories(_opentrim_core PRIVATE
    ${CMAKE_SOURCE_DIR}/source/include
)
```

**Output:** `_opentrim_core.cpython-3XX-linux-gnu.so` (Linux/WSL) or `_opentrim_core.pyd` (Windows)

---

## 12. File Organization

Target directory structure. `[Feature A]` marks new or modified files.

```
opentrim/
├── cmake/
│   └── Externals.cmake              ← add pybind11 FetchContent  [Feature A]
├── dist/
│   ├── mingw64/
│   └── OBS/
├── source/
│   ├── cli/
│   ├── gendoc/
│   ├── generators/
│   ├── gui/
│   ├── include/
│   │   └── mcinfo.h                 ← may need minor additions  [Feature A]
│   └── lib/
├── bindings/                        ← new top-level directory  [Feature A]
│   ├── python/
│   │   ├── CMakeLists.txt
│   │   ├── bindings.cpp             ← PYBIND11_MODULE(_opentrim_core, m)
│   │   ├── enums_bind.cpp
│   │   ├── config_bind.cpp
│   │   ├── driver_bind.cpp
│   │   ├── info_bind.cpp
│   │   └── opentrim/
│   │       └── __init__.py          ← from ._opentrim_core import Config, Driver, Info
├── examples/                        ← new directory  [Feature A]
│   ├── cpp/
│   └── python/
│       ├── basic_run.ipynb
│       ├── live_plot.ipynb
│       └── parametric_study.ipynb
├── test/
│   ├── cpp/
│   ├── python/
│   │   ├── 1MeV_H_on_Fe.ipynb      (existing)
│   │   ├── 1MeV_H_on_Fe.py         (existing)
│   │   ├── 270keV_He_on_C.ipynb     (existing)
│   │   ├── 270keV_He_on_C.py        (existing)
│   │   ├── 2MeV_Fe_on_Fe.ipynb      (existing)
│   │   ├── 2MeV_Fe_on_Fe.py         (existing)
│   │   ├── 3MeV_Xe_on_UO2.ipynb     (existing)
│   │   ├── 3MeV_Xe_on_UO2.py        (existing)
│   │   └── test_bindings.py         ← new pytest suite  [Feature A]
│   ├── octave/
│   ├── post-build/
│   ├── benchmarks/
│   └── srim_comp/
├── GSoC2026/                        ← new  [Feature A]
│   └── feature-A.md                 ← this document
├── doc/
├── pyproject.toml                   ← new  [Feature A]
└── CMakeLists.txt                   ← add bindings/python subdirectory  [Feature A]
```

> Note: `bindings/python/` links against the `opentrim` CMake target (`libopentrim.so`)
> which excludes Qt by design - no GUI dependency in the Python bindings.

---

## 13. PR Plan

5 PRs, one per week, targeting midterm completion.

```
Week 1  A-1  Build Foundation           May 25–31
         bindings/python/ directory. pybind11 via FetchContent.
         import opentrim works. pip install . works.

Week 2  A-2  Config + Enums             Jun 1–7
         All enums as Python enums. Config with all sub-structs.
         config.Simulation.electronic_stopping = opentrim.Stopping.SRIM13 works.
         config.validate() raises ValueError on bad input.

Week 3  A-3  Driver + Run Modes         Jun 8–14
         opentrim.Driver, sim.init(config), sim.run() (Mode A).
         GIL released. sim.wait(), sim.abort(), sim.ion_count().
         sim.run(callback, interval) (Mode B) with trampoline.

Week 4  A-4  Info + Full Results        Jun 15–21
         opentrim.Info wrapping mcinfo.
         All tally tables as (data, sem) numpy tuples.
         Target grid, atom properties, run metadata.
         __repr__ for Config and Info.

Week 5  A-5  HDF5 + Tests + Docs       Jun 22–28
         sim.save() / sim.load() round-trip.
         pytest suite in test/python/test_bindings.py.
         Sphinx API docs. Example notebooks.

         ────── Feature A complete before midterm ──────
         Midterm evaluation: Jul 6–10
```

### PR Table

| PR | Title | Delivers | Key Files |
|----|-------|----------|-----------|
| **A-1** | `build: pybind11 via FetchContent + bindings/python/ module` | `import opentrim` works, pip wheel builds | `cmake/Externals.cmake` ✎ · `CMakeLists.txt` ✎ · `pyproject.toml` ✚ · `bindings/python/CMakeLists.txt` ✚ · `bindings/python/bindings.cpp` ✚ · `bindings/python/opentrim/__init__.py` ✚ |
| **A-2** | `feat: opentrim.Config with typed sub-structs and Python enums` | Full Config API, all enums, validate() | `bindings/python/enums_bind.cpp` ✚ · `bindings/python/config_bind.cpp` ✚ |
| **A-3** | `feat: opentrim.Driver with Mode A and Mode B run semantics` | Non-blocking run, blocking+callback, GIL release | `bindings/python/driver_bind.cpp` ✚ |
| **A-4** | `feat: opentrim.Info wrapping mcinfo tree` | All tally data as (data, sem) numpy, grid, atoms, repr | `bindings/python/info_bind.cpp` ✚ · `bindings/python/config_bind.cpp` ✎ |
| **A-5** | `feat: HDF5 save/load + pytest suite + Sphinx docs + examples` | Complete round-trip, CI tests, API docs, example notebooks | `test/python/test_bindings.py` ✚ · `doc/conf.py` ✚ · `doc/api.rst` ✚ · `examples/python/basic_run.ipynb` ✚ · `examples/python/live_plot.ipynb` ✚ · `examples/python/parametric_study.ipynb` ✚ |

> ✚ = new file · ✎ = modified existing file

### File Trees per PR

**A-1 - Build Foundation**
```
cmake/Externals.cmake                    ✎  add pybind11 v3.0.4 FetchContent
CMakeLists.txt                           ✎  add_subdirectory(bindings/python)
pyproject.toml                           ✚
bindings/python/
├── CMakeLists.txt                       ✚
├── bindings.cpp                         ✚  PYBIND11_MODULE(_opentrim_core, m)
└── opentrim/
    └── __init__.py                      ✚  from ._opentrim_core import Config, Driver, Info
```

**A-2 - Config + Enums**
```
bindings/python/
├── CMakeLists.txt                       ✎  add enums_bind.cpp, config_bind.cpp
├── bindings.cpp                         ✎  register enums + Config
├── enums_bind.cpp                       ✚  all 9 Python enums
└── config_bind.cpp                      ✚  Config, all sub-structs, validate()
```

**A-3 - Driver + Run Modes**
```
bindings/python/
├── CMakeLists.txt                       ✎  add driver_bind.cpp
├── bindings.cpp                         ✎  register Driver
└── driver_bind.cpp                      ✚  Driver, Mode A/B, GIL, trampoline
```

**A-4 - Info + Full Results**
```
bindings/python/
├── CMakeLists.txt                       ✎  add info_bind.cpp
├── bindings.cpp                         ✎  register Info
├── config_bind.cpp                      ✎  add __repr__ for Config sub-structs
└── info_bind.cpp                        ✚  Info, tally (data,sem) tuples, __repr__
```

**A-5 - HDF5 + Tests + Docs**
```
bindings/python/
└── driver_bind.cpp                      ✎  add save() / load()
test/python/
└── test_bindings.py                     ✚
doc/
├── conf.py                              ✚  Sphinx config (doc/ already exists)
└── api.rst                              ✚
examples/python/
├── basic_run.ipynb                      ✚
├── live_plot.ipynb                      ✚
└── parametric_study.ipynb               ✚
```

---

## 14. Library Modifications (Minimal)

Feature A avoids touching the C++ library where possible.

| File | Change | Reason |
|------|--------|--------|
| `cmake/Externals.cmake` | Add pybind11 v3.0.4 FetchContent | Required for binding compilation |
| `CMakeLists.txt` (root) | Add `add_subdirectory(bindings/python)` | Enable Python build |
| `pyproject.toml` | New file at root | `pip install .` support |
| `source/include/mccore.h` | Fix `flight_path_const` comment: `[nm]` → `[atomic radii]` | Unit is wrong per `flight_path.h:24–29` |
| `source/include/ion.h` | Add `element_t(symbol)` and `element_t(Z)` constructors via `periodic_table.h` | Required for `opentrim.Element("He")` / `opentrim.Element(2)` auto-fill |
| `source/include/mcinfo.h` | Possibly minor additions as binding is developed | `mcinfo` is not a mature C++ class yet - may need changes as the wrapper is developed |

**No changes to:**
- `source/include/mcdriver.h` - no new methods needed; trampoline handles callback
- `source/lib/mccore.cpp` - no new C++ methods needed
- Anything in `source/gui/` - zero overlap

---

## 15. Code Introspection & Autocomplete

Python mechanisms that help when working with IDEs or Jupyter notebooks should be used extensively, especially for `Config` and `Info` where the user needs to remember a lot of key names.

### For `Config`

```python
repr(config)
# → Config(
#     Simulation(simulation_type=FullCascade, electronic_stopping=SRIM13, ...),
#     IonBeam(ion=He, energy=2.0 MeV, ...),
#     Run(max_no_ions=10000, threads=4, seed=42),
#     ...
#   )

config.Simulation.__doc__
# → "Wraps mccore::parameters. Fields: simulation_type, screening_type, ..."
```

### For `Info`

```python
repr(info)
# → Info [group]
#   ├── run_info [group]
#   │   ├── title [string]: "Ion Simulation"
#   │   ├── total_ion_count [uint64]: 10000
#   │   └── json_config [string]
#   ├── target [group]
#   │   ├── grid [group]
#   │   │   ├── X [real32] shape=(201,)
#   │   │   ├── Y [real32] shape=(2,)
#   │   │   └── Z [real32] shape=(2,)
#   │   └── atoms [group] (1 atoms: Fe)
#   └── tally [group]
#       ├── damage_events [group]
#       │   ├── Vacancies [tally_score] shape=(1,200,1,1)
#       │   └── ...
```

### Mechanisms used

- `__repr__` on Config, all sub-structs, Info
- `__repr_html__` on Info for Jupyter rendering (collapsible HTML tree)
- `__str__` as plain-text fallback
- `.pyi` stub files - can also be considered; enable IDE autocomplete for all classes and enum values
- `options_spec.json` and `mcinfo_spec.json` used at runtime to populate docstrings and `__repr__`

---

## 16. Testing & Documentation

### Test Plan (`test/python/test_bindings.py`)

```python
# Config tests
def test_config_default():           # all defaults set correctly
def test_config_enum_assignment():   # opentrim.Stopping.SRIM13 accepted
def test_config_validate_error():    # ValueError on bad config
def test_config_round_trip_json():   # config.to_json() → Config.from_json(s) round-trip

# Driver tests
def test_driver_mode_a():            # run() returns immediately
def test_driver_wait():              # wait() blocks until done
def test_driver_abort():             # abort() stops early
def test_driver_ion_count():         # ion_count() increases during run

# Info tests
def test_info_grid_shape():          # x grid has Nx+1 edges
def test_info_tally_shape():         # (data, sem) shape = [natoms, Nx, Ny, Nz]
def test_info_tally_normalized():    # data is per-ion (not raw counts)
def test_info_sem_positive():        # SEM >= 0 always
def test_info_repr():                # repr(info) contains group names

# HDF5 tests
def test_hdf5_round_trip():          # save → load → same ion_count
def test_hdf5_tally_match():         # loaded tallies match original
```

### Documentation

- Sphinx `autodoc` for the `opentrim` module in `doc/api.rst`
- Three example notebooks in `examples/python/`:
  - `basic_run.ipynb` - the complete example from Section 10
  - `live_plot.ipynb` - Mode B with live matplotlib update
  - `parametric_study.ipynb` - sweep over energies or materials
- Short API reference page listing `Config`, `Driver`, `Info` with all methods and properties

---

## 17. Future Work

The following are out of scope for Feature A:

| Feature | Why deferred |
|---------|-------------|
| **Direct `mccore` access** | Advanced topic. Needs careful design once the base is solid. |
| **Zero-copy numpy from `ArrayNDd`** | Unsafe: arrays can reallocate; raw tallies are unnormalized. |
| **Event streams** (pka, exit, damage) | Part of mccore; deferred with mccore access. |
| **RNG state write** (`set_rng_state`) | Direct `mccore` mutation - deferred with mccore access. Read-only `rng_state` is already available via `info["run_info"]["rng_state"]` (goes through mcinfo). |
| **Run merging** (`r1 + r2`) | Needs mcdriver-level merge support in C++; deferred with mccore access. |
| **Raw tally variance arrays** (`get_tally_var`) | mccore-level; deferred. |
| **xarray `Dataset` export** | Optional comfort layer; add after core is stable. |

---

