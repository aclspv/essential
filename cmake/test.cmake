# SPIR-V test helpers.
#
# a compile test writes its module to a per-target path passed as the
# ACLTEST_OUT compile definition. the path is derived from the test source
# name, so a validator test can be registered from the same value instead of
# a hand-kept "<test>|<module>" table.
#
# usage, after the test source directories are known:
#
#   aclspv_CoreTestTentVerbose(aclspv-essential-test ./test/compile)
#   aclspv_test_cxx(aclspv-essential-test ./test/compile)
#   aclspv_spv_target()

# optional: the validator tests simply do not exist when it is missing.
find_program(ACLSPV_SPIRV_VAL NAMES spirv-val)

# module stem for a test source: "<ext>-<name>", e.g. "c-ternary.1.0".
function(aclspv_test_spv_stem prm_source prm_out)
	get_filename_component(__FILE "${prm_source}" NAME)

	string(REGEX REPLACE "^.*\\.([^.]+)$" "\\1" __EXT "${__FILE}")
	string(REGEX REPLACE "\\.[^.]+$" "" __STEM "${__FILE}")

	set(${prm_out} "${__EXT}-${__STEM}" PARENT_SCOPE)
endfunction()

# attach the module output and the validator to one existing test target.
#
#   prm_target : a target created by add_executable
#   prm_stem   : module stem from aclspv_test_spv_stem()
function(aclspv_test_spv prm_target prm_stem)
	set(spv "${CMAKE_CURRENT_BINARY_DIR}/result-${prm_stem}.spv")

	target_compile_definitions("${prm_target}" PRIVATE
		ACLTEST_OUT="${spv}")

	# the test writes the module itself; expose that as a build rule so
	# `--target aclspv-spv` can materialise every module.
	add_custom_command(OUTPUT "${spv}"
		COMMAND $<TARGET_FILE:${prm_target}>
		DEPENDS "${prm_target}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "generating ${spv}")

	# a later aclspv_spv_target() collects these.
	set_property(GLOBAL APPEND PROPERTY ACLSPV_SPV_OUTPUTS "${spv}")

	if(NOT ACLSPV_SPIRV_VAL)
		return()
	endif()

	set_tests_properties("${prm_target}"
		PROPERTIES FIXTURES_SETUP "spv_${prm_stem}")

	add_test(NAME "Test-spirv-val-${prm_stem}"
		COMMAND "${ACLSPV_SPIRV_VAL}" "${spv}")

	set_tests_properties("Test-spirv-val-${prm_stem}"
		PROPERTIES FIXTURES_REQUIRED "spv_${prm_stem}")
endfunction()

# wrap ae2f_CoreTestTentVerbose and attach the SPIR-V step to every test it
# registers. the target naming mirrors that generator exactly.
function(aclspv_CoreTestTentVerbose prm_LibName prm_TestSourcesDir)
	ae2f_CoreTestTentVerbose(${prm_LibName} ${prm_TestSourcesDir} ${ARGN})

	if(NOT ae2f_TEST)
		return()
	endif()

	if(${ae2f_CXX})
		file(GLOB files "${prm_TestSourcesDir}/*.c*")
	else()
		file(GLOB files "${prm_TestSourcesDir}/*.c")
	endif()

	foreach(item IN LISTS files)
		get_filename_component(__NAME ${item} NAME_WLE)
		get_filename_component(___EXT ${item} EXT)
		string(SUBSTRING "${___EXT}" 1 -1 __EXT)

		aclspv_test_spv_stem("${item}" __STEM)

		aclspv_test_spv(
			"Test-${__EXT}-${prm_LibName}-${__NAME}"
			"${__STEM}")
	endforeach()
endfunction()

# C++ shader variants. ae2f_CoreTestTentVerbose globs only *.c when
# ae2f_CXX is off, so register them here with the same naming and the same
# SPIR-V step.
function(aclspv_test_cxx prm_LibName prm_TestSourcesDir)
	if(NOT (ae2f_TEST AND CMAKE_CXX_COMPILER))
		return()
	endif()

	file(GLOB files "${prm_TestSourcesDir}/*.cc")

	foreach(item IN LISTS files)
		get_filename_component(__NAME ${item} NAME_WLE)
		set(tgt "Test-cc-${prm_LibName}-${__NAME}")

		aclspv_test_spv_stem("${item}" __STEM)

		add_executable("${tgt}" ${item})
		target_link_libraries("${tgt}" PRIVATE ${ARGN} ${prm_LibName})
		add_test(NAME "${tgt}" COMMAND "${tgt}")

		aclspv_test_spv("${tgt}" "${__STEM}")
	endforeach()
endfunction()

# one on-demand target that produces every module a compile test can write.
function(aclspv_spv_target)
	get_property(outputs GLOBAL PROPERTY ACLSPV_SPV_OUTPUTS)

	if(NOT outputs)
		return()
	endif()

	add_custom_target(aclspv-spv DEPENDS ${outputs})
endfunction()
