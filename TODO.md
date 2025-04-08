## Issues

## Bugs

- [ ] The rpath is not correctly set the 1st time that cmake configures the project. (problem for local builds/installs to $HOME/.local)

## Functionality that needs to be completed

GUI:
- [ ] Getting Started
- [ ] About

Dist:
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

- cli:
  - [ ] Block Ctrl-C signal so that data is saved before the program is aborted 

- Tally data:
  - [ ] Instead of keeping the total sum for a tally bin, better keep the mean value and refine it every update interval.
    Example: 
      A bin has mean value b(N)=Σx_i/N after N histories. This is stored in the 'main' tally.
      The execution thread(s) run additional ΔN histories which have mean value δb.
      The new value of b, b'=b(N+ΔN), is obtained using the following formula
        b' = b + (δb-b)*ΔN/(N+ΔN)
      The same can be done for the square

- [ ] Database of known atomic data (displacement energy, FP energy, density of elements)

- GUI: 
  - [ ] In material def form, add a button to calc density based on composition (by simple mixing)

- [ ] Provide progress info for HDF5 i/o operations

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

- [ ] Handle surface effects