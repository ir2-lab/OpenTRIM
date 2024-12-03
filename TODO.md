## Issues

## Functionality that needs to be completed

- GUI / Recent files
- GUI / Getting Started
- GUI / About

## Enhancements

- Provide progress info for HDF5 i/o operations
  
- Improve Ion Beam source definition
  - Ion
    - Symbol/Z, 
    - M
  - Energy -> Energy distribution
    - Distribution: Monoenergetic(Delta), Hat, Gaussian, Lorentzian??
    - mean
    - fwhm
  - Position -> spatial distribution
    - Source type: Point, Surface, Volume
    - Distribution: Delta, Uniform, Hat, Gaussian
    - mean (3d vector) 
    - fwhm
  - Direction -> Direction distribution
    - Monodirectional, Uniform, Hat, Gaussian
    - mean (3d vector, un-normalized)
    - fwhm (rad or srad)

- Improve geometry definition
  - Change cell_size to volume_size (size of the simulation box)
  - introduce origin (vector 3d, origin of the simulation box)
  - The regions will be also defined by an origin and a size (both 3d vectors) 

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