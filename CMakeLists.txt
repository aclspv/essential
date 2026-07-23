cmake_minimum_required(VERSION 3.10)

enable_testing()

project(aclspv VERSION 0)

# Set this source directory as the tree root for project.
set(ae2f_ProjRoot ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "Tree root for project")
set(ae2f_BinRoot ${CMAKE_CURRENT_BINARY_DIR} CACHE STRING "Tree root for project")
set(ae2f_submod  .submod CACHE STRING "Relative path to submodule (subdirectory)")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# Execute git to fetch CoreScript when repository does not exist
set(ae2f_CoreScript_Wh ${ae2f_ProjRoot}/${ae2f_submod}/ae2f/CoreScript)
if(NOT EXISTS ${ae2f_CoreScript_Wh})
	execute_process(
		COMMAND git clone http://talkischeap.work.gd/ae2f/CoreScript
		${ae2f_CoreScript_Wh}
		)
endif()

# add subdirectory
if(NOT TARGET ae2f::CoreScript)
	add_subdirectory(${ae2f_CoreScript_Wh} ${ae2f_BinRoot}/${ae2f_submod}/ae2f/CoreScript)
endif()

ae2f_CoreLibFetchX(ae2f Core	main)
ae2f_CoreLibFetchX(ae2f Pat	master)
ae2f_CoreLibFetchX(ae2f Sys	main)

ae2f_CoreLibFetchX_DNS(KhronosGroup KhronosGroup SPIRV-Headers main https://github.com/)

find_package(Clang)
find_library(Clang_FOUND clang)
message(STATUS ${Clang_FOUND})

if(Clang_FOUND)
	message(STATUS "clang is found")
else()
	if(NOT EXISTS ${ae2f_CoreScript_Wh}/llvm)
		execute_process(
			COMMAND git clone https://github.com/llvm/llvm-project
			${ae2f_CoreScript_Wh}/llvm --depth 1
			)
	endif()

	set(LLVM_ENABLE_PROJECTS "clang")
	set(CLANG_BUILD_TOOLS OFF)
	set(LLVM_BUILD_TOOLS OFF)
	set(CLANG_ENABLE_STATIC_ANALYZER OFF)
	set(CLANG_ENABLE_ARCMT OFF)
	set(LLVM_ENABLE_ZLIBS OFF)
	set(LLVM_ENABLE_TERMINFO OFF)
	set(LLVM_ENABLE_LIBXML2 OFF)
	set(LLVM_INCLUDE_TESTS OFF)
	set(LLVM_INCLUDE_EXAMPLES OFF)
	set(LLVM_INCLUDE_BENCHMARKS OFF)
	set(LLVM_INSTALL_UTILS OFF)

endif()

file(GLOB aclspv-inc ${PROJECT_SOURCE_DIR}/inc/*)

file(GLOB_RECURSE aclspv-src ${PROJECT_SOURCE_DIR}/lib/*)

ae2f_CoreLibTentConfigCustom(
	aclspv-common	INTERFACE
	inc ae3f
	${PROJECT_SOURCE_DIR}/cfg/aclspv-common.cmake.in
	inc/aclspv.h
	inc/aclspv.hh
	)

ae2f_CoreLibTentConfigCustom(
	aclspv ${ae2f_LIBPREFIX}
	inc ae3f
	${PROJECT_SOURCE_DIR}/cfg/aclspv.cmake.in
	${aclspv-src} ${aclspv-inc} 
	)

add_subdirectory(src)

add_library(aclspv		ALIAS ae3f-aclspv)
add_library(aclspv::aclspv	ALIAS ae3f-aclspv)

target_link_libraries(ae3f-aclspv-common
	INTERFACE	libclang
	INTERFACE	ae2f::Core
	INTERFACE	ae2f::Sys
	INTERFACE	$<BUILD_INTERFACE:SPIRV-Headers::SPIRV-Headers>
	)

target_compile_definitions(ae3f-aclspv PRIVATE ACLSPVLIB=1)

target_link_libraries(ae3f-aclspv
	PRIVATE		ae3f::aclspvCore
	PUBLIC		ae3f::aclspv-common
	)

ae2f_FileRaw_init(1 100)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/testinc/.acltest)
ae2f_FileRaw_Run_One2(
	${PROJECT_SOURCE_DIR}/src/source.cl 
	${PROJECT_BINARY_DIR}/testinc/.acltest/source.h
	"SRC0"
	)

ae2f_FileRaw_Run_One2(
	${PROJECT_SOURCE_DIR}/src/ocl.h
	${PROJECT_BINARY_DIR}/testinc/.acltest/ocl.h
	"OCLBASE"
	)

add_custom_target(ae3f-aclspv-test-gen ALL
	SOURCES
	${PROJECT_BINARY_DIR}/testinc/.acltest/source.h
	${PROJECT_BINARY_DIR}/testinc/.acltest/ocl.h
	./test/inc/acltest/onefile.h
	)

add_library(ae3f-aclspv-test INTERFACE)

target_link_libraries(ae3f-aclspv-test 
	INTERFACE ae3f-aclspv
	)

add_dependencies(ae3f-aclspv-test ae3f-aclspv-test-gen)

target_include_directories(ae3f-aclspv-test 
	INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/testinc/>
	INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/test/inc/>
	)

ae2f_CoreTestTentVerbose(ae3f-aclspv-test ./test/lifespan)
ae2f_CoreTestTentVerbose(ae3f-aclspv-test ./test/util)
target_link_libraries(Test-c-ae3f-aclspv-test-scale PRIVATE ae3f::aclspvCore)

if(Clang_FOUND)
	target_include_directories(ae3f-aclspv-common INTERFACE $<BUILD_INTERFACE:${CLANG_INCLUDE_DIRS}>)
else()
	find_library(libclang_FOUND libclang)

	if(libclang_FOUND)
	else()
		add_subdirectory(
			${ae2f_CoreScript_Wh}/llvm/llvm 
			${ae2f_BinRoot}/${ae2f_submod}/llvm
			)
	endif()

	target_include_directories(ae3f-aclspv-common INTERFACE $<BUILD_INTERFACE:${ae2f_CoreScript_Wh}/llvm/clang/include>)
endif()

ae2f_UltraErrWERROR(ae3f-aclspv PRIVATE)
