# Copyright (c) 2012 GENIVI Alliance
# Copyright (c) 2012 BMW
#
# author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
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

include(CMakeDependentOption)
find_package(PkgConfig)

PROJECT(PluginRoutingInterfaceCAPI)

     
if (NOT DAEMONVERSION)
    message(STATUS "Building without AudioManager, searching for cmake definition")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    find_package(AudioManager)  
    if (AUDIOMANAGER_FOUND)
        include ("${AUDIOMANAGER_CMAKE_CONFIG_PATH}/audiomanagerConfig.cmake")
        set(PLUGINS_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}) 
        set(TEST_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}) 
        if(WITH_TESTS)
            add_subdirectory(${GOOGLE_MOCK_PROJECT_FOLDER} ${CMAKE_CURRENT_BINARY_DIR}/gmock)
        endif(WITH_TESTS)
        
        set(COMMONAPI_DBUS_CONFIGURATIONS "") 
    else(AUDIOMANAGER_FOUND)
        message(FATAL_ERROR "Could not find audiomanager cmake configuration")
    endif(AUDIOMANAGER_FOUND)
endif (NOT DAEMONVERSION)        

IF(${WITH_ENABLED_IPC} STREQUAL "CAPI")

    set(INCLUDE_FOLDER "include")
       
    FIND_PACKAGE(CommonAPI REQUIRED)
    
    set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/routing)
    
	if(WITH_DLT)    
	    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
	    add_definitions(${DLT_CFLAGS_OTHER})
	    include_directories(${DLT_INCLUDE_DIRS})
	    link_directories(${DLT_LIBRARY_DIRS})
	endif(WITH_DLT)    
    
    # Find the common-api sources. 
    # The sources can be placed in the build directory (DESTINATION) or in the source tree (ALT_DESTINATION). 
    # The ALT_DESTINATION is used as fall back if the DESTINATION directory dosen't contain the sources or you just don't want to use the generator.
    INCLUDE(CommonAPIGenerator)
    COMMON_API_GENERATE_SOUCRES(TARGET COMMON_API_ROUTING_INTERFACE 
                                FIDLS fidl/RoutingInterface.fidl # a list with fidls
                                DESTINATION src-gen  # a relative path to the build directory or an absolute path
                                ALT_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src-gen # an alternative relative/absolute path with common-api sources, usually in the source tree
                                DBUS)
    
    INCLUDE_DIRECTORIES(
    	${CMAKE_SOURCE_DIR} 
     	${CMAKE_CURRENT_BINARY_DIR}
    	${AUDIO_INCLUDE_FOLDER}
    	${AUDIOMANAGER_INCLUDE_FOLDER}
    	${AUDIOMANAGER_UTILITIES_INCLUDE}
    	${COMMON_API_INCLUDE_DIRS}
    	${COMMON_API_DBUS_INCLUDE_DIRS}
    	${COMMON_API_ROUTING_INTERFACE_GEN_INCLUDE_DIR}
    	${INCLUDE_FOLDER}
    )
    
    # all source files go here
    file(GLOB PLUGINCAPI_SRCS_CXX "src/*.cpp" ${COMMON_API_ROUTING_INTERFACE_GEN_SOURCES})
    
    add_library(PluginRoutingInterfaceCAPI MODULE ${PLUGINCAPI_SRCS_CXX})
    
    TARGET_LINK_LIBRARIES(PluginRoutingInterfaceCAPI 
        ${DLT_LIBRARIES}
        ${COMMON_API_LIBRARIES}
        ${COMMON_API_DBUS_LIBRARIES}
    )
    
    IF(WITH_TESTS)
    	add_subdirectory (test)
    ENDIF(WITH_TESTS)
   
    
    INSTALL(TARGETS PluginRoutingInterfaceCAPI 
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/routing
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugin
    )

ENDIF(${WITH_ENABLED_IPC} STREQUAL "CAPI")
