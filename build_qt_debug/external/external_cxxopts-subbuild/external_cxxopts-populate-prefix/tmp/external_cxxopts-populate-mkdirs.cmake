# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-build"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-subbuild/external_cxxopts-populate-prefix"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-subbuild/external_cxxopts-populate-prefix/tmp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-subbuild/external_cxxopts-populate-prefix/src/external_cxxopts-populate-stamp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-subbuild/external_cxxopts-populate-prefix/src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-subbuild/external_cxxopts-populate-prefix/src/external_cxxopts-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-subbuild/external_cxxopts-populate-prefix/src/external_cxxopts-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_cxxopts-subbuild/external_cxxopts-populate-prefix/src/external_cxxopts-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
