# Copyright (c) 2012 GENIVI Alliance
# Copyright (c) 2012 BMW
#
# author Christian Linke, christian.linke@bmw.de BMW 2011,2012
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
cmake_minimum_required(VERSION 2.8.8)

PROJECT(PluginRoutingInterfaceDbus)

if (NOT DAEMONVERSION)
    message(STATUS "Building without AudioManager, searching for cmake definition")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    find_package(AudioManager)  
    if (AUDIOMANAGER_FOUND)
        include ("${AUDIOMANAGER_CMAKE_CONFIG_PATH}/audiomanagerConfig.cmake")
        set(PLUGINS_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}) 
        set(TEST_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}) 
       
        set(COMMONAPI_DBUS_CONFIGURATIONS "") 
    else(AUDIOMANAGER_FOUND)
        message(FATAL_ERROR "Could not find audiomanager cmake configuration")
    endif(AUDIOMANAGER_FOUND)
endif (NOT DAEMONVERSION)          

IF(NOT WITH_DBUS_WRAPPER)
	message (FATAL_ERROR "For Dbus plugins you need to compile the AM WITH_DBUS_WRAPPER")
ENDIF(NOT WITH_DBUS_WRAPPER)

    set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/routing)
    set(INCLUDE_FOLDER "include")
	if(WITH_DLT)    
	    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
	    add_definitions(${DLT_CFLAGS_OTHER})
	    include_directories(${DLT_INCLUDE_DIRS})
	    link_directories(${DLT_LIBRARY_DIRS})
	endif(WITH_DLT)    
    
    IF(USE_BUILD_LIBS) 
        SET(ROUTING_DBUS_INTROSPECTION_FOLDER ${EXECUTABLE_OUTPUT_PATH})
        SET(ROUTING_DBUS_INTROSPECTION_FILE ${EXECUTABLE_OUTPUT_PATH}/RoutingReceiver.xml)
    ELSE(USE_BUILD_LIBS) 
        SET(ROUTING_DBUS_INTROSPECTION_FOLDER ${AM_SHARE_FOLDER}/${LIB_INSTALL_SUFFIX})
        SET(ROUTING_DBUS_INTROSPECTION_FILE ${AM_SHARE_FOLDER}/${LIB_INSTALL_SUFFIX}/RoutingReceiver.xml)
    ENDIF(USE_BUILD_LIBS)
    
    
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake ${CMAKE_CURRENT_SOURCE_DIR}/include/configRoutingDbus.h )
    
   	pkg_check_modules (DBUS "dbus-1 >= 1.4" REQUIRED)
    
    INCLUDE_DIRECTORIES(
    	${CMAKE_SOURCE_DIR} 
     	${CMAKE_CURRENT_BINARY_DIR}
     	${DBUS_INCLUDE_DIRS}
    	${AUDIO_INCLUDE_FOLDER}
    	${AUDIOMANAGER_INCLUDE_FOLDER}
    	${AUDIOMANAGER_UTILITIES_INCLUDE}
    	${INCLUDE_FOLDER}
    )
    
    # all source files go here
    file(GLOB PLUGINDBUS_SRCS_CXX "src/*.cpp")
    
    add_library(PluginRoutingInterfaceDbus MODULE ${PLUGINDBUS_SRCS_CXX})
    
    TARGET_LINK_LIBRARIES(PluginRoutingInterfaceDbus 
        ${DLT_LIBRARIES}
        ${DBUS_LIBRARIES}
    )
    
    IF(WITH_TESTS)
    #	add_subdirectory (test)
    ENDIF(WITH_TESTS)
    
    IF(USE_BUILD_LIBS) 
        execute_process(
        		COMMAND mkdir -p "${EXECUTABLE_OUTPUT_PATH}"
        		COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_FOLDER}/RoutingReceiver.xml" ${EXECUTABLE_OUTPUT_PATH}/RoutingReceiver.xml)
        execute_process(
        		COMMAND mkdir -p "${EXECUTABLE_OUTPUT_PATH}"
        		COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_FOLDER}/RoutingSender.xml" ${EXECUTABLE_OUTPUT_PATH}/RoutingSender.xml)
    ENDIF(USE_BUILD_LIBS) 
    

    
    
    INSTALL(TARGETS PluginRoutingInterfaceDbus 
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/routing
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugin
    )
    
    INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/RoutingReceiver.xml 
            DESTINATION ${AM_SHARE_FOLDER}/${LIB_INSTALL_SUFFIX}
            COMPONENT plugin
    )
    