MKOCTFILE     ?= mkoctfile
CMAKE         ?= cmake
CMAKE_BUILD   := _cmake_flags

# Override if OpenTRIM is installed to a non-default prefix, e.g.:
#   make CMAKE_PREFIX_PATH=/opt/opentrim
CMAKE_PREFIX_PATH ?= $(HOME)/.local

# ---- default target must come first so 'make' without arguments works -----
TARGETS := __opentrim_config__.oct __opentrim_driver__.oct __opentrim_info__.oct \
           __opentrim_oct__.oct

.PHONY: all clean

all: $(TARGETS)

# ---- flags discovered via CMake find_package(OpenTRIM) -------------------
-include $(CMAKE_BUILD)/flags.mk

$(CMAKE_BUILD)/flags.mk: cmake_flags/CMakeLists.txt
	$(CMAKE) -S cmake_flags -B $(CMAKE_BUILD) \
	    -DCMAKE_PREFIX_PATH="$(CMAKE_PREFIX_PATH)"

%.oct: %.cpp class_handle.h $(CMAKE_BUILD)/flags.mk
	$(MKOCTFILE) --std=c++17 $(OPENTRIM_CFLAGS) $< $(OPENTRIM_LDFLAGS) -o $@

clean:
	rm -f $(TARGETS) *.o
	rm -rf $(CMAKE_BUILD)
