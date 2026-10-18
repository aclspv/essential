cmake_minimum_required(VERSION 3.10)

enable_testing()

project(aclspv VERSION 0)

option(aclspv_llvm_from_source	"it build llvm from source when set to true without trying to find clang." OFF)
option(aclspv_example		"build examples" OFF)
option(aclspv_example_vk	"build vulkan examples" OFF)

# Set this source directory as the tree root for project.
set(ae2f_ProjRoot ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "Tree root for project")
set(ae2f_BinRoot ${CMAKE_CURRENT_BINARY_DIR} CACHE STRING "Tree root for project")
set(ae2f_submod  .submod CACHE STRING "Relative path to submodule (subdirectory)")
set(ae2f_host "http://talkischeap.work.gd/" CACHE STRING "Default git host for fetching.")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(ae2f_CoreScript_Wh ${ae2f_ProjRoot}/${ae2f_submod}/ae2f/CoreScript)
set(llvm_Wh ${ae2f_ProjRoot}/${ae2f_submod}/llvm/llvm-project)
if(NOT EXISTS ${ae2f_CoreScript_Wh})
	execute_process(
		COMMAND git clone ${ae2f_host}/ae2f/CoreScript
		${ae2f_CoreScript_Wh} --depth 1
		)
endif()

set(ae2f_port89_Wh ${ae2f_ProjRoot}/${ae2f_submod}/ae2f/port89)
if(NOT EXISTS ${ae2f_port89_Wh})
	execute_process(
		COMMAND git clone ${ae2f_host}/ae2f/port89
		${ae2f_port89_Wh} --depth 1
		)
endif()

# add subdirectory
if(NOT TARGET ae2f::CoreScript)
	add_subdirectory(${ae2f_CoreScript_Wh} ${ae2f_BinRoot}/${ae2f_submod}/ae2f/CoreScript)
endif()

find_package(Clang)
message(STATUS ${Clang_FOUND})
if(Clang_FOUND)
else()
	unset(Clang_FOUND)
	message(STATUS "clang is not found as package")
	find_library(Clang_FOUND clang
		PATH_SUFFIXES lib
		)
endif()

message(STATUS ${Clang_FOUND})
if(Clang_FOUND)
else()
	unset(Clang_FOUND)
	message(STATUS "clang is not found as library (clang)")
	find_library(Clang_FOUND clang.lib
		PATH_SUFFIXES lib
		)
endif()

message(STATUS ${Clang_FOUND})
if(Clang_FOUND)
else()
	unset(Clang_FOUND)
	message(STATUS "clang is not found as library (clang.lib)")
	find_library(Clang_FOUND libclang
		PATH_SUFFIXES lib
		)
endif()

message(STATUS ${Clang_FOUND})
if(Clang_FOUND)
else()
	unset(Clang_FOUND)
	message(STATUS "clang is not found as library (libclang.lib)")
	find_library(Clang_FOUND libclang.lib
		PATH_SUFFIXES lib
		)
endif()

message(STATUS ${Clang_FOUND})

if(Clang_FOUND)
	message(STATUS "clang is found")
else()
	if(NOT EXISTS ${llvm_Wh})
		execute_process(
			COMMAND git clone https://github.com/llvm/llvm-project
			${llvm_Wh} --depth 1
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

	add_subdirectory(
		${llvm_Wh}/llvm
		${ae2f_BinRoot}/${ae2f_submod}/llvm/llvm-project/llvm
		)

	#	add_subdirectory(
	#		${llvm_Wh}/clang
	#		${ae2f_BinRoot}/${ae2f_submod}/llvm/llvm-project/clang
	#		)
endif()


ae2f_CoreLibFetchX(ae2f Core	main)
ae2f_CoreLibFetchX(ae2f Pat	master)
ae2f_CoreLibFetchX(ae2f Sys	main)

ae2fpreproctent_rel(port89 ${ae2f_port89_Wh} "ae2f/port89")
ae2f_CoreLibFetchX_DNS(KhronosGroup KhronosGroup SPIRV-Headers main https://github.com/)

find_path(
	spirv_inc NAMES spirv/unified1/spirv.h	REQUIRED 
	HINTS ${ae2f_ProjRoot}/${ae2f_submod}
	$ENV{PATH}

	PATH_SUFFIXES	
	include
	SPIRV-Headers/include
	KhronosGroup/SPIRV-Headers/include
	)
find_path(
	clang_inc NAMES clang-c/Index.h		REQUIRED
	HINTS ${ae2f_ProjRoot}/${ae2f_submod}/
	$ENV{PATH}

	PATH_SUFFIXES
	include
	llvm-project/include
	llvm/llvm-project/include
	)

file(GLOB_RECURSE spirv_headers ${spirv_inc}/spirv/*.h)
file(GLOB_RECURSE clang_headers ${clang_inc}/clang-c/*.h)

set(head_gen "")

foreach(header ${spirv_headers})
	file(RELATIVE_PATH header_rel ${spirv_inc} ${header})
	get_filename_component(header_rel_par ${header_rel} DIRECTORY)
	set(header_out ${PROJECT_BINARY_DIR}/3rdinc/${header_rel})
	file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/3rdinc/${header_rel_par})
	add_custom_command(
		OUTPUT ${header_out}
		DEPENDS ${header} ${port89_dep}
		COMMAND ${port89_exe} "aclspv_swrd2_t" "aclspv_wrd2_t"  ${header} ${header_out}
		)

	set(head_gen ${head_gen} ${header_out})
endforeach()

foreach(header ${clang_headers})
	file(RELATIVE_PATH header_rel ${clang_inc} ${header})
	get_filename_component(header_rel_par ${header_rel} DIRECTORY)
	set(header_out ${PROJECT_BINARY_DIR}/3rdinc/${header_rel})
	file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/3rdinc/${header_rel_par})
	add_custom_command(
		OUTPUT ${header_out}
		DEPENDS ${header} ${port89_dep}
		COMMAND ${port89_exe} "aclspv_swrd2_t" "aclspv_wrd2_t" ${header} ${header_out}
		)

	set(head_gen ${head_gen} ${header_out})
endforeach()

file(GLOB aclspv-inc ${PROJECT_SOURCE_DIR}/inc/*)
file(GLOB_RECURSE aclspv-src ${PROJECT_SOURCE_DIR}/lib/*)

ae2f_CoreLibTentConfigCustom(
	aclspv-common	INTERFACE
	inc ae3f
	${PROJECT_SOURCE_DIR}/cfg/aclspv-common.cmake.in
	inc/aclspv.h
	inc/aclspv.hh
	${head_gen}
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

if(ae2f_TEST)
	target_link_libraries(ae3f-aclspv-test 
		INTERFACE ae3f-aclspv
		)
endif()

add_dependencies(ae3f-aclspv-test ae3f-aclspv-test-gen)

target_include_directories(ae3f-aclspv-test 
	INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/testinc/>
	INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/test/inc/>
	)

ae2f_CoreTestTentVerbose(ae3f-aclspv-test ./test/compile)
ae2f_CoreTestTentVerbose(ae3f-aclspv-test ./test/util)

if(ae2f_TEST)
	target_link_libraries(Test-c-ae3f-aclspv-test-scale 
		PRIVATE ae3f::aclspvCore
		)
endif()

ae2f_UltraErrWERROR(ae3f-aclspv PRIVATE)

target_include_directories(ae3f-aclspv PUBLIC 
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/3rdinc>
	)

include(./cmake/predef.cmake)

message(STATUS "[aclspv_predef]")

aclspv_predef_switch(ae3f-aclspv "ACLSPV_OpCapabilityKernel" "1")
aclspv_predef_switch(ae3f-aclspv "ACLSPV_OpCapabilityShader" "1")
aclspv_predef_switch(ae3f-aclspv "ACLSPV_CHECK_EXTRA" "1")
aclspv_predef_switch(ae3f-aclspv "ACLSPV_SYMBOL" "1")
aclspv_predef_switch(ae3f-aclspv "ACLSPV_ATOMIC_ID" 0)
aclspv_predef_switch(ae3f-aclspv "ACLSPV_OPACC_IDX_MAX" 64)

message(STATUS ""
	"[aclspv]\n"
	"\taclspv_llvm_from_source: ${aclspv_llvm_from_source}\n"
	"\taclspv_example: ${aclspv_example}\n"
	"\taclspv_example_vk: ${aclspv_example_vk}"
	)

if(${aclspv_example})
	add_subdirectory(./example/)
endif()
