macro(aclspv_predef_switch tar prefix switch default)
	if(DEFINED ${switch})
		target_compile_definitions(${tar} ${prefix} "__${switch}=${${switch}}")
	else()
		target_compile_definitions(${tar} ${prefix} "__${switch}=${default}")
	endif()

	message(STATUS "\t ${switch}: ${default}")
endmacro()
