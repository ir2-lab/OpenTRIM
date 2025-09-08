# TODO

## Bugs

- [X] The rpath is not correctly set the 1st time that cmake configures the project. (problem for local builds/installs to $HOME/.local)
  Solved - include(GNUInstalldirs) must be called after setting the prefix !!

## Functionality that needs to be completed

### Screened Coulomb scattering

- [ ] Fix high s (impulse approx) region in Bohr, Moliere, KrC
- [ ] gen_scattering_tbl:  generate log2(sin2thetaby2). Currently not possible due to errors in high s region for Bohr, Moliere, KrC 

### Flight path

- [ ] Write critique for SRIM & MHW in doc
- [ ] Remove MHW option ?

### Tests

- [ ] Run SRIM-FC & Iradina benchmark #2 for 20000 histories 
- [ ] Run SRIM & Iradina QC benchmarks and compare 

### GUI:

- [X] Getting Started
- [X] About

### Dist:
- [ ] Program icon/logo
- [ ] Desktop integration

- [ ] Implement function to set limit on ion histories or time to run
  E.g., the "Ions to run" label can become a ToolButton, allowing the user
  to set the limit either on the # of ions or in time

Tally:
- [X] Remove counting of recoil energy (tally::eRecoil). It is not a well-defined sum
- [X] Add a counter of Displacements (tally::cD)
  Every recoil should add +1 to this, regardless of fate (implantation = I, exit = L, replacement = R)
  V = D - R
  I = D - R - L

## Enhancements

### Core lib

- [ ] json parser: un-recognized options should create an error
  - This will aid in checking if a .json file is valid OpenTRIM options
  - With the possibility to relax the rule in the future to allow extra/new options

- [x] Improve the FullMC flight path algorithm
  - Implement the steps described in the docs, i.e., pre-compute the probability that a collision is rejected

- [ ] Make user-defined tallies for various events. E.g.
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

- [ ] Handle surface effects (sputtering etc.)

- [ ] Database of known atomic data (displacement energy, FP energy, density of elements)

### User

- GUI: 
  - [ ] In material def form, add a button to calc density based on composition (by simple mixing)

- CLI:
  - [ ] Block Ctrl-C signal so that data is saved before the program is aborted

- General
  - [ ] Provide progress info for HDF5 i/o operations

