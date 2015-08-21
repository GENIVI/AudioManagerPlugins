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

project(PluginCommandInterfaceCAPI)

if (NOT DAEMONVERSION)
    message(STATUS "Building without AudioManager, searching for cmake definition")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    find_package(AudioManager)  
    if (AUDIOMANAGER_FOUND)
        include ("${AUDIOMANAGER_CMAKE_CONFIG_PATH}/audiomanagerConfig.cmake")
        set(PLUGINS_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}) 
        set(TEST_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}) 

        if(WITH_TESTS)
            if(NOT TARGET gtest)
                add_subdirectory(${GOOGLE_MOCK_PROJECT_FOLDER} ${CMAKE_CURRENT_BINARY_DIR}/gmock)
            endif(NOT TARGET gtest)
        endif(WITH_TESTS)

        set(COMMONAPI_DBUS_CONFIGURATIONS "") 
    else(AUDIOMANAGER_FOUND)
        message(FATAL_ERROR "Could not find audiomanager cmake configuration")
    endif(AUDIOMANAGER_FOUND)
endif (NOT DAEMONVERSION)    

if (${WITH_ENABLED_IPC} STREQUAL "CAPI")

	if(WITH_DLT)    
	    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
	    add_definitions(${DLT_CFLAGS_OTHER})
	    include_directories(${DLT_INCLUDE_DIRS})
	    link_directories(${DLT_LIBRARY_DIRS})
	endif(WITH_DLT)

    set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/command)

    set (AM_LINK_LIBS 
        ${CMAKE_DL_LIBS}
        rt)
    
    set(INCLUDE_FOLDER "include")
        
    # Include CommonAPI ... 
    INCLUDE (CommonAPI)
    
    # load all the necessary libraries 
    # LOAD_COMMONAPI() without argument will use the CACHE variable COMMONAPI_USED_BINDING (0-DBus, 1-SomeIP).
    # It can be called with DBUS or SOMEIP as parameter: LOAD_COMMONAPI(DBUS) , LOAD_COMMONAPI(SOMEIP) which will overwrite COMMONAPI_USED_BINDING.
    LOAD_COMMONAPI()
    
        
    IF(${COMMONAPI_USED_BINDING} EQUAL 1)#SomeIP
	SET(SRC_GEN src-gen/v${COMMONAPI_SOMEIP_VERSION_NUMBER}/someip)
	SET(CAPI_BINDING_FIDLS fidl/CommandInterface.fdepl fidl/AudioManagerTypes.fdepl)	
    ELSE()#Dbus
	   SET(SRC_GEN src-gen/v${COMMONAPI_DBUS_VERSION_NUMBER}/dbus)
	   SET(CAPI_BINDING_FIDLS fidl/CommandInterface.fidl fidl/AudioManagerTypes.fidl)	
    ENDIF()

    # Find the common-api sources. 
    # The sources can be placed in the build directory (DESTINATION) or in the source tree (ALT_DESTINATION). 
    # The ALT_DESTINATION is used as fall back if the DESTINATION directory dosen't contain the sources or you just don't want to use the generator.
    COMMON_API_GENERATE_SOURCES(
                                # Prefix.
                                TARGET COMMON_API_COMMAND_INTERFACE 
                                # A list with fidls for the generic generator.
                                FIDLS_GENERIC fidl/CommandInterface.fidl
                                # A list with fidls for the binding generator.
                                FIDLS_BINDING ${CAPI_BINDING_FIDLS}
                                # A relative path to the build directory or an absolute path.
                                DESTINATION ${SRC_GEN} 
                                # An alternative relative/absolute path with common-api sources, usually in the source tree.
                                ALT_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src-gen
                                )
    
    INCLUDE_DIRECTORIES(
    	${CMAKE_SOURCE_DIR} 
     	${CMAKE_CURRENT_BINARY_DIR}
    	${AUDIO_INCLUDE_FOLDER}
    	${AUDIOMANAGER_INCLUDE_FOLDER}
    	${AUDIOMANAGER_UTILITIES_INCLUDE}
    	${COMMONAPI_INCLUDE_DIRS}
    	${COMMON_API_COMMAND_INTERFACE_GEN_INCLUDE_DIR}
    	${INCLUDE_FOLDER}
    )
    
        # all source files go here
    file(GLOB PLUGINCAPI_SRCS_CXX "src/*.cpp" )
    SET(PLUGINCAPI_SRCS_CXX ${PLUGINCAPI_SRCS_CXX} ${COMMON_API_COMMAND_INTERFACE_GEN_SOURCES})
    IF(${COMMONAPI_USED_BINDING} EQUAL 0)# CommonAPI-DBus
        INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
                ${COMMONAPI_DBUS_INCLUDE_DIRS}
                ${DBUS_INCLUDE_DIRS})	   
   ELSE()# CommonAPI-SomeIP
       	INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
                ${VSOMEIP_INCLUDE_DIRS}
                ${COMMONAPI_SOMEIP_INCLUDE_DIRS})
   ENDIF()
    
    
    IF(WITH_DLT)
        INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ${DLT_INCLUDE_DIRS})
        set (AM_LINK_LIBS ${AM_LINK_LIBS} ${DLT_LIBRARIES})
    ENDIF(WITH_DLT)    
        
    add_library(PluginCommandInterfaceCAPI MODULE ${PLUGINCAPI_SRCS_CXX})
    set (AM_LINK_LIBS ${AM_LINK_LIBS} ${CommonAPI_LIBRARY})
    if(${COMMONAPI_USED_BINDING} EQUAL 0)
       set (AM_LINK_LIBS ${AM_LINK_LIBS} 
                        ${CommonAPI-DBus_LIBRARY}
                        ${DBUS_LIBRARIES}
                        ${CommonAPI_LIBRARY})
   else()
       set (AM_LINK_LIBS ${AM_LINK_LIBS} 
                        ${CommonAPI-SomeIP_LIBRARY}
                        ${CommonAPI_LIBRARY})
   endif()
    
    TARGET_LINK_LIBRARIES(PluginCommandInterfaceCAPI ${AM_LINK_LIBS})
    
    IF(WITH_TESTS)
    	add_subdirectory (test)
    ENDIF(WITH_TESTS)
             
    install(TARGETS PluginCommandInterfaceCAPI          
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/command
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugin
    )

endif (${WITH_ENABLED_IPC} STREQUAL "CAPI")
