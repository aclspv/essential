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

macro(aclspv_predef_switch tar prefix switch default)
	if(DEFINED ${switch})
		target_compile_definitions(${tar} ${prefix} "__${switch}=${${switch}}")
	else()
		target_compile_definitions(${tar} ${prefix} "__${switch}=${default}")
		set(${switch} ${default})
	endif()

	message(STATUS "\t ${tar}\t-> ${switch}:${default}")
endmacro()
