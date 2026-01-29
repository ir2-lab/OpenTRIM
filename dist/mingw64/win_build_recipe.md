## Windows Build

OpenTRIM on windows is built with https://msys2.org

### Prepare the build environment

1. Install MSYS2 to default path (e.g. `C:\msys64`)
2. Open MSYS2 console and repeat-update until there are no new updates
   The command is
   ```
   pacman -Syuu
   ```
3. Install build tools in UCRT64
   Open UCRT64 console and run
   ```
   pacman -S \
     mingw-w64-ucrt-x86_64-toolchain \
     mingw-w64-ucrt-x86_64-cmake \
     mingw-w64-ucrt-x86_64-ninja
   ```
4. Install dependencies
  - `mingw-w64-ucrt-x86_64-eigen3-3.4.0-1-any.pkg.tar.zst`, 
      MSYS2 has v5.0.0 which is not supported. Version 3.4.0 must be downloaded from msys2 repo: https://repo.msys2.org/mingw/ucrt64/.
  - `mingw-w64-ucrt-x86_64-hdf5`
  - `mingw-w64-ucrt-x86_64-qt5-base` 
  - `mingw-w64-ucrt-x86_64-qt5-svg`
  - `mingw-w64-ucrt-x86_64-qwt-qt5`

   TODO: check this

### Build dependencies

OpenTRIM depends on 3 projects of our own: 
- [libdedx](https://github.com/ir2-lab/libdedx)
- [QMatPlotWidget](https://github.com/gapost/qmatplotwidget)
- [QtDataBrowser](https://github.com/ir2-lab/QtDataBrowser)


These have to be built and installed in $HOME/.local. For each of these projects, open a UCRT64 console and do:

```
cd %project_folder%
mkdir .winbuild
cd .winbuild
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target install
```

### Build OpenTRIM

Get the source from https://github.com/ir2-lab/OpenTRIM.

Do the cmake build as usual, specifying $HOME/.local as a search path for CMake's find_package

```
cd opentrim
mkdir .winbuild
cd .winbuild
cmake .. -DCMAKE_BUILD_TYPE=Release  -DCMAKE_PREFIX_PATH=$HOME/.local
cmake --build . --target install
```

The default install location is `${CMAKE_BIN_DIR}/${PROJECT_NAME}`, i.e.,
`.winbuild/OpenTRIM`

Run [`dist/mingw64/mingw64-deploy.sh`](mingw64-deploy.sh) to create a directory for distribution. 
The tool must be run from the build folder (`.winbuild` in this example)

```
../dist/mingw64/mingw64-deploy.sh ./OpenTRIM opentrim-1.1.0
```

Now the `opentrim-1.1.0` folder contains a full windows distribution (program, dlls etc)

To check: open the `opentrim-1.0.0` folder in windows explorer and double-click `opentri-gui`

### Create installer

This can be done with the NSIS project.

Look for template code in inkscape & xournal++.

TODO
