\page userdoc User Documentation

- \subpage install "Installation"
- \subpage cliapp "Using the command line application"
- \subpage json_config "The JSON configuration file"
- \subpage out_file "The HDF5 output archive"
- \subpage tallies

\page cliapp Using the command line application 

OpenTRIM provides a command line program which can be invoked by 

    opentrim [options] 

To see all available options run `opentrim -h`, which prints

```
Monte-Carlo simulation of ion transport in materials
Usage:
  opentrim [OPTION...]

  -n arg            Number of histories to run (overrides config input)
  -j arg            Number of threads (overrides config input)
  -s, --seed arg    random generator seed (overrides config input)
  -i, --input arg   input HDF5 file name
  -f arg            JSON config file
  -o, --output arg  output HDF5 file name (overrides config input)
  -t, --template    print a template JSON config to stdout
  -v, --version     Display version information
  -h, --help        Display short help message

```

To start a new simulation, the user must provide a 
@ref json_config "JSON-formatted configuration" either
from a file (with the `-f` option) 

    opentrim -f config.json

or from stdin. 

The program will first read the configuration. If an error or incompatible
parameter is found, an error message is printed and the program stops. 
Otherwise, it proceeds to run the simulation. In regular intervals the program
will print messages regarding the progress and the estimated completion time.

When the simulation is completed the output data are written to a 
\ref out_file "HDF5 archive". The name of the output file is defined in the
@ref json_config "JSON configuration" or with the `-o` command option.

The output file can be used in order to continue a simulation using, e.g.

   opentrim -i sim_output1.h5 -n 10000 -o sim_output2.h5

Here, the output from the 1st simulation run is extended to 10000 histories 
and the result is written to a new file.




