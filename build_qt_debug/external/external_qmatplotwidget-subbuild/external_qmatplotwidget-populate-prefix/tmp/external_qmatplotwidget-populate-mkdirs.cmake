# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-build"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-subbuild/external_qmatplotwidget-populate-prefix"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-subbuild/external_qmatplotwidget-populate-prefix/tmp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-subbuild/external_qmatplotwidget-populate-prefix/src/external_qmatplotwidget-populate-stamp"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-subbuild/external_qmatplotwidget-populate-prefix/src"
  "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-subbuild/external_qmatplotwidget-populate-prefix/src/external_qmatplotwidget-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-subbuild/external_qmatplotwidget-populate-prefix/src/external_qmatplotwidget-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/eri/Projects/OpenTRIM/build_qt_debug/external/external_qmatplotwidget-subbuild/external_qmatplotwidget-populate-prefix/src/external_qmatplotwidget-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
