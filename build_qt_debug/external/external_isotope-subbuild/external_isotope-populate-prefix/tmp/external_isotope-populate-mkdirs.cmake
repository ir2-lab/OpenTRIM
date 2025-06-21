# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-build"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-subbuild/external_isotope-populate-prefix"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-subbuild/external_isotope-populate-prefix/tmp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-subbuild/external_isotope-populate-prefix/src/external_isotope-populate-stamp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-subbuild/external_isotope-populate-prefix/src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-subbuild/external_isotope-populate-prefix/src/external_isotope-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-subbuild/external_isotope-populate-prefix/src/external_isotope-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_isotope-subbuild/external_isotope-populate-prefix/src/external_isotope-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
