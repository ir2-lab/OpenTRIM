## %1

%2

Source: https://github.com/ir2-lab/OpenTRIM

Documentation: https://ir2-lab.gitlab.io/opentrim

### Version Information

Version: %3

Git-tag: %4

Build time: %5

Compiler: %6 v%7

System: %8

### Contributors

George Apostolopoulos <<gapost@ipta.demokritos.gr>>

Eleni Mitsi <<elmitsi@ipta.demokritos.gr>>

Michael Axiotis <<axiotis@inp.demokritos.gr>>

### Credits

`OpenTRIM` draws heavily on [SRIM](http://www.srim.org/) by J.F. Ziegler, one of the first ion simulation programs created in the 80s and still widely used until today. The electronic energy loss data in `OpenTRIM` have been obtained from the [SRIM-2013](http://www.srim.org/) distribution using the provided utility `SRmodule.exe`.

Many ideas, techniques and code were taken from the following open-source ion simulation programs:

- The program [iradina](https://sourceforge.net/projects/iradina/) written by Ch. Borschel & C. Ronning and extended by J.P. Crocombette & Ch. Van Wambeke.
- The program [Corteo](http://www.lps.umontreal.ca/%7Eschiette/index.php?n=Recherche.Corteo) by F. Schiettekatte.

Furthermore, the following general open-source projects are used:

- The [Eigen](http://eigen.tuxfamily.org/) library by B. Jacob & G. Guennebaud is used for 3D vector math.

- The [Xoshiro256+](https://prng.di.unimi.it/) algorithm by D. Blackman and S. Vigna is used for random number generation.

- [JSON for Modern C++](https://github.com/nlohmann/json) by N. Lohmann is used for encoding/decoding program options to/from json.

- [cxxopts](https://github.com/jarro2783/cxxopts) by [jarro2783](https://github.com/jarro2783) is used for handling cli options.

- The [HDF5 library](https://github.com/HDFGroup/hdf5) with the [HighFive C++ interface](https://github.com/BlueBrain/HighFive) are used for saving results to the HDF5 archive.

- The [Qt C++ toolkit](https://www.qt.io/) is utilized for the GUI implementation.

### License
<br>     
     
> ```
> MIT License
>
> Copyright (c) 2024-2025, National Centre for Scientific Research "Demokritos" and OpenTRIM contributors.
>
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.
> ```
