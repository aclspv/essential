project(aclspv_example_vk LANGUAGES C CXX)
find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)

file(GLOB files "./*.c*")

foreach(file ${files})
	get_filename_component(tar ${file} NAME)

	add_executable(aclspv-example-vk-${tar} ${file})
	target_link_libraries(
		aclspv-example-vk-${tar}
		PRIVATE Vulkan::Vulkan
		PRIVATE aclspv::essential
		PRIVATE SDL3::SDL3
		)

	set_target_properties(aclspv-example-vk-${tar} PROPERTIES OUTPUT_NAME ${tar})
endforeach()
