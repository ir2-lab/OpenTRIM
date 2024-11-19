## Issues

- In `GUI/Save_As H5`, if there are event buffers(files) they are not copied to the new H5 
  - this is because the simulation changes output file name while the event buffers have the old name
  - possible solution: create the buffers in temp locations with arbitrary names independent of the simulation output file 

- Fix the random seed
  -  1 seed value (an int) is needed to set the state of the rng. 
  -  In case of multiple threads, use rng.long_jump to set the state in each thread
  -  Save the rng state to the h5 output file

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