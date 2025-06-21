# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-build"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-subbuild/external_json-populate-prefix"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-subbuild/external_json-populate-prefix/tmp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-subbuild/external_json-populate-prefix/src/external_json-populate-stamp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-subbuild/external_json-populate-prefix/src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-subbuild/external_json-populate-prefix/src/external_json-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-subbuild/external_json-populate-prefix/src/external_json-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_json-subbuild/external_json-populate-prefix/src/external_json-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
