# TODO

## Bugs

## Functionality that needs to be completed

### Grid definition
- [ ] More flexible grid definition should be possible
  - Not only equally spaced grids along X, Y, Z
  - **Idea**: Move "/Target/cell_count" to "/Target/regions/0/cell_count", i.e. cell_count becomes a property of each region.
    - Each region requires its own equally spaced
    - the grid points at each direction is the union of the points from all regions

### Screened Coulomb scattering

- [ ] Fix high s (impulse approx) region in Bohr, Moliere, KrC
- [ ] gen_scattering_tbl:  
      generate directly log2(sin2thetaby2) which is used in the simulation. This will eliminate a log2() call from the simulation loop.
      Currently this is not possible due to errors in high s region for Bohr, Moliere, KrC. The log2 tables contain NaNs 

### Flight path

- [ ] Write tables of effective lowest recoil energy and scattering angle in output file

### Tests

- [ ] Create a series of CTest runs that calculate one value with relatively low error (e.g. < 1%). These values will be compared with previous OpenTRIM versions. Significant differences should be checked. 

### GUI:

- [ ] Simulation Control Tab
      A redesign in needed as few things need changing
      - no. of Threads and RNG seed are set at the beginning of the simulation. They remain fixed when stopping/resuming 
      - no of Thread 0 = means the program decides the no of threads. This has to be indicated 
      e.g. instead of just "0" to show "0 (Auto)" in the spin-box 
      - when the sim runs, the thread spinbox deactivates and shows the actual number of threads used
      - max no of ions can be changed any time
      - The time limit should also be shown. How? keeping the control tab simple and not flooded

### Doc:
- [ ] Update install instructions 
- [ ] Revise docs

### Dist:
- [ ] Program icon/logo
- [ ] Desktop integration

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
  - [ ] Extend the database of pre-defined materials

- General
  - [ ] Provide progress info for HDF5 i/o operations

