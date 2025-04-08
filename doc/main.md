\mainpage

A C++ Monte-Carlo code for simulating ion transport in materials with an emphasis on calculation of radiation damage.

It consists of the following components:

- \ref guiapp "`opentrim-gui`" : A GUI tool to configure, run and evaluate simulations
- The \ref cliapp "`opentrim` command line program"
- The \ref MC with all the C++ ion transport code, which can be linked to by external applications
- A \ref XS "library of C++ classes" for screened Coulomb scattering calculations 
- The \ref dedx containing tables of electronic stopping data

The documentation is divide in the following parts

- \ref physics
- \ref userdoc
- \ref <a href="topics.html">Programming Documentation</a>

![opentrim-gui screenshot](./dist/screenshot.png)

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