# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zhuangyan/projects/CPP/da4qi4/daqi

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zhuangyan/projects/CPP/da4qi4/daqi

# Utility rule file for NightlyStart.

# Include the progress variables for this target.
include nlohmann_json/CMakeFiles/NightlyStart.dir/progress.make

nlohmann_json/CMakeFiles/NightlyStart:
	cd /home/zhuangyan/projects/CPP/da4qi4/daqi/nlohmann_json && /usr/bin/ctest -D NightlyStart

NightlyStart: nlohmann_json/CMakeFiles/NightlyStart
NightlyStart: nlohmann_json/CMakeFiles/NightlyStart.dir/build.make

.PHONY : NightlyStart

# Rule to build all files generated by this target.
nlohmann_json/CMakeFiles/NightlyStart.dir/build: NightlyStart

.PHONY : nlohmann_json/CMakeFiles/NightlyStart.dir/build

nlohmann_json/CMakeFiles/NightlyStart.dir/clean:
	cd /home/zhuangyan/projects/CPP/da4qi4/daqi/nlohmann_json && $(CMAKE_COMMAND) -P CMakeFiles/NightlyStart.dir/cmake_clean.cmake
.PHONY : nlohmann_json/CMakeFiles/NightlyStart.dir/clean

nlohmann_json/CMakeFiles/NightlyStart.dir/depend:
	cd /home/zhuangyan/projects/CPP/da4qi4/daqi && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zhuangyan/projects/CPP/da4qi4/daqi /home/zhuangyan/projects/CPP/da4qi4/daqi/nlohmann_json /home/zhuangyan/projects/CPP/da4qi4/daqi /home/zhuangyan/projects/CPP/da4qi4/daqi/nlohmann_json /home/zhuangyan/projects/CPP/da4qi4/daqi/nlohmann_json/CMakeFiles/NightlyStart.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : nlohmann_json/CMakeFiles/NightlyStart.dir/depend
