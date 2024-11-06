# Iradina++

A C++ Monte-Carlo code for simulating ion
transport in materials.

The emphasis is on calculation of target damage.

Iradina++ offers the following components:

- A GUI tool to configure, run and evaluate simulations 
- A command line program for batch operations 
- A C++ library with all the ion transport code, which can be linked to by external applications
- A C++ library for screened Coulomb scattering calculations 
- A C++ library for electronic stopping calculations 

Documentation can be found here: https://ir2-lab.gitlab.io/iradinapp

## Getting started

### Installation

On **Linux** the project can be built and installed with `cmake`.

The `Eigen` and `HDF5` libraries are needed for building. For the GUI
component the `Qt` libraries and the `Qwt` plotting library are also needed.

Install them with
```bash
# Ubuntu 22.04 / DEB
sudo apt install libeigen3-dev libhdf5-dev libhdf5-103-1 
# for the GUI component add the following
sudo apt install qtbase5-dev libqwt-qt5-dev libqwt-qt5-6

# RHEL 9 / RPM
sudo dnf install eigen3-devel.noarch hdf5.x86_64 hdf5-devel.x86_64
```  

The HDF5 runtime libraries are needed for running the program.

Basic build recipe is (run from project directory)

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
make install
```
The default install location is `$HOME/.local`, thus `sudo` is not required.
Override this by setting the option `-DCMAKE_INSTALL_PREFIX="/your/install/location"` when calling `cmake`. 

On **Windows** please download the latest binary distribution release provided as a zip file and extract to some location. To be able to run the program from the windows command line, add the program folder to the user or system path.

### Usage

The GUI application `ions-gui` can be invoked on the command line or by double-clicking the executable. Some simulation examples are included and can be used as templates. 

The command line program which can be invoked by 

```
> iradina++ [options] [-f config.json]
```
The program accepts a JSON-formatted configuration input either
directly from a file (with the `-f` option) or from stdin.

To see all available options run `iradina++ -h`, which prints
```
Monte-Carlo ion trasport simulation
Usage:
  iradina++ [OPTION...]

  -n arg            Number of histories to run (overrides config input)
  -j arg            Number of threads (overrides config input)
  -s, --seed arg    random generator seed (overrides config input)
  -o, --output arg  output file base name (overrides config input)
  -f arg            JSON config file
  -t, --template    pring a template JSON config file to stdout
  -v, --version     Display version information
  -h, --help        Display short help message
```

The program first checks and validates the configuration input. 

It then runs the simulation and saves the results into a HDF5 archive.

## Testing

The GUI app contains some examples that can be used for testing.

Some benchmark tests for comparison to other codes are given in folder `test/`.

The file [`test/README.md`](test/README.md) gives a short description of the benchmarks.

Folders [`test/iradina++/b1`](test/iradina++/b1) to `b7` have config files for running the benchmarks with iradina++.

The file [`test/octave/plot_benchmark.m`](test/octave/plot_benchmark.m) is an OCTAVE script which can be used for plotting results from benchmarks.

## Credits

Iradina++ draws heavily on the following similar open-source projects:

- The program [iradina](https://sourceforge.net/projects/iradina/) written by Ch. Borschel & C. Ronning and extended by J.P. Crocombette & Ch. Van Wambeke.
- The program [Corteo](http://www.lps.umontreal.ca/%7Eschiette/index.php?n=Recherche.Corteo) by F. Schiettekatte.

Electronic energy loss data has been obtained from the [SRIM-2013](http://www.srim.org/) distribution by  [J.F. Ziegler](ziegler[at]srim.org) using the provided utility `SRmodule.exe`

The [Eigen](http://eigen.tuxfamily.org/) library by B. Jacob & G. Guennebaud is used for 3D vector math.

The [Xoshiro256+](https://prng.di.unimi.it/) algorithm by D. Blackman and S. Vigna is used for random number generation.

[JSON for Modern C++](https://github.com/nlohmann/json) by N. Lohmann is used for encoding/decoding program options to/from json.

[cxxopts](https://github.com/jarro2783/cxxopts) by [jarro2783](https://github.com/jarro2783) is used for handling cli options.

The [HDF5 library](https://github.com/HDFGroup/hdf5) with the [HighFive C++ interface](https://github.com/BlueBrain/HighFive) are used for saving
results to the HDF5 archive.