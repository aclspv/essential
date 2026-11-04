project(aclspv_example_plain LANGUAGES C CXX)

file(GLOB files "./*.c*")

foreach(file ${files})
	get_filename_component(tar ${file} NAME)

	add_executable(aclspv-example-plain-${tar} ${file})
	target_link_libraries(
		aclspv-example-plain-${tar}
		PRIVATE aclspv::essential
		)

	set_target_properties(aclspv-example-plain-${tar} PROPERTIES OUTPUT_NAME ${tar})
endforeach()
