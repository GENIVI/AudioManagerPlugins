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

include(CMakeDependentOption)
find_package(PkgConfig)

PROJECT(PluginRoutingInterfaceAsync)

       
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

	if(WITH_DLT)    
	    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
	    add_definitions(${DLT_CFLAGS_OTHER})
	    include_directories(${DLT_INCLUDE_DIRS})
	    link_directories(${DLT_LIBRARY_DIRS})
	endif(WITH_DLT)      
        
    set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/routing)

    set(INCLUDE_FOLDER "include")
    
    find_package( Threads )
    
    INCLUDE_DIRECTORIES(
     	${CMAKE_CURRENT_BINARY_DIR}
    	${AUDIOMANAGER_INCLUDE_FOLDER}
    	${AUDIO_INCLUDE_FOLDER}
    	${AUDIOMANAGER_UTILITIES_INCLUDE}
    	${INCLUDE_FOLDER}
    )
    
    # all source files go here
    file(GLOB PLUGINDBUS_SRCS_CXX "src/*.cpp")
    
    add_library(PluginRoutingInterfaceAsync MODULE ${PLUGINDBUS_SRCS_CXX})
    
    TARGET_LINK_LIBRARIES(PluginRoutingInterfaceAsync
        ${DLT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
    
    IF(WITH_TESTS)
    	add_subdirectory (test)
    ENDIF(WITH_TESTS)
    
   
    INSTALL(TARGETS PluginRoutingInterfaceAsync 
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/routing
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugins
    )
