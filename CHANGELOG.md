# Changelog

All notable changes to OpenTRIM are documented here.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and the project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Material Database dialog for picking predefined materials from a JSON database.
- UserTally configuration UI in the GUI.
- Contextual Help Panel in the Config view.
- Octave bindings.

### Changed
- Rewrite of `mcinfo` and change of `mcdriver` behaviour.
- Unified `Dialogs` helper; GUI migrated to it.
- Examples reorganised under `examples/json`.
- Core config parsing and validation reworked.
- Windows CI pins the toolchain and dependency versions.

## [1.1.6] - 2026-06-09

### Fixed
- Correct `ion_id`s of ions simulated by each thread and complete missing IDs after an abort.
- Merge events ordered by history id.

### Added
- CTest post-build testing framework and GitHub CI for the Windows build.
- Reproducibility documentation and Python example notebooks/scripts for the four examples.

## [1.1.5] - 2026-05-11

### Fixed
- Region size issue in the GUI.

## [1.1.4] - 2026-05-11

### Changed
- Reworked package-building files.

## [1.1.3] - 2026-05-07

### Added
- Experimental options in `options_spec.json`.

### Changed
- Run timestamps are now ISO 8601 compliant.
- GUI window is centred on Windows.

## [1.1.2] - 2026-05-04

### Fixed
- Segfault when running with default user-tally template values.
- Gaussian angular distribution sampling (2D isotropic Gaussian).
- Periodic distance calculation in `grid1D`.
- `coord_sys` anti-parallel check and tally cell count for 3D grids.
- Unknown JSON config keys are now caught before the simulation runs.

### Added
- CLI saves partial results on Ctrl-C (SIGINT).
- Documentation for installing OpenTRIM on WSL.

### Changed
- Use HighFive v3.3.0; removed `<charconv>` dependency.

## [1.1.1] - 2025-12-12

### Changed
- Improved user-tally bin definition.
- CMake dependency bumped for external projects; Qt5Svg required for deb packages.
- Package-building scripts updated.

## [1.1.0] - 2025-11-17

### Added
- User-defined tallies (`user_tally`): multi-dimensional binning in multiple
  coordinate systems, with HDF5 output. Multiple tallies per run.
- `mcinfo` class exposing target/atom/run info; config templates.
- `mcdriver::version_info()` including `git_tag`.
- 3D simulation-box view and `QDataBrowser` in the GUI.
- `test/srim_comp` and `test/xs_corteo` benchmarks.

### Changed
- Electronic stopping tables now use an external `libdedx` (srim95, srim13, dpass).
- Corteo/IEEE-754 indexing split into an external library.
- Screened-Coulomb scattering built as an external project.
- Replaced `cxxopts` with CLI11; reorganised the CMake build into subfolders.
- Reworked cascade and damage event handling.

### Removed
- MHW / MW flight-path options; only constant and variable flight paths remain.

## [1.0.3] - 2025-06-13

### Fixed
- Bug in `flight_path_calc`.
- Bug loading HDF5 output files.

### Added
- `T_dam` comparison with SRIM / Lindhard; reorganised benchmark scripts.

## [1.0.2] - 2025-04-29

Maintenance release (version bump only).

## [1.0.1] - 2025-04-29

### Added
- Separate `mcconfig` class.
- GUI "About" tab and getting-started content.
- Reading config from stdin (CLI).
- Automatic generation of HDF5 output documentation.

### Changed
- Redesigned cross-section classes; renamed IPP flight path to `FullMC`.
- Locale-independent number formatting.
- Improved flight-path and impact-parameter sampling.

## [1.0.0] - 2025-03-24

Initial release. A C++ Monte-Carlo code for ion transport in materials with an
emphasis on radiation-damage calculation, shipped as three components:
`libopentrim` (the core Monte-Carlo library), `opentrim` (batch CLI, 
JSON config input, HDF5 archive output) and `opentrim-gui` (Qt tool to configure, 
run and evaluate simulations). Includes screened Coulomb scattering with several 
potentials (ZBL, Moliere, ...), electronic stopping and straggling from 
SRIM-2013 data, Corteo-style tabulated cross sections, multi-threading, 
and a set of benchmarks against SRIM and iradina.

[Unreleased]: https://github.com/ir2-lab/OpenTRIM/compare/v1.1.6...HEAD
[1.1.6]: https://github.com/ir2-lab/OpenTRIM/compare/v1.1.5...v1.1.6
[1.1.5]: https://github.com/ir2-lab/OpenTRIM/compare/v1.1.4...v1.1.5
[1.1.4]: https://github.com/ir2-lab/OpenTRIM/compare/v1.1.3...v1.1.4
[1.1.3]: https://github.com/ir2-lab/OpenTRIM/compare/v1.1.2...v1.1.3
[1.1.2]: https://github.com/ir2-lab/OpenTRIM/compare/v1.1.1...v1.1.2
[1.1.1]: https://github.com/ir2-lab/OpenTRIM/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/ir2-lab/OpenTRIM/compare/v1.0.3...v1.1.0
[1.0.3]: https://github.com/ir2-lab/OpenTRIM/compare/v1.0.2...v1.0.3
[1.0.2]: https://github.com/ir2-lab/OpenTRIM/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/ir2-lab/OpenTRIM/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/ir2-lab/OpenTRIM/releases/tag/v1.0.0
