\page userdoc User Documentation

- \subpage install "Installation"
- \subpage cliapp "Using the command line application"
- \subpage json_config "The JSON configuration file"
- \subpage out_file "The HDF5 output archive"

\page install Installation

## Binary packages

### Linux
Binary packages and repositories for a number of **Linux** distributions (Ubuntu, RHEL, OpenSUSE, etc.) are built on the [openSUSE Build Service](https://software.opensuse.org//download.html?project=home%3Amaxiotis%3Agapost&package=opentrim). Please follow the instructions found there to install OpenTRIM on your system.

### Windows
Download the latest [binary distribution release](https://github.com/ir2-lab/OpenTRIM/releases) provided as a zip file and extract to some location. To be able to run the program from the windows command line, add the program folder to the user or system path.

## Building from source

### Linux
The project can be built and installed with `cmake`.

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

### Windows
The project can be built with [MSYS2](https://www.msys2.org/). Detailed instructions will be given in the future.

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




