# Copyright (C) 2012, BMW AG
#
# This file is part of GENIVI Project AudioManager.
# 
# Contributions are licensed to the GENIVI Alliance under one or more
# Contribution License Agreements.
# 
# copyright
# This Source Code Form is subject to the terms of the
# Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
# this file, You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# author Christian Linke, christian.linke@bmw.de BMW 2011,2012
#
# For further information see http://www.genivi.org/.
#

cmake_minimum_required(VERSION 2.8.8)

include(CMakeDependentOption)
find_package(PkgConfig)


OPTION (WITH_COMMAND_INTERFACE_COMMON_CAPI
        "build with commandinterface common-api plugin" ON )
OPTION (WITH_COMMAND_INTERFACE_DBUS
        "build with commandinterface dbus plugin" OFF )
OPTION (WITH_TEST_CONTROLLER 
        "Build with the test controller" ON)     
OPTION (WITH_ROUTING_INTERFACE_ASYNC
        "build with routing interface async plugin" ON)
OPTION(WITH_ROUTING_INTERFACE_CAPI
       "build with routing iterface common-api plugin" ON)   
OPTION(WITH_ROUTING_INTERFACE_DBUS
       "build with routing iterface dbus plugin" OFF)        
       
IF (WITH_CAPI_WRAPPER) 
    set (USE_CAPI_BINDING "DBUS" CACHE STRING "Select prefered binding for common-api 'DBUS' or 'SOMEIP'")
    set_property(CACHE USE_CAPI_BINDING PROPERTY STRINGS "DBUS" "SOMEIP") 
ELSE()     
    unset(USE_CAPI_BINDING CACHE)
ENDIF ()

IF (USE_CAPI_BINDING STREQUAL "SOMEIP")
    set(SELECTED_CAPI_BINDING 1 CACHE INTERNAL "hide this!" FORCE)
ELSEIF (USE_CAPI_BINDING STREQUAL "DBUS")
    set(SELECTED_CAPI_BINDING 0 CACHE INTERNAL "hide this!" FORCE)
ELSE()
    unset(SELECTED_CAPI_BINDING CACHE)
ENDIF() 
             

IF(USE_BUILD_LIBS) 
	file(MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/plugins")
ENDIF(USE_BUILD_LIBS)

IF (WITH_COMMAND_INTERFACE_COMMON_CAPI)
	add_subdirectory(PluginCommandInterfaceCAPI)
ENDIF(WITH_COMMAND_INTERFACE_COMMON_CAPI)
 
IF (WITH_COMMAND_INTERFACE_DBUS)
	add_subdirectory(PluginCommandInterfaceDbus)
ENDIF (WITH_COMMAND_INTERFACE_DBUS)
        
IF (WITH_TEST_CONTROLLER)
	add_subdirectory(PluginControlInterface)
ENDIF (WITH_TEST_CONTROLLER)   

IF (WITH_ROUTING_INTERFACE_ASYNC)
	add_subdirectory(PluginRoutingInterfaceAsync)
ENDIF (WITH_ROUTING_INTERFACE_ASYNC)  

IF(WITH_ROUTING_INTERFACE_CAPI)
		add_subdirectory(PluginRoutingInterfaceCAPI)
ENDIF(WITH_ROUTING_INTERFACE_CAPI)

IF(WITH_ROUTING_INTERFACE_DBUS) 
		add_subdirectory(PluginRoutingInterfaceDbus)
ENDIF(WITH_ROUTING_INTERFACE_DBUS) 
        
add_custom_target(plugins-install
  COMMAND
     "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=plugin
     -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)

add_custom_target(plugin-test-install
  COMMAND
     "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=plugin-tests
     -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)


