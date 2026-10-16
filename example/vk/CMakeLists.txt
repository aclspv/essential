project(aclspv_example_vk LANGUAGES C CXX)
find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

file(GLOB files "./*.c*")

foreach(file ${files})
	get_filename_component(tar ${file} NAME)
	add_executable(${tar} ${file})
	target_link_libraries(${tar} PRIVATE
		vulkan
		ae3f-aclspv
		SDL2
	)
endforeach()
