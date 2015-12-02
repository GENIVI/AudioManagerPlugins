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
            if(NOT TARGET gtest)
                add_subdirectory(${GOOGLE_MOCK_PROJECT_FOLDER} ${CMAKE_CURRENT_BINARY_DIR}/gmock)
            endif()
        endif(WITH_TESTS)
        
        set(COMMONAPI_DBUS_CONFIGURATIONS "") 
    else(AUDIOMANAGER_FOUND)
        message(FATAL_ERROR "Could not find audiomanager cmake configuration")
    endif(AUDIOMANAGER_FOUND)
endif (NOT DAEMONVERSION)        

IF(WITH_CAPI_WRAPPER)

    OPTION (WITH_ROUTING_INTERFACE_GENERIC_LIB
            "build routing iterface common-api glue code" OFF )

    set(INCLUDE_FOLDER "include")

    set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/routing)
    
	if(WITH_DLT)    
	    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
	    add_definitions(${DLT_CFLAGS_OTHER})
	    include_directories(${DLT_INCLUDE_DIRS})
	    link_directories(${DLT_LIBRARY_DIRS})
	endif(WITH_DLT)    

    INCLUDE_DIRECTORIES(
    	${CMAKE_SOURCE_DIR} 
     	${CMAKE_CURRENT_BINARY_DIR}
    	${AUDIO_INCLUDE_FOLDER}
    	${AUDIOMANAGER_INCLUDE_FOLDER}
    	${AUDIOMANAGER_UTILITIES_INCLUDE}
    	${INCLUDE_FOLDER}
    )
    
     # all source files go here
    file(GLOB PLUGINCAPI_SRCS_CXX "src/*.cpp" )


    SET(ROUTINGCONTROL_CAPI RoutingControlGenericCommonAPI )
    SET(ROUTINGCONTROL_CAPI_LIB lib${ROUTINGCONTROL_CAPI}.so )                
    SET(ROUTINGCONTROL_CAPI_LIB ${ROUTINGCONTROL_CAPI_LIB} PARENT_SCOPE )

    INCLUDE (cmake/CommonAPI.cmake)
    LOAD_COMMONAPI_LIB()

    IF(NOT WITH_ROUTING_INTERFACE_GENERIC_LIB ) 
        pkg_check_modules(RC RoutingInterfaceHUProviderSomeIpProxy)
        pkg_check_modules(RC1 RoutingInterfaceHUProviderSomeIpStub)
        IF(NOT RC_FOUND)
            SET(WITH_ROUTING_INTERFACE_GENERIC_LIB ON)
        ELSE(NOT RC_FOUND)
           SET(ROUTING_GEN_INCLUDE_DIR ${RC_INCLUDEDIR}) 
           INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ${COMMONAPI_INCLUDE_DIRS} ${RC_INCLUDE_DIRS} ${RC1_INCLUDE_DIRS})  
           link_directories(${RC_LIBRARY_DIRS} ${RC1_LIBRARY_DIRS})  
        ENDIF(NOT RC_FOUND)
    ENDIF()
    
    IF(WITH_ROUTING_INTERFACE_GENERIC_LIB)        
      
   	   	SET(SRC_GEN src-gen/v${COMMONAPI_VERSION_NUMBER}/Generic)
        # Find the common-api sources. 
        # The sources can be placed in the build directory (DESTINATION) or in the source tree (ALT_DESTINATION). 
        # The ALT_DESTINATION is used as fall back if the DESTINATION directory dosen't contain the sources or you just don't want to use the generator.
        COMMON_API_GENERATE_SOURCES(
                                    # Prefix.
                                    TARGET ROUTING 
                                    # A list with fidls for the generic generator. Leave this parameter empty if you won't generic code.
                                    FIDLS_GENERIC fidl/RoutingInterface.fidl 
                                    # A list with fidls for the binding generator. Leave this parameter empty if you won't binding code.
                                    FIDLS_BINDING #Empty
                                    # A relative path to the build directory or an absolute path.
                                    DESTINATION ${SRC_GEN} 
                                    # An alternative relative/absolute path with common-api sources, usually in the source tree.
                                    ALT_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src-gen 
                                    )
        
        INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
    		    ${VSOMEIP_INCLUDE_DIRS}
	    		${COMMONAPI_SOMEIP_INCLUDE_DIRS}
	    		${COMMONAPI_DBUS_INCLUDE_DIRS}
                ${DBUS_INCLUDE_DIRS}
                ${ROUTING_GEN_INCLUDE_DIR}
                ${COMMONAPI_INCLUDE_DIRS}
                )
                     
        add_library (${ROUTINGCONTROL_CAPI} SHARED ${ROUTING_GEN_COMMON_SOURCES} ) 	
    	target_link_libraries(${ROUTINGCONTROL_CAPI} ${CommonAPI_LIBRARY} )
    	
    	set_target_properties(${ROUTINGCONTROL_CAPI} PROPERTIES 
    	                            LINKER_LANGUAGE CXX 
    	                            LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_OUTPUT_PATH})	    	
                                    
      INSTALL (TARGETS ${ROUTINGCONTROL_CAPI}
              PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
              LIBRARY DESTINATION lib
              COMPONENT plugin)                   
       set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)       
#       link_directories(${CMAKE_INSTALL_PREFIX}/lib/${LIB_INSTALL_SUFFIX})  
       link_directories(${CMAKE_CURRENT_BINARY_DIR})   

      INSTALL( DIRECTORY ${ROUTING_GEN_INCLUDE_DIR} DESTINATION include/${LIB_INSTALL_SUFFIX} COMPONENT plugin)
      configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/plugin.pc.in ${CMAKE_BINARY_DIR}/${ROUTINGCONTROL_CAPI}.pc @ONLY )  
      install(FILES ${CMAKE_BINARY_DIR}/${ROUTINGCONTROL_CAPI}.pc DESTINATION lib/pkgconfig COMPONENT plugin)  
                        
   ENDIF()  
   
    add_library(PluginRoutingInterfaceCAPI MODULE ${PLUGINCAPI_SRCS_CXX})
    
    TARGET_LINK_LIBRARIES(PluginRoutingInterfaceCAPI 
        #${ROUTINGCONTROL_CAPI}
        ${RC_LIBRARIES}
        ${RC1_LIBRARIES}
    )
        
    INSTALL(TARGETS PluginRoutingInterfaceCAPI 
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/routing
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugin
    )

    IF(WITH_TESTS)
    	add_subdirectory (test)
    ENDIF(WITH_TESTS)
    
ENDIF(WITH_CAPI_WRAPPER)
