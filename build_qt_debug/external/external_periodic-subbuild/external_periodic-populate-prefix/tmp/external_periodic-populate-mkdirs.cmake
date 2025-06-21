# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-build"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-subbuild/external_periodic-populate-prefix"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-subbuild/external_periodic-populate-prefix/tmp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-subbuild/external_periodic-populate-prefix/src/external_periodic-populate-stamp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-subbuild/external_periodic-populate-prefix/src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-subbuild/external_periodic-populate-prefix/src/external_periodic-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-subbuild/external_periodic-populate-prefix/src/external_periodic-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_periodic-subbuild/external_periodic-populate-prefix/src/external_periodic-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
