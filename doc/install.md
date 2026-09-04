# Installation {#install}

## Binary packages

Binary packages and repositories for a number of **Linux** distributions (Ubuntu, RHEL, OpenSUSE, etc.) are built on the [openSUSE Build Service](https://software.opensuse.org//download.html?project=home%3Amaxiotis%3Agapost&package=opentrim). 
There are different packages for each program component :

    opentrim
    opentrim-gui
    opentrim-libs
    opentrim-tests
    opentrim-dev
    
Please follow the installation instructions found on the [OBS page](https://software.opensuse.org//download.html?project=home%3Amaxiotis%3Agapost&package=opentrim).

On **Windows**, please download the latest [binary distribution release](https://github.com/ir2-lab/OpenTRIM/releases) provided as a zip file and extract to some location. To be able to run the program from the windows command line, add the program folder to the user path or to the global system path.

## Python bindings

The `opentrim` Python package wraps the core library (`Config`, `Driver` and
`Info` classes). It has no Qt dependency.

### From a distribution package

    # openSUSE / RHEL family
    sudo zypper install python3-opentrim
    # Debian / Ubuntu
    sudo apt install python3-opentrim

### With pip

    pip install .

This builds from source and needs a C++17 compiler, CMake ≥ 3.23 and the
OpenTRIM build dependencies. The wheel bundles `libopentrim`; HDF5 and libdedx
must be present at runtime.

### Standalone build against an installed OpenTRIM

If OpenTRIM is already installed, the bindings can be built on their own with a
system `pybind11` (≥ 2.9.1) and the python3 development headers:

    cmake -S bindings/python -B build/python \
        -DCMAKE_PREFIX_PATH=<opentrim-install-prefix> \
        -DOPENTRIM_PYTHON_INSTALL_DIR=<site-packages>
    cmake --build build/python
    cmake --install build/python

`OPENTRIM_PYTHON_INSTALL_DIR` defaults to the interpreter's `site-packages`
directory.

## Building from source

On **Linux** the project can be built and installed with `cmake`.

Tested compilers: GCC 8 and above, Clang 14. 

Clone the project using
```
> git clone git@gitlab.com:ir2-lab/opentrim.git
```
or download a tarball of the last version.

OpenTRIM has the following dependencies:
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

On **Windows** the project can be built with [MSYS2](https://www.msys2.org/). Detailed instructions will be given in the future.