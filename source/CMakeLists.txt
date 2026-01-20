
# Generators build scattering table libs etc.
add_subdirectory(generators)

# Set the target names of the library (opentrim) and executable (opentrim_exe) 
set (LIB_TARGET ${PROJECT_NAME_LOWERCASE})
set (EXE_TARGET ${PROJECT_NAME_LOWERCASE}_exe)

# Build opentrim library
add_subdirectory(lib)

# Build opentrim CLI program
add_subdirectory(cli)

# Build a utility to generate options documentation
add_subdirectory(gendoc)

# Build GUI if enabled
if (OPENTRIM_BUILD_GUI)
    add_subdirectory(gui)
endif () ## BUILD GUI