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

cmake_minimum_required(VERSION 3.10)
project(aclspvCore VERSION 0)

ae2f_Macro_init(1 100 100)
ae2f_Macro_Lib_V2_REL(
	aclspv common-essential INTERFACE
	${PROJECT_SOURCE_DIR}/lib "__NOTHING"
	${PROJECT_SOURCE_DIR}/lib "*.c*"
	inc/.aclspv ".h"
	${PROJECT_SOURCE_DIR}/../cfg/aclspv-common-essential.cmake.in
	inc ON
	)

target_link_libraries(aclspv-common-essential
	INTERFACE ae2f::Pat
	INTERFACE aclspv::common
	INTERFACE ae2f::num
	)

ae2f_CoreTestTentVerbose(aclspv-common-essential ./test/dummy)
target_include_directories(aclspv-common-essential
	INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../lib>
	)
