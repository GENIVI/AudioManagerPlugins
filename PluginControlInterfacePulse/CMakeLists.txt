############################################################################
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2012-2014, Wind River Systems
# Copyright (C) 2014, GENIVI Alliance
#
# This file is part of Pulse Audio Interface Control Plugin.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License (MPL), v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
#
# List of changes:
#
# 08.09.2014, Adrian Scarlat, First version of the code;
#                             Porting code from AM ver1.x;
############################################################################

cmake_minimum_required(VERSION 2.6)

PROJECT(PluginControlInterfacePulse)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

FIND_PACKAGE(PkgConfig)

OPTION( WITH_DOCUMENTATION
	"Build together with Doxygen Documentation" OFF )

SET(PLUGINS_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/plugins)
set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/control)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/doc/ControlPlugin)
set(INCLUDE_FOLDER "include")
SET(LIB_INSTALL_SUFFIX "audiomanager")

FIND_PATH(AUDIO_INCLUDE_FOLDER audiomanagertypes.h /usr/include)

if(DEFINED AUDIO_INCLUDE_FOLDER)
	message(STATUS "Found AudioManager include: ${AUDIO_INCLUDE_FOLDER}")
else(DEFINED AUDIO_INCLUDE_FOLDER)
	message(STATUS "Did not found AudioManager include!")
endif(DEFINED AUDIO_INCLUDE_FOLDER)

FILE(READ "${AUDIO_INCLUDE_FOLDER}/IAmControl.h" VERSION_BUFFER LIMIT 6000)
STRING(REGEX MATCH "ControlVersion*.[^0-9]*[0-9]" LIB_INTERFACE_VERSION_STRING ${VERSION_BUFFER})
STRING(REGEX REPLACE "[^0-9]" "" LIB_INTERFACE_VERSION ${LIB_INTERFACE_VERSION_STRING})
MESSAGE(STATUS "Building against control interface version ${LIB_INTERFACE_VERSION}")

FIND_PACKAGE(NSM REQUIRED)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}
	${STD_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${AUDIO_INCLUDE_FOLDER}
    	${AUDIOMANAGER_INCLUDE_FOLDER}
    	${AUDIOMANAGER_UTILITIES_INCLUDE}
	${PROJECT_INCLUDE_FOLDER}
	${INCLUDE_FOLDER}
	${NSM_INCLUDE_DIR}
)

# all source files go here
file(GLOB PLUGIN_CONTROL_SRCS_CXX "src/*.cpp")

add_library(PluginControlInterface SHARED ${PLUGIN_CONTROL_SRCS_CXX})

TARGET_LINK_LIBRARIES(PluginControlInterface
    ${DLT_LIBRARIES})

IF(WITH_DOCUMENTATION)
	file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
	configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
	add_custom_target (PluginControlInterfaceDBusDocs ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
		SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
	)
ENDIF(WITH_DOCUMENTATION)

INSTALL(TARGETS PluginControlInterface
        DESTINATION "lib/${LIB_INSTALL_SUFFIX}/control/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT sampleplugins
)

INSTALL(FILES data/libPluginControlInterface.conf
        DESTINATION "lib/${LIB_INSTALL_SUFFIX}/control/"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT sampleplugins
)

# Uncomment the following five line bellow, that start with #CONFIGURE_FILE...,
# to make libPluginControlInterface.conf file available in build environment;
# For meta-ivi deployment purposes leave it commented.

#CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/PluginControlInterfacePulse/data/libPluginControlInterface.conf ${PLUGINS_OUTPUT_PATH}/control/libPluginControlInterface.conf)
