# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-build"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-subbuild/external_highfive-populate-prefix"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-subbuild/external_highfive-populate-prefix/tmp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-subbuild/external_highfive-populate-prefix/src/external_highfive-populate-stamp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-subbuild/external_highfive-populate-prefix/src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-subbuild/external_highfive-populate-prefix/src/external_highfive-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-subbuild/external_highfive-populate-prefix/src/external_highfive-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_highfive-subbuild/external_highfive-populate-prefix/src/external_highfive-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
