\page userdoc User Documentation

- \subpage install "Installation"
- \subpage cliapp "Using the command line application"
- \subpage json_config "The JSON configuration file"
- \subpage out_file "The HDF5 output archive"

\page install Installation

### Binary packages

Binary packages and repositories for a number of **Linux** distributions (Ubuntu, RHEL, OpenSUSE, etc.) are built on the [openSUSE Build Service](https://software.opensuse.org//download.html?project=home%3Amaxiotis%3Agapost&package=opentrim). Please follow the instructions found there to install OpenTRIM on your system.

On **Windows**, please download the latest [binary distribution release](https://github.com/ir2-lab/OpenTRIM/releases) provided as a zip file and extract to some location. To be able to run the program from the windows command line, add the program folder to the user or system path.

### Building from source

On **Linux** the project can be built and installed with `cmake`.

Tested compilers: GCC 8 and above, Clang 14. 

Clone the project using
```
> git clone git@gitlab.com:ir2-lab/opentrim.git
```
or download a tarball of the last version.

To build OpenTRIM you will need the following libraries:
- [Eigen v3.4.0](https://eigen.tuxfamily.org) for vector operations
- [HDF5 v1.10.7](https://www.hdfgroup.org/solutions/hdf5/) for file storage
- [Qt5 or Qt6](https://www.qt.io/) for the GUI program
- [Qwt v6.x](https://qwt.sourceforge.io/) for plotting

They can be installed with
```bash
# Ubuntu 22.04 / DEB
sudo apt install libeigen3-dev libhdf5-dev libhdf5-103-1 
# for the GUI component add the following
sudo apt install qtbase5-dev libqt5svg5 libqwt-qt5-dev libqwt-qt5-6

# RHEL 9 / RPM
sudo dnf install eigen3-devel.noarch hdf5.x86_64 hdf5-devel.x86_64
```  

Basic build recipe (run from project directory):

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
ninja
ninja install
```
The default install location is `$HOME/.local`, thus `sudo` is not required.
Override this by setting the option `-DCMAKE_INSTALL_PREFIX="/your/install/location"` when calling `cmake`. 

On **Windows** the project can be built with [MSYS2](https://www.msys2.org/). Detailed instructions will be given in the future.

\page cliapp Using the command line application 

OpenTRIM provides a command line program which can be invoked by 

    opentrim [options] [-f config.json]

The program accepts a @ref json_config "JSON-formatted configuration input" either
directly from a file (with the `-f` option) or from stdin.

To see all available options run `opentrim -h`, which prints

```
Monte-Carlo ion trasport simulation
Usage:
  opentrim [OPTION...]

 -n arg            Number of histories to run (overrides config input)
 -j arg            Number of threads (overrides config input)
 -s, --seed arg    random generator seed (overrides config input)
 -o, --output arg  output file base name (overrides config input)
 -f arg            JSON config file
 -t, --template    pring a template JSON config to stdout
 -v, --version     Display version information
 -h, --help        Display short help message
```

The program first checks and validates the configuration input. 

It then runs the simulation and saves the results into a HDF5 archive.

\page out_file The HDF5 output archive

The simulation produces a single HDF5 output file which contains all results from tallies and events along with the input conditions, run timing and other information. 
 
A brief description of the file contents is given here. More detailed information can be found within the file itself. 

Dimensions of tables depend on:
- \f$N_{at}\f$ : # of atoms, i.e., all atoms in the target plus the projectile. Note that the same atom species belonging to different materials is counted as different atom. 
- \f$N_{mat}\f$ : # of target materials
- \f$N_x, N_y, N_z\f$ : # of grid points along the 3 axes
- \f$N_c=(N_x-1)(N_y-1)(N_z-1)\f$ : # of target cells
- \f$N_e\f$ : # of energy points for energy loss tables
- \f$N_{ev}\f$ : # of events

**HDF5 output archive structure** 

- `/` Root folder
  - `config_json`: [1x1], JSON formatted simulation options (string)
  - `title`: [1x1], user supplied simulation title (string)
  - `variable_list`: [1x1] list of variables in file (string)
  - `atom/`
    - `Ed`: [\f$N_{at}\times 1\f$], displacement energy [eV]
    - `El`: [\f$N_{at}\times 1\f$], lattice binding energy [eV]
    - `Er`: [\f$N_{at}\times 1\f$], replacement energy [eV]
    - `Es`: [\f$N_{at}\times 1\f$], surface binding energy [eV]
    - `M`: [\f$N_{at}\times 1\f$], atomic mass
    - `Z`: [\f$N_{at}\times 1\f$], atomic number
    - `label`: [\f$N_{at}\times 1\f$], labels = [Atom (Chemical name)] in [Material]
    - `name`: [\f$N_{at}\times 1\f$], Chemical names
  - `eels/`
    - `dEdx`: [\f$N_{at}\times N_{mat}\times N_e\f$], dEdx values [eV/nm]
    - `dEdx_erg`: [\f$N_e \times 1\f$], dEdx table energy values [eV]
    - `dEstrag`: [\f$N_{at}\times N_{mat}\times N_e\f$], straggling values [eV/nm^(1/2)]
    - `ipmax`: [\f$N_{at}\times N_{mat}\times N_e\f$], max impact parameter [nm]
    - `mfp`: [\f$N_{at}\times N_{mat}\times N_e\f$], ion mean free [nm]
  - `exit_events/`
    - `column_descriptions`: [10x1], event data column descriptions
    - `column_names`: [10x1], event data column names
    - `event_data`: [\f$N_{ev}\times 10\f$]
  - `grid/`
    - `X`: [\f$N_x\times 1\f$], x-axis grid
    - `Y`: [\f$N_y\times 1\f$], y-axis grid
    - `Z`: [\f$N_z\times 1\f$], z-axis grid
    - `cell_xyz`: [\f$3\times N_c\f$], cell center \f$(x,y,z)\f$ coordinates
  - `material/`
    - `atomic_density`: [\f$N_m\times 1\f$], atomic density [at/nm^3]
    - `atomic_radius`: [\f$N_m\times 1\f$], atomic radius [nm]
    - `mass_density`: [\f$N_m\times 1\f$], mass density [g/cm^3]
    - `name`: [\f$N_m\times 1\f$], names of materials
  - `pka_events/`
    - `column_descriptions`: [8x1], event data column descriptions
    - `column_names`: [8x1], event data column names
    - `event_data`: [\f$N_{ev}\times 8\f$]
  - `run_stat/`
    - `Nh`: [1x1], # of histories
    - `end_time`: [1x1], finish time/date (string)
    - `ips`: [1x1], ion histories per second
    - `process_cpu_time`: [1x1], total cpu time [s]
    - `start_time`: [1x1], start time/date (string)
  - `tally/`  The score tables from the \ref tally object
    - `damage/`
      - `Tdam`: [\f$N_{at}\times N_c\f$], Damage energy [eV]
      - `Tdam_LSS`: [\f$N_{at}\times N_c\f$], Damage energy estimated by the LSS approximation [eV]
      - `Vnrt`: [\f$N_{at}\times N_c\f$], Vacancies per the NRT model using Tdam
      - `Vnrt_LSS`: [\f$N_{at}\times N_c\f$], Vacancies per the NRT model using Tdam_LSS
    - `defects/`
      - `Implantations`: [\f$N_{at}\times N_c\f$], Implantations
      - `Lost`: [\f$N_{at}\times N_c\f$], Ions that exit the simulation volume
      - `PKAs`: [\f$N_{at}\times N_c\f$], PKAs
      - `Replacements`: [\f$N_{at}\times N_c\f$], Replacements
      - `Vacancies`: [\f$N_{at}\times N_c\f$], Vacancies
    - `energy_deposition/` (see also \ref energy_part)
      - `Ionization`: [\f$N_{at}\times N_c\f$0], Energy deposited to electron ionization [eV]
      - `Lost`: [\f$N_{at}\times N_c\f$], Energy lost due to ions exiting the simulation [eV]
      - `PKA`: [\f$N_{at}\times N_c\f$], PKA recoil energy [eV]
      - `Phonons`: [\f$N_{at}\times N_c\f$], Energy deposited to the lattice [eV], Phonons = Stored+Lattice
      - `Stored`: [\f$N_{at}\times N_c\f$], Energy stored in lattice defects [eV]
      - `Lattice`: [\f$N_{at}\times N_c\f$], Thermal energy deposited to the lattice [eV]
    - `ion_stat/`
      - `collisions`: [\f$N_{at}\times N_c\f$], ion collisions
      - `flight_path`: [\f$N_{at}\times N_c\f$], flight path [nm]
    - `totals/`
      - `data`: \f$[19\times 1]\f$ for each tally, sum of scores from all atoms and all cells
      - `column_names`: name of each column in `data`	
  - `version_info/`
    - `build_system`: [1x1], build operating system
    - `build_time`: [1x1], build timestamp
    - `compiler`: [1x1], compiler id
    - `compiler_version`: [1x1], compiler version
    - `version`: [1x1], OpenTRIM version

To reach a variable in the archive use the complete path, e.g. `/tally/damage/Tdam`.

Each number in the tally tables is a mean value over all histories.
For each tally table the standard error of the mean (SEM) is also included. This is a separate table of equal dimension and with the same name plus the ending `_sem`. E.g., for the table `/tally/damage/Tdam` there is also `/tally/damage/Tdam_sem`.

The definition of SEM employed is as follows: if \f$x_i\f$ is the contribution to the table entry \f$x\f$ from the \f$i\f$-th ion history, then the mean, \f$\bar{x}\f$, and the SEM, \f$\f$\sigma_{\bar{x}}\f$\f$, listed in the output file are calculated as:
$$
\bar{x} = \frac{1}{N_h} \sum_i {x_i}
$$
$$
\sigma_{\bar{x}}^2 = \frac{1}{N_h(N_h-1)} \sum_i { (x_i - \bar{x})^2 }=
\frac{\bar{{x^2}} - \bar{x}^2}{N_h-1}
$$

\see tally

