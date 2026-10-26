cmake_minimum_required(VERSION 3.10)

enable_testing()

project(aclspv VERSION 0)

option(aclspv_llvm_from_source	"it build llvm from source when set to true without trying to find clang." OFF)
option(aclspv_essential_shared	"build aclspv::assential as shared object" ON)
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

ae2f_CoreLibTentConfigCustom(
	common	INTERFACE
	inc aclspv
	${PROJECT_SOURCE_DIR}/cfg/aclspv-common.cmake.in
	inc/aclspv.h
	inc/aclspv.hh
	${head_gen}
	)

target_include_directories(aclspv-common
	INTERFACE ${spirv_inc}
	INTERFACE ${clang_inc}
	)

add_subdirectory(src)
include(./cmake/abitent.cmake)

aclspv_abitent(aclconst)
aclspv_abitent(aclcur)
aclspv_abitent(aclexpr)
aclspv_abitent(aclemit)
aclspv_abitent(aclmem)
aclspv_abitent(aclty)
aclspv_abitent(aclval)
aclspv_abitent(util)

aclspv_abitent(essential-public)

file(GLOB aclspv-inc ${PROJECT_SOURCE_DIR}/inc/*)
file(GLOB_RECURSE aclspv-src ${PROJECT_SOURCE_DIR}/lib/*)

target_link_libraries(aclspv-essential-public
	PRIVATE	$<BUILD_INTERFACE:aclspv::common>
	)

if(${aclspv_essential_shared})
	ae2f_CoreLibTentConfigCustom(
		essential	SHARED
		inc			aclspv
		${PROJECT_SOURCE_DIR}/cfg/aclspv-essential.cmake.in
		)

	set(ACLSPV_SHARED 1)
else()
	ae2f_CoreLibTentConfigCustom(
		essential	STATIC
		inc			aclspv
		${PROJECT_SOURCE_DIR}/cfg/aclspv-essential.cmake.in
		)

	set(ACLSPV_SHARED 0)
endif()

target_link_libraries(aclspv-essential
	PUBLIC	aclspv::common
	PRIVATE	$<BUILD_INTERFACE:aclspv::aclconst>
	PRIVATE	$<BUILD_INTERFACE:aclspv::aclcur>
	PRIVATE	$<BUILD_INTERFACE:aclspv::aclexpr>
	PRIVATE	$<BUILD_INTERFACE:aclspv::aclemit>
	PRIVATE	$<BUILD_INTERFACE:aclspv::aclmem>
	PRIVATE	$<BUILD_INTERFACE:aclspv::aclty>
	PRIVATE	$<BUILD_INTERFACE:aclspv::aclval>
	PRIVATE	$<BUILD_INTERFACE:aclspv::util>
	PRIVATE $<BUILD_INTERFACE:aclspv::essential-public>
	)

target_link_libraries(aclspv-common
	INTERFACE	libclang
	INTERFACE	ae2f::Core
	INTERFACE	ae2f::Sys
	INTERFACE	$<BUILD_INTERFACE:SPIRV-Headers::SPIRV-Headers>
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

add_custom_target(aclspv-essential-test-gen ALL
	SOURCES
	${PROJECT_BINARY_DIR}/testinc/.acltest/source.h
	${PROJECT_BINARY_DIR}/testinc/.acltest/ocl.h
	./test/inc/acltest/onefile.h
	)

add_library(aclspv-essential-test INTERFACE)

if(ae2f_TEST)
	target_link_libraries(aclspv-essential-test 
		INTERFACE aclspv::essential
		)
endif()

add_dependencies(aclspv-essential-test aclspv-essential-test-gen)

target_include_directories(aclspv-essential-test
	INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/testinc/>
	INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/test/inc/>
	)

ae2f_CoreTestTentVerbose(aclspv-essential-test ./test/compile)
ae2f_CoreTestTentVerbose(aclspv-essential-test ./test/util)

if(ae2f_TEST)
	target_link_libraries(Test-c-aclspv-essential-test-scale 
		PRIVATE	$<BUILD_INTERFACE:aclspv::aclconst>
		PRIVATE	$<BUILD_INTERFACE:aclspv::aclcur>
		PRIVATE	$<BUILD_INTERFACE:aclspv::aclexpr>
		PRIVATE	$<BUILD_INTERFACE:aclspv::aclemit>
		PRIVATE	$<BUILD_INTERFACE:aclspv::aclmem>
		PRIVATE	$<BUILD_INTERFACE:aclspv::aclty>
		PRIVATE	$<BUILD_INTERFACE:aclspv::aclval>
		PRIVATE	$<BUILD_INTERFACE:aclspv::util>
		PRIVATE $<BUILD_INTERFACE:aclspv::common-essential>
		)
endif()

include(./cmake/predef.cmake)

message(STATUS "[aclspv_essential_predef]")


aclspv_predef_switch(aclspv-common		INTERFACE "ACLSPV_SHARED"		1)
aclspv_predef_switch(aclspv-common-essential 	INTERFACE "ACLSPV_OpCapabilityKernel" "0")
aclspv_predef_switch(aclspv-common-essential	INTERFACE "ACLSPV_OpCapabilityShader" "1")
aclspv_predef_switch(aclspv-common-essential	INTERFACE "ACLSPV_CHECK_EXTRA" "1")
aclspv_predef_switch(aclspv-common-essential	INTERFACE "ACLSPV_SYMBOL" "1")
aclspv_predef_switch(aclspv-common-essential	INTERFACE "ACLSPV_ATOMIC_ID" 0)
aclspv_predef_switch(aclspv-common-essential	INTERFACE "ACLSPV_OPACC_IDX_MAX" 64)
aclspv_predef_switch(aclspv-aclexpr	PRIVATE		"ACLSPV_MATH_NR" 0)	# not ready
aclspv_predef_switch(aclspv-aclexpr	PRIVATE		"ACLSPV_MATH_NR_RSQRTF_MAGIC"	0x5f375a86)
aclspv_predef_switch(aclspv-aclexpr	PRIVATE		"ACLSPV_MATH_NR_IT"		2)

message(STATUS ""
	"[aclspv_essential]\n"
	"\taclspv_llvm_from_source: ${aclspv_llvm_from_source}\n"
	"\taclspv_essential_shared: ${aclspv_essential_shared}\n"
	"\taclspv_example: ${aclspv_example}\n"
	"\taclspv_example_vk: ${aclspv_example_vk}"
	)

if(${aclspv_example})
	add_subdirectory(./example/)
endif()
