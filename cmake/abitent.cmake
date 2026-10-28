function(aclspv_abitent prm_postfix)
	file(GLOB inc ${PROJECT_SOURCE_DIR}/inc/*)
	file(GLOB_RECURSE src ${PROJECT_SOURCE_DIR}/lib/${prm_postfix}/*)
	file(GLOB_RECURSE src2 ${PROJECT_SOURCE_DIR}/lib/${prm_postfix}*)

	ae2f_CoreLibTentConfigCustom(
		${prm_postfix} OBJECT
		inc aclspv
		${PROJECT_SOURCE_DIR}/cfg/aclspv-${prm_postfix}.cmake.in
		${src} ${inc} ${src2}
		)

	target_compile_definitions(aclspv-${prm_postfix} PRIVATE ACLSPVLIB=1)
	target_link_libraries(aclspv-${prm_postfix}
		PRIVATE		aclspv::common-essential
		PUBLIC		aclspv::common
		)

	ae2f_UltraErrWERROR(aclspv-${prm_postfix} PRIVATE)

	if(${aclspv_essential_shared})
		set_property(TARGET aclspv-${prm_postfix} PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif()
endfunction()

