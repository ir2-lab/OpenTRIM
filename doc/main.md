\mainpage

A C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on calculation of radiation damage.

It consists of the following components:

- \ref opentrim_gui "opentrim-gui" : A GUI tool to configure, run and evaluate simulations
- The \ref cliapp "`opentrim` command line program"
- The \ref MC with all the C++ ion transport code, which can be linked to by external applications
- A \ref XS "library of C++ classes" for screened Coulomb scattering calculations 
- The \ref dedx containing tables of electronic stopping data

The documentation is divided in the following parts

- \ref physics
- \ref userdoc
- \ref <a href="topics.html">Programming Documentation</a>

![opentrim-gui screenshot](./dist/screenshot.png)

\section Contributors

George Apostolopoulos <gapost@ipta.demokritos.gr> \n
Eleni Mitsi <elmitsi@ipta.demokritos.gr> \n
Michael Axiotis <axiotis@inp.demokritos.gr> \n

\section credits Credits

`OpenTRIM` draws heavily on [SRIM](http://www.srim.org/) by [J.F. Ziegler](ziegler[at]srim.org), one of the first ion simulation programs created in the 80s and still widely used until today. The electronic energy loss data in `OpenTRIM` have been obtained from the [SRIM-2013](http://www.srim.org/) distribution using the provided utility `SRmodule.exe`.

Many ideas, techniques and code were taken from the following open-source ion simulation programs:

- The program [iradina](https://sourceforge.net/projects/iradina/) written by Ch. Borschel & C. Ronning and extended by J.P. Crocombette & Ch. Van Wambeke.
- The program [Corteo](http://www.lps.umontreal.ca/%7Eschiette/index.php?n=Recherche.Corteo) by F. Schiettekatte.

The [Eigen](http://eigen.tuxfamily.org/) library by B. Jacob & G. Guennebaud is used for 3D vector math.

The [Xoshiro256+](https://prng.di.unimi.it/) algorithm by D. Blackman and S. Vigna is used for random number generation.

[JSON for Modern C++](https://github.com/nlohmann/json) by N. Lohmann is used for encoding/decoding program options to/from json.

[cxxopts](https://github.com/jarro2783/cxxopts) by [jarro2783](https://github.com/jarro2783) is used for handling cli options.

The [HDF5 library](https://github.com/HDFGroup/hdf5) with the [HighFive C++ interface](https://github.com/BlueBrain/HighFive) are used for saving results to the HDF5 archive.

\section License

 ```
 MIT License

 Copyright (c) 2024-2025, National Centre for Scientific Research "Demokritos" and penTRIM contributors.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 ```