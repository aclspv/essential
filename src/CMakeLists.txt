cmake_minimum_required(VERSION 3.10)
project(aclspvCore VERSION 0)

ae2f_Macro_init(1 100 100)
ae2f_Macro_Lib_V2_REL(
	aclspv common-essential INTERFACE
	${PROJECT_SOURCE_DIR}/lib "__NOTHING"
	${PROJECT_SOURCE_DIR}/lib "*.c*"
	inc/.aclspv ".h"
	${PROJECT_SOURCE_DIR}/../cfg/aclspv-common-essential.cmake.in
	inc ON
	${head_gen}
	)

target_link_libraries(aclspv-common-essential
	INTERFACE ae2f::Pat
	INTERFACE aclspv::common
	)

ae2f_CoreTestTentVerbose(aclspv-common-essential ./test/dummy)
target_include_directories(aclspv-common-essential
	INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>
	INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/3rdinc>
	)
