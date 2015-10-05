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

    GET_PROPERTY(_USE_CAPI_BINDING GLOBAL PROPERTY USE_CAPI_BINDING)
    IF(NOT _USE_CAPI_BINDING)
        set (USE_CAPI_BINDING "DBUS" CACHE STRING "Select prefered binding for common-api 'DBUS' or 'SOMEIP'")
        set_property(CACHE USE_CAPI_BINDING PROPERTY STRINGS "DBUS" "SOMEIP") 
        
        IF (USE_CAPI_BINDING STREQUAL "SOMEIP")
            set(SELECTED_CAPI_BINDING 1 CACHE INTERNAL "hide this!" FORCE)
        ELSEIF (USE_CAPI_BINDING STREQUAL "DBUS")
            set(SELECTED_CAPI_BINDING 0 CACHE INTERNAL "hide this!" FORCE)
        ELSE()
            unset(SELECTED_CAPI_BINDING CACHE)
        ENDIF() 
        set (WITH_CAPI_GENERATOR_IF_AVAILABLE ON CACHE INTERNAL "hide this!" FORCE)
    ENDIF ()

    OPTION (WITH_ROUTING_INTERFACE_GLUE_LIBS
            "build routing iterface common-api glue code" OFF )

    set(INCLUDE_FOLDER "include")
    # Include CommonAPI ... 
    INCLUDE (cmake/CommonAPI.cmake)
    
    # load all the necessary libraries 
    # LOAD_COMMONAPI() without argument will use the CACHE variable SELECTED_CAPI_BINDING (0-DBus, 1-SomeIP).
    # It can be called with DBUS or SOMEIP as parameter: LOAD_COMMONAPI(DBUS) , LOAD_COMMONAPI(SOMEIP) which will overwrite SELECTED_CAPI_BINDING.
    LOAD_COMMONAPI_LIB()
    LOAD_COMMONAPI_DBUS_LIB()
	LOAD_COMMONAPI_SOMEIP_LIB()
    
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
  
	IF(${SELECTED_CAPI_BINDING} EQUAL 1)#SomeIP
	    SET(BINDING_SUFFIX SomeIP)
	  	SET(SRC_GEN src-gen/v${COMMONAPI_SOMEIP_VERSION_NUMBER}/${BINDING_SUFFIX})
		SET(CAPI_BINDING_FIDLS fidl/RoutingInterface.fdepl fidl/AudioManagerTypes.fdepl ) # fidl/RoutingInterfaceProviderSomeIp.fdepl	
	ELSE()#Dbus
	    SET(BINDING_SUFFIX DBus)
	   	SET(SRC_GEN src-gen/v${COMMONAPI_DBUS_VERSION_NUMBER}/${BINDING_SUFFIX})
	    SET(CAPI_BINDING_FIDLS fidl/RoutingInterface.fidl fidl/AudioManagerTypes.fdepl)	
	ENDIF()
    COMMON_API_GENERATE_SOURCES(
                                # Prefix.
                                TARGET ROUTING 
                                # A list with fidls for the generic generator.
                                FIDLS_GENERIC fidl/RoutingInterface.fidl 
                                # A list with fidls for the binding generator.
                                FIDLS_BINDING ${CAPI_BINDING_FIDLS}
                                # A relative path to the build directory or an absolute path.
                                DESTINATION ${SRC_GEN} 
                                # An alternative relative/absolute path with common-api sources, usually in the source tree.
                                ALT_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src-gen 
                                )
	              
	message (status "dfdfdb ${COMMON_API_ROUTING_INTERFACE_GEN_INCLUDE_DIR}")                  
    
    INCLUDE_DIRECTORIES(
    	${CMAKE_SOURCE_DIR} 
     	${CMAKE_CURRENT_BINARY_DIR}
    	${AUDIO_INCLUDE_FOLDER}
    	${AUDIOMANAGER_INCLUDE_FOLDER}
    	${AUDIOMANAGER_UTILITIES_INCLUDE}
    	${COMMONAPI_INCLUDE_DIRS}
    	${ROUTING_GEN_INCLUDE_DIR}
    	${INCLUDE_FOLDER}
    )
    
     # all source files go here
    file(GLOB PLUGINCAPI_SRCS_CXX "src/*.cpp" )
	SET(PLUGINCAPI_SRCS_CXX 
	    ${PLUGINCAPI_SRCS_CXX} 
        ${ROUTING_GEN_COMMON_SOURCES}
	    )
	
   	INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
	    		    ${VSOMEIP_INCLUDE_DIRS}
    	    		${COMMONAPI_SOMEIP_INCLUDE_DIRS}
    	    		${COMMONAPI_DBUS_INCLUDE_DIRS}
                    ${DBUS_INCLUDE_DIRS}
                    ${ROUTING_GEN_INCLUDE_DIR}
                    )
	
    IF(WITH_ROUTING_INTERFACE_GLUE_LIBS)
        SET(STUB_ROUTINGCONTROL_BINDING RoutingStubs-${BINDING_SUFFIX} )
        SET(PROXY_ROUTINGCONTROL_BINDING RoutingProxies-${BINDING_SUFFIX} )
                
        SET(STUB_ROUTINGCONTROL_BINDING_LIB lib${STUB_ROUTINGCONTROL_BINDING}.so )
        SET(PROXY_ROUTINGCONTROL_BINDING_LIB lib${PROXY_ROUTINGCONTROL_BINDING}.so )
                
        SET(STUB_ROUTINGCONTROL_BINDING_LIB ${STUB_ROUTINGCONTROL_BINDING_LIB} PARENT_SCOPE )
        SET(PROXY_ROUTINGCONTROL_BINDING_LIB ${PROXY_ROUTINGCONTROL_BINDING_LIB} PARENT_SCOPE )
                      
        add_library (${STUB_ROUTINGCONTROL_BINDING} SHARED ${ROUTING_GEN_BINDING_STUB_SOURCES} ${ROUTING_GEN_BINDING_DEPLOYMENT_SOURCES})
        add_library (${PROXY_ROUTINGCONTROL_BINDING} SHARED ${ROUTING_GEN_BINDING_PROXY_SOURCES} ${ROUTING_GEN_BINDING_DEPLOYMENT_SOURCES})
                  	
    	target_link_libraries(${STUB_ROUTINGCONTROL_BINDING} ${CommonAPI-${BINDING_SUFFIX}_LIBRARY})
	    target_link_libraries(${PROXY_ROUTINGCONTROL_BINDING} ${CommonAPI-${BINDING_SUFFIX}_LIBRARY})
    	
    	set_target_properties(${STUB_ROUTINGCONTROL_BINDING} PROPERTIES 
    	                            LINKER_LANGUAGE CXX 
    	                            LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_OUTPUT_PATH})
    	set_target_properties(${PROXY_ROUTINGCONTROL_BINDING} PROPERTIES 
    	                            LINKER_LANGUAGE CXX 
    	                            LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_OUTPUT_PATH})  	    	
        
        INSTALL(TARGETS ${PROXY_ROUTINGCONTROL_BINDING} ${STUB_ROUTINGCONTROL_BINDING}
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugin)
                          
   ENDIF(WITH_ROUTING_INTERFACE_GLUE_LIBS)  
   
    GET_PROPERTY(CONFIG_INSTALLED GLOBAL PROPERTY CAPI_CONFIG_INSTALLED)

    IF(NOT CONFIG_INSTALLED)
        SET_PROPERTY(GLOBAL PROPERTY CAPI_CONFIG_INSTALLED ON)
        SET(CONFIG_INI_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/commonapi4${BINDING_SUFFIX}.ini )
        configure_file(${CONFIG_INI_PATH} ${CMAKE_BINARY_DIR}/commonapi-bin.ini COPYONLY)
        configure_file(${CONFIG_INI_PATH} ${CMAKE_BINARY_DIR}/commonapi-depl.ini COPYONLY) 
                    
        SET(CONFIG_INI_DEPL_PATH ${CMAKE_BINARY_DIR}/commonapi-depl.ini)
        SET(CONFIG_INI_DEPL_PATH ${CONFIG_INI_DEPL_PATH} PARENT_SCOPE)
        SET(CONFIG_INI_BIN_PATH ${CMAKE_BINARY_DIR}/commonapi-bin.ini)
        SET(CONFIG_INI_BIN_PATH ${CONFIG_INI_BIN_PATH} PARENT_SCOPE)
        configure_file(${CONFIG_INI_PATH} ${CMAKE_BINARY_DIR}/commonapi-test.ini COPYONLY)      
        SET(CONFIG_INI_TEST_PATH ${CMAKE_BINARY_DIR}/commonapi-test.ini)
        SET(CONFIG_INI_TEST_PATH ${CONFIG_INI_TEST_PATH} PARENT_SCOPE)   
    
        INSTALL(FILES ${CMAKE_BINARY_DIR}/commonapi-depl.ini
                    DESTINATION /etc
                    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
                    COMPONENT bin
                    RENAME commonapi.ini) 
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vsomeip.json
                DESTINATION /etc
                PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
                COMPONENT bin) 
        
    ENDIF (NOT CONFIG_INSTALLED)
       
       
    FILE(READ ${CONFIG_INI_DEPL_PATH} CONFIG_INI)
    
    SET(RoutingControlObserver "local:org.genivi.am.routinginterface.RoutingControlObserver:org.genivi.audiomanager") 
    SET(RoutingControl "local:org.genivi.am.routinginterface.RoutingControl:RoutingControlInstance2") 
    IF(STUB_ROUTINGCONTROL_BINDING_LIB AND PROXY_ROUTINGCONTROL_BINDING_LIB) 
        STRING(REPLACE "[stub]" 
                        "[stub]\n${RoutingControlObserver}=../lib/audiomanager/plugins/${STUB_ROUTINGCONTROL_BINDING_LIB}\n${RoutingControl}=../lib/audiomanager/plugins/${STUB_ROUTINGCONTROL_BINDING_LIB}" 
                       CONFIG_INI_DEPLYMENT 
                       ${CONFIG_INI})
        STRING(REPLACE "[proxy]" 
                       "[proxy]\n${RoutingControlObserver}=../lib/audiomanager/plugins/${PROXY_ROUTINGCONTROL_BINDING_LIB}\n${RoutingControl}=../lib/audiomanager/plugins/${PROXY_ROUTINGCONTROL_BINDING_LIB}" 
                       CONFIG_INI_DEPLYMENT 
                       ${CONFIG_INI_DEPLYMENT})
    ELSE()
        SET(CONFIG_INI_DEPLYMENT ${CONFIG_INI})                                    
    ENDIF()               
    FILE(WRITE ${CONFIG_INI_DEPL_PATH} ${CONFIG_INI_DEPLYMENT})          
    
    IF(NOT ${EXECUTABLE_OUTPUT_PATH} STREQUAL "")    
        FILE(READ ${CONFIG_INI_BIN_PATH} CONFIG_INI)
        IF(STUB_ROUTINGCONTROL_BINDING_LIB AND PROXY_ROUTINGCONTROL_BINDING_LIB) 
            STRING(REPLACE "[stub]" 
                        "[stub]\n${RoutingControlObserver}=plugins/${STUB_ROUTINGCONTROL_BINDING_LIB}\n${RoutingControl}=plugins/${STUB_ROUTINGCONTROL_BINDING_LIB}" 
                       CONFIG_INI_BIN 
                       ${CONFIG_INI})
            STRING(REPLACE "[proxy]" 
                           "[proxy]\n${RoutingControlObserver}=plugins/${PROXY_ROUTINGCONTROL_BINDING_LIB}\n${RoutingControl}=plugins/${PROXY_ROUTINGCONTROL_BINDING_LIB}" 
                           CONFIG_INI_BIN 
                           ${CONFIG_INI_BIN})
        ELSE()
             SET(CONFIG_INI_BIN ${CONFIG_INI})                                    
        ENDIF()
        FILE(WRITE ${CONFIG_INI_BIN_PATH} ${CONFIG_INI_BIN})               
        configure_file( ${CONFIG_INI_BIN_PATH}  ${EXECUTABLE_OUTPUT_PATH}/commonapi.ini COPYONLY)
        configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vsomeip.json ${EXECUTABLE_OUTPUT_PATH}/vsomeip.json COPYONLY)
    
    ENDIF()	    		

     
    add_library(PluginRoutingInterfaceCAPI MODULE ${PLUGINCAPI_SRCS_CXX})
    
    TARGET_LINK_LIBRARIES(PluginRoutingInterfaceCAPI 
        ${DLT_LIBRARIES}
        ${CommonAPI_LIBRARY}
#        ${DBUS_LIBRARIES}
    )
    
#   if(${SELECTED_CAPI_BINDING} EQUAL 0)
#		TARGET_LINK_LIBRARIES(PluginRoutingInterfaceCAPI  
#		                ${DBUS_LIBRARIES} 
#		                ${CommonAPI-DBus_LIBRARY} 
#		                )
#   else()
#		TARGET_LINK_LIBRARIES(PluginRoutingInterfaceCAPI  
#                        ${vsomeip_LIBRARY} 
#                        ${CommonAPI-SomeIP_LIBRARY} 
#                        )
#   endif()
        
    INSTALL(TARGETS PluginRoutingInterfaceCAPI 
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/routing
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugin
    )

    IF(WITH_TESTS)
    	add_subdirectory (test)
    ENDIF(WITH_TESTS)
    
ENDIF(WITH_CAPI_WRAPPER)
