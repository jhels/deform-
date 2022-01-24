# Copyright 2018 The Min-DevKit Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${PROJECT_SOURCE_DIR}/min-devkit/source/min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)


#############################################################
# MAX EXTERNAL
#############################################################

# execute_process("python3 ./boost-headers-only/make_headers.py")

include_directories( 
	"${C74_INCLUDES}"
	# "${CMAKE_SOURCE_DIR}/source/projects/lib.deform_tilde/NumCpp/include"
	# "./NumCpp/include"
	# "${CMAKE_SOURCE_DIR}/source/projects/lib.deform_tilde/boost-headers-only"
	# "./boost-headers-only"
)


set( SOURCE_FILES
	${PROJECT_NAME}.cpp
)


add_library( 
	${PROJECT_NAME} 
	MODULE
	${SOURCE_FILES}
)


include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)


#############################################################
# UNIT TEST
#############################################################

include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)
