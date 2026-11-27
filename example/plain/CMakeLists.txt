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
