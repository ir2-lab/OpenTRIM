# Python bindings for OpenTRIM

Python package providing `opentrim.Config`, `opentrim.Driver`, and
`opentrim.Info` classes for running OpenTRIM ion-in-matter Monte Carlo
simulations from Python.

## Prerequisites

* Python ≥ 3.9
* `numpy` ≥ 1.21
* `libopentrim` build dependencies: a C++17-capable compiler and CMake ≥ 3.23
* `pybind11` (fetched automatically by the build backend)

## Installing the package

### Binary package (Linux only)

Distribution packages named `python3-opentrim` are built alongside the other
OpenTRIM components on the
[openSUSE Build Service](https://software.opensuse.org//download.html?project=home%3Amaxiotis%3Agapost&package=opentrim)
for a number of Linux distributions (Ubuntu, RHEL, OpenSUSE, etc.). Follow the
installation instructions found on the OBS page and then, in Python:

```python
import opentrim
```

### From source

From the repository root:

```bash
pip install .
```

This builds `libopentrim` and the bindings together using
[scikit-build-core](https://github.com/scikit-build/scikit-build-core). For
an editable install with the Jupyter extras (used by the example notebooks):

```bash
pip install -e .[jupyter] --no-build-isolation
```

## End-to-end example

```python
import opentrim

# 1. Create and configure a simulation
config = opentrim.Config()

# Inspect the default ion energy (eV)
print("default ion energy:", config.IonBeam.energy_distribution.center, "eV")

# Override some options
config.IonBeam.ion = opentrim.Element("He")
config.IonBeam.energy_distribution.center = 2e6   # 2 MeV
config.Run.max_no_ions = 10000
config.Run.seed = 42                              # fixed seed -> reproducible

config.validate()   # raises ValueError if invalid

# Optional: inspect the full JSON
print(config.to_json())

# 2. Run the simulation
sim = opentrim.Driver(config)

def on_progress(frac):
    print(f"\rprogress: {100*frac:5.1f}%", end="")

sim.run(on_progress)   # Mode B: blocking, with progress callback
print("\ndone")

# 3. Read results
info = opentrim.Info(sim)

x = info["target"]["grid"]["x"]                       # target depth grid (nm)
vacancies, sem = info["tally"]["damage_events"]["Vacancies"]  # vacancies/ion ± SEM

import matplotlib.pyplot as plt
plt.plot(x, vacancies)
plt.xlabel("x (nm)")
plt.ylabel("Vacancies / ion")

# 4. Save / load
sim.save("result.h5")

sim2 = opentrim.Driver.load("result.h5")
info2 = opentrim.Info(sim2)
```

Type stubs are included with the package, so editors such as VS Code provide
auto-completion for the `Config`, `Driver` and `Info` classes. Some example
notebooks are provided in the [`examples/python`](../../examples/python)
folder.

## Object reference

### `opentrim.Config`

| Method / attribute                                              | Description                                           |
| --------------------------------------------------------------- | ----------------------------------------------------- |
| `Config()`                                                      | Default construction                                  |
| `Config(path)`                                                  | Construct from a JSON file                            |
| `IonBeam`, `Target`, `Transport`, `Simulation`, `Run`, `Output` | Sub-structs; set options via attribute or `[]` access |
| `validate()`                                                    | Raises `ValueError` on invalid config                 |
| `to_json()`                                                     | Full config as a JSON string                          |
| `Config.from_json(s)`                                           | Classmethod: construct from a JSON string             |
| `Config.options_spec()`                                         | Classmethod: JSON spec of all options                 |

### `opentrim.Driver`

| Method                         | Description                                                     |
| ------------------------------ | --------------------------------------------------------------- |
| `Driver(config)`               | Construct and initialise from a `Config`                        |
| `run()`                        | Mode A: start simulation, return immediately; use with `wait()` |
| `run(callback, interval_ms)`   | Mode B: run on the calling thread, calling `callback(frac)`     |
| `wait()`                       | Block until a Mode A run finishes                               |
| `config()`                     | Return a copy of the active `Config`                            |
| `save(fn)` / `Driver.load(fn)` | HDF5 serialisation                                              |
| `is_running()` / `abort()`     | Lifecycle control                                               |
| `ion_count`                    | Number of ions simulated so far                                 |
| `max_ions`, `max_cpu_time`     | Run limits (readable/settable, from `Run` config)               |

### `opentrim.Info`

| Method / operator             | Description                                          |
| ----------------------------- | ---------------------------------------------------- |
| `Info(driver)`                | Build an info view over a `Driver`                   |
| `info[key]`                   | Sub-`Info` for a group, or the value for a data node |
| `v, dv = info[key]`           | For tally nodes, values and SEM errors               |
| `keys()`                      | Available keys at this level                         |
| `description`, `path`, `type` | Metadata of the current node                         |
| `Info.info_spec()`            | Classmethod: JSON spec of the full info tree         |

**Returned types:**

| Node type       | Python value                         |
| --------------- | ------------------------------------ |
| group           | `opentrim.Info` sub-tree             |
| real64 / real32 | numpy `float64` / `float32` array    |
| uint64          | numpy `uint64` array                 |
| string / json   | `str`                                |
| tally_score     | `(values, errors)` numpy array tuple |

## License

MIT — see [`COPYING`](../../COPYING).
