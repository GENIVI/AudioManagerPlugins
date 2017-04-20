############################################################################
# SPDX license identifier: MPL-2.0
#
# Copyright (C) 2012-2014, Wind River Systems
# Copyright (C) 2014, GENIVI Alliance
#
# This file is part of Pulse Audio Interface Routing Plugin.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License (MPL), v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
#
# List of changes:
#
# 21.08.2014, Adrian Scarlat, First version of the code;
#                             Porting code from AM ver1.x to AM ver3.0;
#                             Added Copyright and License information;
############################################################################

cmake_minimum_required(VERSION 2.6)

PROJECT(PluginRoutingInterfacePULSE)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
FILE(GLOB pulseConf "${CMAKE_CURRENT_SOURCE_DIR}/README*")
FIND_PACKAGE(PkgConfig)

OPTION( WITH_DOCUMENTATION
        "Build together with Doxygen Documentation" OFF )

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
SET(PLUGINS_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/plugins)
set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/routing)
set(DOC_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/doc/RoutingPlugin)
set(INCLUDES_FOLDER "include")
SET(LIB_INSTALL_SUFFIX "audiomanager")

FIND_PACKAGE(DBUS REQUIRED)
FIND_PATH(AUDIO_INCLUDE_FOLDER audiomanagertypes.h /usr/include)

if(DEFINED AUDIO_INCLUDE_FOLDER)
        message(STATUS "Found AudioManager include: ${AUDIO_INCLUDE_FOLDER}")
else(DEFINED AUDIO_INCLUDE_FOLDER)
        message(STATUS "Did not found AudioManager include!")
endif(DEFINED AUDIO_INCLUDE_FOLDER)

FILE(READ "${AUDIO_INCLUDE_FOLDER}/IAmRouting.h" VERSION_BUFFER LIMIT 6000)
STRING(REGEX MATCH "RoutingVersion*.[^0-9]*[0-9]" LIB_INTERFACE_VERSION_STRING ${VERSION_BUFFER})
STRING(REGEX REPLACE "[^0-9]" "" LIB_INTERFACE_VERSION ${LIB_INTERFACE_VERSION_STRING})
MESSAGE(STATUS "Building against routing interface version ${LIB_INTERFACE_VERSION}")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
FIND_PACKAGE(DBUS REQUIRED)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${DBUS_INCLUDE_DIR}
        ${DBUS_ARCH_INCLUDE_DIR}
        ${AUDIO_INCLUDE_FOLDER}
    	${AUDIOMANAGER_INCLUDE_FOLDER}
    	${AUDIOMANAGER_UTILITIES_INCLUDE}
        ${INCLUDES_FOLDER}
)

# all source files go here
file(GLOB PLUGINDBUS_SRCS_CXX "src/*.cpp")

add_library(PluginRoutingInterfacePULSE SHARED ${PLUGINDBUS_SRCS_CXX})

TARGET_LINK_LIBRARIES(PluginRoutingInterfacePULSE
    pulse
    ${DLT_LIBRARIES}
    ${DBUS_LIBRARY}
)

IF(WITH_DOCUMENTATION)
        file(MAKE_DIRECTORY ${DOC_OUTPUT_PATH})
        configure_file(${DOXY_FILE}  ${DOC_OUTPUT_PATH}/Doxyfile  @ONLY IMMEDIATE)
        add_custom_target (PluginRoutingInterfacePULSEDocs ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOC_OUTPUT_PATH}/Doxyfile WORKING_DIRECTORY ${DOC_OUTPUT_PATH}
                SOURCES ${PROJECT_BINARY_DIR} ${DOC_OUTPUT_PATH}/Doxyfile
        )
ENDIF(WITH_DOCUMENTATION)

INSTALL(TARGETS PluginRoutingInterfacePULSE
        DESTINATION "lib/${LIB_INSTALL_SUFFIX}/routing"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT sampleplugins
)

INSTALL(FILES data/libPluginRoutingInterfacePULSE.conf
        DESTINATION "lib/${LIB_INSTALL_SUFFIX}/routing"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT sampleplugins
)

# Uncomment the following five lines bellow (that start with #CONFIGURE_FILE...)
# to make those files available in build environment;
# For meta-ivi deployment purposes leave them commented.

#CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/PluginRoutingInterfacePulse/data/libPluginRoutingInterfacePULSE.conf ${PLUGINS_OUTPUT_PATH}/routing/libPluginRoutingInterfacePULSE.conf )
