# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sergio/Dropbox/eos/contracts/evo/tests

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sergio/Dropbox/eos/contracts/evo/tests

# Include any dependencies generated for this target.
include CMakeFiles/unit_test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/unit_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/unit_test.dir/flags.make

CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.o: CMakeFiles/unit_test.dir/flags.make
CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.o: evolutiondex_tests.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sergio/Dropbox/eos/contracts/evo/tests/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.o -c /home/sergio/Dropbox/eos/contracts/evo/tests/evolutiondex_tests.cpp

CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sergio/Dropbox/eos/contracts/evo/tests/evolutiondex_tests.cpp > CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.i

CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sergio/Dropbox/eos/contracts/evo/tests/evolutiondex_tests.cpp -o CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.s

CMakeFiles/unit_test.dir/main.cpp.o: CMakeFiles/unit_test.dir/flags.make
CMakeFiles/unit_test.dir/main.cpp.o: main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sergio/Dropbox/eos/contracts/evo/tests/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/unit_test.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/unit_test.dir/main.cpp.o -c /home/sergio/Dropbox/eos/contracts/evo/tests/main.cpp

CMakeFiles/unit_test.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/unit_test.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sergio/Dropbox/eos/contracts/evo/tests/main.cpp > CMakeFiles/unit_test.dir/main.cpp.i

CMakeFiles/unit_test.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/unit_test.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sergio/Dropbox/eos/contracts/evo/tests/main.cpp -o CMakeFiles/unit_test.dir/main.cpp.s

# Object files for target unit_test
unit_test_OBJECTS = \
"CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.o" \
"CMakeFiles/unit_test.dir/main.cpp.o"

# External object files for target unit_test
unit_test_EXTERNAL_OBJECTS =

unit_test: CMakeFiles/unit_test.dir/evolutiondex_tests.cpp.o
unit_test: CMakeFiles/unit_test.dir/main.cpp.o
unit_test: CMakeFiles/unit_test.dir/build.make
unit_test: /home/sergio/eosio/eos/build/libraries/testing/libeosio_testing.a
unit_test: /home/sergio/eosio/eos/build/libraries/chain/libeosio_chain.a
unit_test: /home/sergio/eosio/eos/build/libraries/fc/libfc.a
unit_test: /home/sergio/eosio/eos/build/libraries/wasm-jit/Source/WAST/libWAST.a
unit_test: /home/sergio/eosio/eos/build/libraries/wasm-jit/Source/WASM/libWASM.a
unit_test: /home/sergio/eosio/eos/build/libraries/wabt/libwabt.a
unit_test: /home/sergio/eosio/eos/build/libraries/wasm-jit/Source/Runtime/libRuntime.a
unit_test: /home/sergio/eosio/eos/build/libraries/wasm-jit/Source/Platform/libPlatform.a
unit_test: /home/sergio/eosio/eos/build/libraries/wasm-jit/Source/IR/libIR.a
unit_test: /home/sergio/eosio/eos/build/libraries/softfloat/libsoftfloat.a
unit_test: /usr/lib/x86_64-linux-gnu/libcrypto.so
unit_test: /usr/lib/x86_64-linux-gnu/libssl.so
unit_test: /home/sergio/eosio/eos/build/libraries/wasm-jit/Source/Logging/libLogging.a
unit_test: /home/sergio/eosio/eos/build/libraries/chainbase/libchainbase.a
unit_test: /home/sergio/eosio/eos/build/libraries/builtins/libbuiltins.a
unit_test: /home/sergio/eosio/eos/build/libraries/fc/secp256k1/libsecp256k1.a
unit_test: /usr/lib/x86_64-linux-gnu/libgmp.a
unit_test: /lib/libboost_filesystem.a
unit_test: /lib/libboost_system.a
unit_test: /lib/libboost_chrono.a
unit_test: /lib/libboost_iostreams.a
unit_test: /lib/libboost_date_time.a
unit_test: /usr/lib/llvm-7/lib/libLLVMSupport.a
unit_test: /usr/lib/llvm-7/lib/libLLVMCore.a
unit_test: /usr/lib/llvm-7/lib/libLLVMPasses.a
unit_test: /usr/lib/llvm-7/lib/libLLVMMCJIT.a
unit_test: /usr/lib/llvm-7/lib/libLLVMDebugInfoDWARF.a
unit_test: /usr/lib/llvm-7/lib/libLLVMOrcJIT.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86CodeGen.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86AsmParser.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86AsmPrinter.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86Desc.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86Disassembler.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86Info.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86Utils.a
unit_test: /usr/lib/llvm-7/lib/libLLVMipo.a
unit_test: /usr/lib/llvm-7/lib/libLLVMInstrumentation.a
unit_test: /usr/lib/llvm-7/lib/libLLVMVectorize.a
unit_test: /usr/lib/llvm-7/lib/libLLVMIRReader.a
unit_test: /usr/lib/llvm-7/lib/libLLVMAsmParser.a
unit_test: /usr/lib/llvm-7/lib/libLLVMLinker.a
unit_test: /usr/lib/llvm-7/lib/libLLVMExecutionEngine.a
unit_test: /usr/lib/llvm-7/lib/libLLVMRuntimeDyld.a
unit_test: /usr/lib/llvm-7/lib/libLLVMAsmPrinter.a
unit_test: /usr/lib/llvm-7/lib/libLLVMGlobalISel.a
unit_test: /usr/lib/llvm-7/lib/libLLVMSelectionDAG.a
unit_test: /usr/lib/llvm-7/lib/libLLVMCodeGen.a
unit_test: /usr/lib/llvm-7/lib/libLLVMScalarOpts.a
unit_test: /usr/lib/llvm-7/lib/libLLVMAggressiveInstCombine.a
unit_test: /usr/lib/llvm-7/lib/libLLVMInstCombine.a
unit_test: /usr/lib/llvm-7/lib/libLLVMBitWriter.a
unit_test: /usr/lib/llvm-7/lib/libLLVMTarget.a
unit_test: /usr/lib/llvm-7/lib/libLLVMTransformUtils.a
unit_test: /usr/lib/llvm-7/lib/libLLVMAnalysis.a
unit_test: /usr/lib/llvm-7/lib/libLLVMProfileData.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86AsmPrinter.a
unit_test: /usr/lib/llvm-7/lib/libLLVMX86Utils.a
unit_test: /usr/lib/llvm-7/lib/libLLVMObject.a
unit_test: /usr/lib/llvm-7/lib/libLLVMBitReader.a
unit_test: /usr/lib/llvm-7/lib/libLLVMCore.a
unit_test: /usr/lib/llvm-7/lib/libLLVMMCParser.a
unit_test: /usr/lib/llvm-7/lib/libLLVMMCDisassembler.a
unit_test: /usr/lib/llvm-7/lib/libLLVMMC.a
unit_test: /usr/lib/llvm-7/lib/libLLVMBinaryFormat.a
unit_test: /usr/lib/llvm-7/lib/libLLVMDebugInfoCodeView.a
unit_test: /usr/lib/llvm-7/lib/libLLVMDebugInfoMSF.a
unit_test: /usr/lib/llvm-7/lib/libLLVMSupport.a
unit_test: /usr/lib/llvm-7/lib/libLLVMDemangle.a
unit_test: CMakeFiles/unit_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sergio/Dropbox/eos/contracts/evo/tests/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable unit_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/unit_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/unit_test.dir/build: unit_test

.PHONY : CMakeFiles/unit_test.dir/build

CMakeFiles/unit_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/unit_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/unit_test.dir/clean

CMakeFiles/unit_test.dir/depend:
	cd /home/sergio/Dropbox/eos/contracts/evo/tests && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sergio/Dropbox/eos/contracts/evo/tests /home/sergio/Dropbox/eos/contracts/evo/tests /home/sergio/Dropbox/eos/contracts/evo/tests /home/sergio/Dropbox/eos/contracts/evo/tests /home/sergio/Dropbox/eos/contracts/evo/tests/CMakeFiles/unit_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/unit_test.dir/depend
