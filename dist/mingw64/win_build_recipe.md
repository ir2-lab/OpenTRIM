## Windows Build

OpenTRIM on windows is built with https://msys2.org in the UCRT64 environment

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
  - `mingw-w64-ucrt-x86_64-qt5-tools`
  - `mingw-w64-ucrt-x86_64-angleproject` (needed for some OpenGL dependencies of Qt5)


### Install ir2-lab dependencies

OpenTRIM depends on 3 projects of our own: 
- [libdedx](https://github.com/ir2-lab/libdedx)
- [QMatPlotWidget](https://github.com/gapost/qmatplotwidget)
- [QtDataBrowser](https://github.com/ir2-lab/QtDataBrowser)

These repos provide MSYS2/UCRT64 runtime+devel packages. Download all package files from the respective release pages and install them all together with, e.g.

```bash
pacman -U --noconfirm /path/to/folder/*.pkg.tar.zst
```

### Build OpenTRIM

Get the source from https://github.com/ir2-lab/OpenTRIM and do the cmake build as usual

```
cd opentrim
mkdir .winbuild
cd .winbuild
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target install
```

The default install location is `${CMAKE_BIN_DIR}/${PROJECT_NAME}`, i.e.,
`.winbuild/OpenTRIM`

Run [`dist/mingw64/mingw64-deploy.sh`](mingw64-deploy.sh) to create a directory for deployment. 

```
dist/mingw64/mingw64-deploy.sh .winbuild/OpenTRIM .winbuild/opentrim-1.1.0
```

Now the `opentrim-1.1.0` folder contains a full windows distribution (program, dlls etc)

To check: open the `opentrim-1.0.0` folder in windows explorer and double-click `opentri-gui`

### Create installer

An installer can be generated with Inno Setup using the script file [`dist/mingw64/opentrim.iss`](mingw64-deploy.sh). The version and the folder containing the program deployment files (`opentrim-1.0.0` in the above example)

Run the following from the windows command prompt (`ISCC.exe` must be in the path)
```
ISCC.exe  dist\mingw64\opentrim.iss
          /DMyAppVersion=1.0.0
          /DMySourceDir=..\..\build\opentrim-1.0.0
          /Obuild
```
This will create the installer executable `build\OpenTRIMSetup.exe`.  
