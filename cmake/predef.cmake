macro(aclspv_predef_switch tar switch default)
	if(DEFINED ${switch})
		target_compile_definitions(${tar} PRIVATE "__${switch}=${switch}")
	else()
		target_compile_definitions(${tar} PRIVATE "__${switch}=${default}")
	endif()

	message(STATUS "\t ${switch}: ${default}")
endmacro()
