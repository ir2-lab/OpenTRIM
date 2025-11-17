## The HDF5 output archive {#out_file}

OpenTRIM saves its output to a single HDF5 file, which contains all
results from tallies and events along with the input conditions, run
timing and other information. 

[HDF5](https://www.hdfgroup.org/solutions/hdf5/) is a widely used binary format for storing scientific data. An example output file from the \ref 3MeV_Xe_on_UO2 "3MeV Xe on UO2" test simulation can be [viewed online here](https://myhdf5.hdfgroup.org/view?url=https%3A%2F%2Fgithub.com%2Fir2-lab%2FOpenTRIM%2Fblob%2Fmain%2Ftest%2F3MeV_Xe_on_UO2.h5). (This is made possible by the [myHDF5](https://myhdf5.hdfgroup.org) tool developed at [ESRF](https://www.esrf.fr/))

The name of the output file can be set in one of the following ways
- by the \ref _Output_outfilename "Output.outfilename" JSON option
- by the `-o` option switch of the \ref cliapp "command line program". This overrides the JSON option
- In `opentrim-gui` using the "Save" or "Save As" commands

The HDF5 archive stores also the state of the simulation. Both the `opentrim-gui` and the `opentrim` command line programs can read a HDF5 output file and continue running the simulation contained in it.

For example, the following is possible:

```bash
> opentrim -f config.json -n 1000 -o f1.h5  # run a simulation with 1000 ions
> opetrim -i f1.h5 -n 2000 -o f2.h5 # continue to 2000 ions and save under a different name
```

A brief description of the file contents is given below. More detailed
information can be found within the file itself. 

Dimensions of data tables depend on:
- \f$N_{at}\f$ : # of atoms, i.e., all atoms in the target plus the
projectile. Note that the same atom species belonging to different
materials is counted as different atom. \n
The order of atoms can be seen in `/target/atoms/labels`.
- \f$N_{mat}\f$ : # of target materials
- \f$[N_x, N_y, N_z]\f$ : # of cells along the 3 axes
- \f$N_e\f$ : # of energy points for energy loss tables
- \f$N_{ev}\f$ : # of events

To reach a variable in the archive use the complete path, e.g. `/tally/energy_deposition/Ionization`.

## OpenTRIM HDF5 output archive structure

\include{doc} h5file.dox.md
