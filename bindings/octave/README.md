# opentrim — GNU Octave bindings for OpenTRIM

Octave package providing `opentrim.config`, `opentrim.driver`, and
`opentrim.info` objects for running OpenTRIM ion-in-matter Monte Carlo
simulations from Octave.

## Prerequisites

* Octave ≥ 6.0
* `libopentrim` installed (default prefix: `~/.local`)
* A C++17-capable compiler and `mkoctfile`
* CMake ≥ 3.14 (used once to discover compile/link flags)

## Building the oct-files

```bash
cd bindings/octave/src
make                          # uses ~/.local as OpenTRIM install prefix
# or, if installed elsewhere:
make CMAKE_PREFIX_PATH=/opt/opentrim
```

The Makefile runs a one-time `cmake` step that calls
`find_package(OpenTRIM)` and writes `_cmake_flags/flags.mk` with the
correct `-I` / `-L` / `-l` flags.  Subsequent `make` runs are incremental.

## Installing the package

### From the published release tarball

```octave
pkg install https://github.com/gapost/opentrim/releases/download/v1.1.6/opentrim-octave-1.1.6.tar.gz
pkg load opentrim
```

### From a locally built tarball

After building the oct-files (see above):

```bash
# From the repository root:
bash dist/make_oct_package.sh 1.1.6
```

Then in Octave:

```octave
pkg install opentrim-octave-1.1.6.tar.gz
pkg load opentrim
```

## End-to-end example

```octave
pkg load opentrim

% 1. Create and configure a simulation
cfg = opentrim.config();

% Inspect the default ion energy (eV)
e0 = cfg.get('/IonBeam/energy');
printf('default ion energy: %g eV\n', e0);

% Override some options (paths follow the mcconfig JSON hierarchy)
cfg.set('/IonBeam/energy',    2.0e6);    % 2 MeV
cfg.set('/Run/max_no_ions',   10000);
cfg.set('/Run/seed',          42);       % fixed seed → reproducible

if ~cfg.validate()
  error('configuration is invalid');
end

% Optional: inspect the full JSON
printf('%s\n', cfg.to_json());

% 2. Run the simulation
d = opentrim.driver(cfg);

function on_progress(frac)
  printf('\rprogress: %5.1f%%', 100*frac);
  fflush(stdout);
endfunction

d.exec(@on_progress);
printf('\ndone\n');

% 3. Read results
res = d.info();

x = res.get('/target/grid/x');           % target depth grid (nm)

[V, dV] = res.get('/tally/damage_events/vacancies');  % vacancies/ion ± SEM
V  = squeeze(V);
dV = squeeze(dV);

plot(x, V);
xlabel('x (nm)');
ylabel('Vacancies / ion');

% 4. Save / load
d.save('result.h5');

d2 = opentrim.driver();
d2.load('result.h5');
res2 = d2.info();
```

## Object reference

### `opentrim.config`

| Method | Description |
|---|---|
| `config()` | Default construction |
| `set(path, value)` | Set option at JSON path; value auto-encoded as JSON |
| `get(path)` | Read option; returns Octave native type |
| `validate()` | Returns `true` on success |
| `to_json()` | Full config as JSON string |
| `from_json(s)` | Populate from JSON string |

### `opentrim.driver`

| Method | Description |
|---|---|
| `driver(cfg)` | Construct and initialise from `opentrim.config` |
| `exec([cb [,ms]])` | Run simulation; optional callback `cb(frac)` every `ms` ms |
| `info()` | Return `opentrim.info` result tree |
| `config()` | Return copy of active config |
| `save(fn)` / `load(fn)` | HDF5 serialisation |
| `is_running()` / `abort()` / `wait()` / `reset()` | Lifecycle control |

### `opentrim.info`

| Method | Description |
|---|---|
| `get(path)` | Retrieve data at path (see types below) |
| `[V,dV] = get(path)` | For tally nodes, also return SEM errors |
| `description([path])` | Human-readable description of a node |

**Returned types:**

| Node type | Octave value |
|---|---|
| group | `opentrim.info` sub-tree |
| real64 / real32 | double / single NDArray |
| uint64 | uint64 NDArray |
| string / json | `char` or cell array of `char` |
| tally_score | `[values, errors]` double arrays |

## License

MIT — see `COPYING`.
