# Copyright (c) 2016 GENIVI Alliance
# Copyright (c) 2016 Advanced Driver Information Technology
#
# author Mattia Guerra, mguerra@de.adit-jv.com ADIT 2016
#
# copyright
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# For further information see http://www.genivi.org/.
#
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

INCLUDE(CMakeDependentOption)
FIND_PACKAGE(PkgConfig)

PROJECT(PluginRoutingInterfacePULSE)

FIND_PACKAGE(AudioManager REQUIRED > 7.3.0)  
FIND_PACKAGE(AudioManagerUtilities REQUIRED > 7.3.0) 

IF (WITH_DLT)
    pkg_check_modules(DLT_REQUIRED automotive-dlt>=2.2.0)
    add_definitions(${DLT_CFLAGS_OTHER})
    include_directories(${DLT_INCLUDE_DIRS})
    link_directories(${DLT_LIBRARY_DIRS})
ENDIF(WITH_DLT)

SET(PLUGIN_LIB_NAME PluginRoutingInterfacePULSE)

SET(AM_LINK_LIBS ${CMAKE_DL_LIBS})

SET(INCLUDE_FOLDER "include")

SET(CONF_ROOT "${PLUGINS_CONFIG_PATH}/routing")

FIND_PACKAGE(PulseAudio REQUIRED)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${AudioManager_INCLUDE_DIRS}
    ${AudioManagerUtilities_INCLUDE_DIRS}
    ${INCLUDE_FOLDER}
)

IF(WITH_DLT)
    INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
        ${DLT_INCLUDE_DIRS})
    SET(AM_LINK_LIBS
        ${AM_LINK_LIBS}
        ${DLT_LIBRARIES})
ENDIF(WITH_DLT)

##build flags set(CPACK_RPM_COMPONENT_INSTALL ON)
set(ROUTING_INTERFACE_PULSE_CMAKE_CXX_FLAGS "-std=c++11 -pedantic -rdynamic")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROUTING_INTERFACE_PULSE_CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")

# all source files go here
file(GLOB PLUGINPULSE_SRCS_CXX "src/*.cpp")

add_library(PluginRoutingInterfacePULSE MODULE ${PLUGINPULSE_SRCS_CXX})

TARGET_LINK_LIBRARIES(PluginRoutingInterfacePULSE
    ${AM_LINK_LIBS}
    ${PULSEAUDIO_LIBRARY}
)

INSTALL(TARGETS PluginRoutingInterfacePULSE
        DESTINATION lib${LIB_SUFFIX}/${LIB_INSTALL_SUFFIX}/routing
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT plugin)

INSTALL(DIRECTORY data/
    DESTINATION lib${LIB_SUFFIX}/${LIB_INSTALL_SUFFIX}/routing
    COMPONENT data
)
