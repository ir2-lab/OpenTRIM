## Issues

## Functionality that needs to be completed

- GUI / Recent files
- GUI / Getting Started
- GUI / About

## Enhancements

- Load a saved simulation from the h5 output file. 
  - Stream files should append new events

- Make user-defined tallies for various events. E.g.
  - Implantation (position, atomic species)
  - Vacancy (position, atomic species)
  - Ion escape (have to distinguish backscattered/transmitted ions)
  - Recoil (PKA or other)
    - energy / damage energy
    - position
    - atom
    - vacancies generated
  - The tallies have their own mesh which can be defined in rect, spherical, cylindrical coordinates 

- Handle surface effects