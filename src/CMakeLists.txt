cmake_minimum_required(VERSION 3.10)
project(aclspvCore VERSION 0)

ae2f_Macro_init(1 100 100)
ae2f_Macro_Lib_V2_REL(
	ae3f aclspvCore INTERFACE
	${PROJECT_SOURCE_DIR}/lib "__NOTHING"
	${PROJECT_SOURCE_DIR}/lib "*.c*"
	inc/.aclspv ".h"
	${PROJECT_SOURCE_DIR}/../cfg/aclspvCore.cmake.in
	inc ON
	)

target_link_libraries(ae3f-aclspvCore 
	INTERFACE ae2f::Pat
	INTERFACE ae3f::aclspv-common
	)

ae2f_CoreTestTentVerbose(ae3f-aclspvCore ./test/dummy)
target_include_directories(ae3f-aclspvCore INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>)
