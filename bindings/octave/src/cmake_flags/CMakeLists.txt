cmake_minimum_required(VERSION 3.14)
project(opentrim_oct_flags NONE)

find_package(OpenTRIM REQUIRED)

# ---------------------------------------------------------------------------
# Helper: resolve IMPORTED_LOCATION, trying configuration-suffixed variants
# before the generic property (which is often unset for multi-config installs).
# ---------------------------------------------------------------------------
function(get_imported_location result tgt)
    foreach(cfg RELEASE RELWITHDEBINFO MINSIZEREL DEBUG)
        get_target_property(_loc ${tgt} IMPORTED_LOCATION_${cfg})
        if(_loc)
            set(${result} "${_loc}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
    get_target_property(_loc ${tgt} IMPORTED_LOCATION)
    set(${result} "${_loc}" PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------
# Include directories: get the raw property then strip generator expressions.
# CMake 3.23+ FILE_SET HEADERS adds a $<BUILD_INTERFACE:...> entry even on
# installed imported targets; filter those out so only plain paths remain.
# ---------------------------------------------------------------------------
get_target_property(_incdirs OpenTRIM::opentrim INTERFACE_INCLUDE_DIRECTORIES)
set(_clean_incdirs "")
foreach(_dir ${_incdirs})
    if(NOT "${_dir}" MATCHES "^\\$<")
        list(APPEND _clean_incdirs "${_dir}")
    endif()
endforeach()
list(REMOVE_DUPLICATES _clean_incdirs)

# ---------------------------------------------------------------------------
# Library location → -L / -l / -Wl,-rpath flags
# ---------------------------------------------------------------------------
get_imported_location(_lib_loc OpenTRIM::opentrim)
if(NOT _lib_loc)
    message(FATAL_ERROR
        "Could not determine IMPORTED_LOCATION for OpenTRIM::opentrim. "
        "Make sure libopentrim is installed and OpenTRIM is findable via "
        "CMAKE_PREFIX_PATH.")
endif()
cmake_path(GET _lib_loc PARENT_PATH _lib_dir)
cmake_path(GET _lib_loc STEM LAST_ONLY _lib_stem)
string(REGEX REPLACE "^lib" "" _lib_name "${_lib_stem}")

# ---------------------------------------------------------------------------
# Transitive interface dependencies: Eigen3 (header-only) and libdedx
# ---------------------------------------------------------------------------
get_target_property(_iface_libs OpenTRIM::opentrim INTERFACE_LINK_LIBRARIES)
if(NOT _iface_libs)
    set(_iface_libs "")
endif()

set(_extra_cflags "")
set(_extra_ldflags "")

foreach(_dep ${_iface_libs})
    if(NOT TARGET ${_dep})
        if("${_dep}" MATCHES "^-")
            string(APPEND _extra_ldflags " ${_dep}")
        endif()
        continue()
    endif()

    get_target_property(_dep_type ${_dep} TYPE)

    # Always collect include dirs from every dep (header-only or not)
    get_target_property(_dep_incdirs ${_dep} INTERFACE_INCLUDE_DIRECTORIES)
    if(_dep_incdirs)
        foreach(_dir ${_dep_incdirs})
            if(NOT "${_dir}" MATCHES "^\\$<")
                list(APPEND _clean_incdirs "${_dir}")
            endif()
        endforeach()
    endif()

    if(NOT _dep_type STREQUAL "INTERFACE_LIBRARY")
        # Shared/static lib: also collect -L / -l flags
        get_imported_location(_dep_loc ${_dep})
        if(_dep_loc)
            cmake_path(GET _dep_loc PARENT_PATH _dep_dir)
            cmake_path(GET _dep_loc STEM LAST_ONLY _dep_stem)
            string(REGEX REPLACE "^lib" "" _dep_name "${_dep_stem}")
            string(APPEND _extra_ldflags " -L${_dep_dir} -l${_dep_name}")
        endif()
    endif()
endforeach()

list(REMOVE_DUPLICATES _clean_incdirs)
list(JOIN _clean_incdirs " -I" _inc_str)
string(PREPEND _inc_str "-I")

# ---------------------------------------------------------------------------
# Write flags.mk
# ---------------------------------------------------------------------------
set(_cflags  "${_inc_str}${_extra_cflags}")
set(_ldflags "-L${_lib_dir} -Wl,-rpath,${_lib_dir} -l${_lib_name}${_extra_ldflags}")

file(WRITE "${CMAKE_BINARY_DIR}/flags.mk"
    "OPENTRIM_CFLAGS  := ${_cflags}\n"
    "OPENTRIM_LDFLAGS := ${_ldflags}\n"
)

message(STATUS "flags.mk written to ${CMAKE_BINARY_DIR}/flags.mk")
message(STATUS "  CFLAGS  = ${_cflags}")
message(STATUS "  LDFLAGS = ${_ldflags}")
