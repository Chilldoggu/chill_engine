# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/chilldog/C++/OpenGL/example_refactor

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/chilldog/C++/OpenGL/example_refactor

# Include any dependencies generated for this target.
include chill_engine/CMakeFiles/light.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include chill_engine/CMakeFiles/light.dir/compiler_depend.make

# Include the progress variables for this target.
include chill_engine/CMakeFiles/light.dir/progress.make

# Include the compile flags for this target's objects.
include chill_engine/CMakeFiles/light.dir/flags.make

chill_engine/CMakeFiles/light.dir/src/light.cpp.o: chill_engine/CMakeFiles/light.dir/flags.make
chill_engine/CMakeFiles/light.dir/src/light.cpp.o: chill_engine/src/light.cpp
chill_engine/CMakeFiles/light.dir/src/light.cpp.o: chill_engine/CMakeFiles/light.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/chilldog/C++/OpenGL/example_refactor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object chill_engine/CMakeFiles/light.dir/src/light.cpp.o"
	cd /home/chilldog/C++/OpenGL/example_refactor/chill_engine && /usr/local/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT chill_engine/CMakeFiles/light.dir/src/light.cpp.o -MF CMakeFiles/light.dir/src/light.cpp.o.d -o CMakeFiles/light.dir/src/light.cpp.o -c /home/chilldog/C++/OpenGL/example_refactor/chill_engine/src/light.cpp

chill_engine/CMakeFiles/light.dir/src/light.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/light.dir/src/light.cpp.i"
	cd /home/chilldog/C++/OpenGL/example_refactor/chill_engine && /usr/local/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/chilldog/C++/OpenGL/example_refactor/chill_engine/src/light.cpp > CMakeFiles/light.dir/src/light.cpp.i

chill_engine/CMakeFiles/light.dir/src/light.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/light.dir/src/light.cpp.s"
	cd /home/chilldog/C++/OpenGL/example_refactor/chill_engine && /usr/local/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/chilldog/C++/OpenGL/example_refactor/chill_engine/src/light.cpp -o CMakeFiles/light.dir/src/light.cpp.s

# Object files for target light
light_OBJECTS = \
"CMakeFiles/light.dir/src/light.cpp.o"

# External object files for target light
light_EXTERNAL_OBJECTS =

chill_engine/liblight.a: chill_engine/CMakeFiles/light.dir/src/light.cpp.o
chill_engine/liblight.a: chill_engine/CMakeFiles/light.dir/build.make
chill_engine/liblight.a: chill_engine/CMakeFiles/light.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/chilldog/C++/OpenGL/example_refactor/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library liblight.a"
	cd /home/chilldog/C++/OpenGL/example_refactor/chill_engine && $(CMAKE_COMMAND) -P CMakeFiles/light.dir/cmake_clean_target.cmake
	cd /home/chilldog/C++/OpenGL/example_refactor/chill_engine && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/light.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
chill_engine/CMakeFiles/light.dir/build: chill_engine/liblight.a
.PHONY : chill_engine/CMakeFiles/light.dir/build

chill_engine/CMakeFiles/light.dir/clean:
	cd /home/chilldog/C++/OpenGL/example_refactor/chill_engine && $(CMAKE_COMMAND) -P CMakeFiles/light.dir/cmake_clean.cmake
.PHONY : chill_engine/CMakeFiles/light.dir/clean

chill_engine/CMakeFiles/light.dir/depend:
	cd /home/chilldog/C++/OpenGL/example_refactor && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/chilldog/C++/OpenGL/example_refactor /home/chilldog/C++/OpenGL/example_refactor/chill_engine /home/chilldog/C++/OpenGL/example_refactor /home/chilldog/C++/OpenGL/example_refactor/chill_engine /home/chilldog/C++/OpenGL/example_refactor/chill_engine/CMakeFiles/light.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : chill_engine/CMakeFiles/light.dir/depend

