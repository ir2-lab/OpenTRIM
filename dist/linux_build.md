# OpenTRIM - build from source on Linux 

The project can be built and installed with cmake 3.23 or higher.

Tested compilers: GCC 8 and above, Clang 14. 

## 3rd-party Dependencies

OpenTRIM has the following 3rd-party dependencies:

- [Eigen v3.4.0](https://eigen.tuxfamily.org) for vector operations
- [HDF5 v1.10.7](https://www.hdfgroup.org/solutions/hdf5/) for file storage
- [Qt5](https://www.qt.io/) for the GUI program. Components: Core, Widgets, Svg
- [Qwt v6.x](https://qwt.sourceforge.io/) for GUI plotting

These are available as packages in all major distributions. They can be installed e.g. with
```bash
# Ubuntu 22.04 / DEB
sudo apt install libeigen3-dev libhdf5-dev libhdf5-103-1 
# for the GUI component add the following
sudo apt install qtbase5-dev libqt5svg5 libqwt-qt5-dev libqwt-qt5-6

# RHEL 9 / RPM
sudo dnf install eigen3-devel.noarch hdf5.x86_64 hdf5-devel.x86_64
```  

## Subprojects

There are 3 subprojects of our own that OpenTRIM depends on:
- [libdedx](https://github.com/ir2-lab/libdedx) for ion stopping data
- [QMatPlotWidget](https://github.com/gapost/qmatplotwidget) 
- [QtDataBrowser](https://github.com/ir2-lab/QtDataBrowser) for plotting results

These have to be built & installed first so that they are available when building OpenTRIM.

To build them follow the standard procedure of cloning from git, build & install with cmake. E.g. for `libdedx`:

```
git clone git@github.com:ir2-lab/libdedx.git
cd libdedx
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build
cmake --build ./build --target install
```

All projects are installed by default in `$HOME/.local`, thus `sudo` is not required. Override this by setting `-DCMAKE_INSTALL_PREFIX="/your/install/location"` in the 1st `cmake` call. 

## Build OpenTRIM

The procedure is essentially the same as for the subprojects:
```
git clone git@github.com:ir2-lab/opentrim.git
cd libdedx
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build
cmake --build ./build --target install
```

If the subprojects have been built and installed in default locations then `cmake` will find them. Otherwise, one may have to specify their locations in the `find_package()` calls in `CMakeLists.txt`. For example
```cmake
find_package(libdedx REQUIRED PATHS /your/special/path)
```

The default install location of OpenTRIM is again `$HOME/.local`. Add `$HOME/.local/bin` to the path.




