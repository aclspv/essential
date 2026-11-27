# Copyright 2026 dalmurii
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# See the LICENSE file for the full text, including LLVM Exceptions.

function(aclspv_abitent prm_postfix)
	file(GLOB inc ${PROJECT_SOURCE_DIR}/inc/*)
	file(GLOB_RECURSE src ${PROJECT_SOURCE_DIR}/lib/${prm_postfix}/*)
	file(GLOB_RECURSE src2 ${PROJECT_SOURCE_DIR}/lib/${prm_postfix}*)

	ae2f_CoreLibTentConfigCustom(
		${prm_postfix} OBJECT
		inc aclspv
		${PROJECT_SOURCE_DIR}/cfg/aclspv-${prm_postfix}.cmake.in
		${src} ${inc} ${src2} ${head_gen}
		)

	target_compile_definitions(aclspv-${prm_postfix} PRIVATE ACLSPVLIB=1)
	target_link_libraries(aclspv-${prm_postfix}
		PRIVATE		aclspv::common-essential
		PUBLIC		aclspv::common
		)
	target_include_directories(aclspv-${prm_postfix}
		PRIVATE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/3rdinc>
		)

	ae2f_UltraErrWERROR(aclspv-${prm_postfix} PRIVATE)

	if(${aclspv_essential_shared})
		set_property(TARGET aclspv-${prm_postfix} PROPERTY POSITION_INDEPENDENT_CODE ON)
	endif()
endfunction()

