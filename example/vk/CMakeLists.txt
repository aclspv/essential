project(aclspv_example_vk LANGUAGES C CXX)
find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)

file(GLOB files "./*.c*")

foreach(file ${files})
	get_filename_component(tar ${file} NAME)
	add_executable(${tar} ${file})
	target_link_libraries(${tar}
		PRIVATE Vulkan::Vulkan
		PRIVATE aclspv::aclspv
		PRIVATE SDL3::SDL3
	)
endforeach()
