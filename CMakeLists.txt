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

if (NOT DAEMONVERSION)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
    message(STATUS "Building without AudioManager, searching for cmake definition")
    find_package(AudioManager)  
    if (AUDIOMANAGER_FOUND)
        include ("${AUDIOMANAGER_CMAKE_CONFIG_PATH}/audiomanagerConfig.cmake")
        set(PLUGINS_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}) 
        if(WITH_TESTS)
            add_subdirectory(${GOOGLE_MOCK_PROJECT_FOLDER} ${CMAKE_CURRENT_BINARY_DIR}/gmock)
        endif(WITH_TESTS)
        
        set(COMMONAPI_DBUS_CONFIGURATIONS "") 
    else(AUDIOMANAGER_FOUND)
        message(FATAL_ERROR "Could not find audiomanager cmake configuration")
    endif(AUDIOMANAGER_FOUND)
endif (NOT DAEMONVERSION)

if(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
    add_definitions(${DLT_CFLAGS_OTHER})
    include_directories(${DLT_INCLUDE_DIRS})
    link_directories(${DLT_LIBRARY_DIRS})
endif(WITH_DLT)



file(GLOB all_valid_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*/CMakeLists.txt")

foreach(dir ${all_valid_subdirs})
    if(${dir} MATCHES "^([^/]*)//CMakeLists.txt")
        string(REGEX REPLACE "^([^/]*)//CMakeLists.txt" "\\1" dir_trimmed ${dir})
        add_subdirectory(${dir_trimmed})
    endif()
endforeach(dir)

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



