
```
mkdir build
cd build
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
ninja -j6
ninja install
```

Now the compiled program is in `build/OpenTRIM`

Run `mingw64-deploy.sh` to create a directory for distribution. 

```
../dist/mingw64-deploy.sh opentrim OpenTRIM opentrim-1.0.0
```

Now `build/opentrim-1.0.0` contains a full windows distribution (program, dlls etc)


## Packages
- mingw-w64-ucrt-x86_64-qt5-datavis3d
- `mingw-w64-ucrt-x86_64-eigen3-3.4.0-1-any.pkg.tar.zst`, Msys has v5.0.0 which is not supported. Version 3.4.0 must be downloaded from msys2 repo: https://repo.msys2.org/mingw/ucrt64/.