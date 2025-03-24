
```
mkdir build
cd build
cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Release
ninja -j6
ninja install
```

Now the compiled program is in `build/OpenTRIM`

Run `mingw64-deploy.sh` to create a directory for distribution. 

```
../dist/mingw64-deploy.sh opentrim OpenTRIM opentrim-1.0.0
```

Now `build/opentrim-1.0.0` contains a full windows distribution (program, dlls etc)