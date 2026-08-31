# TODO

## Bugs

- [X] The rpath is not correctly set the 1st time that cmake configures the project. (problem for local builds/installs to $HOME/.local)
  Solved - include(GNUInstalldirs) must be called after setting the prefix !!

- [X] (Not an actual bug but needs fixing) Monte-Carlo runs with multiple threads are not reproducible
  This is because threads race to obtain the next ion's ID to simulate and the allocation depends on OS scheduling.
  The solution could be to preallocate which ion IDs are to be simulated by each thread, e.g., with 2 threads: thread 0 will simulate even ion IDs and thread 1 odd ones.
  This can be used while merging thread output so that events are stored sequentially per ion ID.

## Functionality that needs to be completed

### Grid definition
- [ ] It should be possible to enter the grid definition in 2 ways:
  - either origin, size and Ncells, or
  - X, Y, Z grid vectors

### JSON I/O

- [ ] Write/report range of bool options as false|true
- [X] Input validation. Check all input variables that they are within their valid range

### Screened Coulomb scattering

- [ ] Fix high s (impulse approx) region in Bohr, Moliere, KrC
- [ ] gen_scattering_tbl:  
      generate directly log2(sin2thetaby2) which is used in the simulation. This will eliminate a log2() call from the simulation loop.
      Currently this is not possible due to errors in high s region for Bohr, Moliere, KrC. The log2 tables conatin NaNs 

### Flight path

- [ ] Write tables of effective lowest recoil energy and scattering angle in output file
- [X] Write critique for SRIM & MHW in doc
- [X] Remove MHW option ?

### Simulation Output Data handling

- [X] Create mcinfo or mcdata object/adaptor that provides data in a unified manner.
      E.g. multidim data Y vs X, name, description
- [X] Create a multi-dim array viewer widget, in the spirit of h5web. 
      Shows either table, 2D graph or 2D map (i.e. "spectrograph", "heatmap"). Has controls for data selection
- [X] Implement "slice selector" popup widget/dialog 

### Tests

- [ ] Complete the OpenTRIM/SRIM comparison (5 projectiles x 15 targets), primarily on damage generation
- [ ] Create a series of CTest runs that calculate one value with relatively low error (e.g. < 1%). These values will be compared with previous OpenTRIM versions. Significant differences should be checked. 
- [X] Run SRIM-FC benchmark #2 for 20000 histories 

### GUI:

- [ ] Enter grid in 2 ways (see above)
- [ ] Better help in configuration. A foldable dedicated text browser widget to show info?
- [ ] Implement UserTally options/definition in GUI
- [ ] Simulation Control Tab
      A redesign in needed as few things need changing
      - no. of Threads and RNG seed are set at the beginning of the simulation. They remain fixed when stopping/resuming 
      - no of Thread 0 = means the program decides the no of threads. This has to be indicated 
      e.g. instead of just "0" to show "0 (Auto)" in the spin-box 
      - when the sim runs, the thread spinbox deactivates and shows the actual number of threads used
      - max no of ions can be changed any time
      - The time limit should also be shown. How? keeping the control tab simple and not flooded
- [X] Getting Started
- [X] About

### Doc:
- [ ] Update install instructions 

### Dist:
- [ ] Program icon/logo
- [ ] Desktop integration


Tally:
- [X] Make std. tally tables 4D by expanding the cells axis to Nx x Ny x Nz, where Nx, Ny, Nz are the # of cells along each axis
- [X] Remove counting of recoil energy (tally::eRecoil). It is not a well-defined sum
- [X] Add a counter of Displacements (tally::cD)
  Every recoil should add +1 to this, regardless of fate (implantation = I, exit = L, replacement = R)
  V = D - R
  I = D - R - L

## Enhancements

### Core lib

- [ ] Extend E range of dEdx to 1e12 eV = 1 TeV = 2^40 eV, 
      2^4..2^40 with 4-bit analysis makes a 36*2^4=576 pts table
      with 3-bit makes 36*2^3 = 288
      dEdx data (SRIM, PASS) are tabulated from 1e3 to 1e9 eV/amu
      For U (M=238) this translates to E = 240 keV ... 0.24 TeV
      For H (M=1), E = 1 keV ... 1 GeV

- [ ] Create 3 new classes to group the MC core algorithm:
      - `propagation`: handles flight path & scattering length selection, electronic stopping & straggling
      - `scattering`: handles the scattering event
      - `damage`: implements the damage model

- [ ] Handle surface effects (sputtering etc.)

- [X] json parser: un-recognized options should create an error
  - This will aid in checking if a .json file is valid OpenTRIM options
  - With the possibility to relax the rule in the future to allow extra/new options

- [X] Improve the Variable flight path algorithm
  - Implement the steps described in the docs, i.e., pre-compute the probability that a collision is rejected

- [X] Make user-defined tallies for various events. E.g.
  - Implantation (position, atomic species)
  - Vacancy (position, atomic species)
  - Ion escape (have to distinguish backscattered/transmitted ions)
  - Recoil (PKA or other)
    - energy / damage energy
    - position
    - atom
    - vacancies generated
  - The tallies have their own mesh which can be defined in rect, spherical, cylindrical coordinates 

- Tally data:
  - [ ] Instead of keeping the total sum for a tally bin, better keep the mean value and refine it every update interval.
    Example: 
      A bin has mean value b(N)=Σx_i/N after N histories. This is stored in the 'main' tally.
      The execution thread(s) run additional ΔN histories which have mean value δb.
      The new value of b, b'=b(N+ΔN), is obtained using the following formula
        b' = b + (δb-b)*ΔN/(N+ΔN)
      The same can be done for the square

### User

- GUI: 
  - [x] Add a database of pre-defined materials with full definition: composition, density, Ed, etc
  - [x] Pressing add material presents to the user a selection/search function to discover & select
  - [] Extend the database of pre-defined materials

- CLI:
  - [X] Block Ctrl-C signal so that data is saved before the program is aborted

- General
  - [ ] Provide progress info for HDF5 i/o operations

