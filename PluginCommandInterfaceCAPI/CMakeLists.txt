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

set(LIBRARY_OUTPUT_PATH ${PLUGINS_OUTPUT_PATH}/command)

if (WITH_CAPI_WRAPPER)
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

    OPTION (WITH_COMMAND_INTERFACE_GLUE_LIBS
            "build command interface common-api glue code" OFF )

	if(WITH_DLT)    
	    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
	    add_definitions(${DLT_CFLAGS_OTHER})
	    include_directories(${DLT_INCLUDE_DIRS})
	    link_directories(${DLT_LIBRARY_DIRS})
	endif(WITH_DLT)

    set (AM_LINK_LIBS 
        ${CMAKE_DL_LIBS}
        rt)
    
    set(INCLUDE_FOLDER "include")
        
    # Include CommonAPI ... 
    INCLUDE (cmake/CommonAPI.cmake)
    
    # load all the necessary libraries 
    # LOAD_COMMONAPI() without argument will use the CACHE variable SELECTED_CAPI_BINDING (0-DBus, 1-SomeIP).
    # It can be called with DBUS or SOMEIP as parameter: LOAD_COMMONAPI(DBUS) , LOAD_COMMONAPI(SOMEIP) which will overwrite SELECTED_CAPI_BINDING.
    LOAD_COMMONAPI_LIB()
    
    IF(${SELECTED_CAPI_BINDING} EQUAL 1)#SomeIP
        LOAD_COMMONAPI_SOMEIP_LIB()
        SET(BINDING_SUFFIX SomeIP)
	  	SET(SRC_GEN src-gen/v${COMMONAPI_SOMEIP_VERSION_NUMBER}/${BINDING_SUFFIX})
		SET(CAPI_BINDING_FIDLS fidl/CommandInterface.fdepl fidl/AudioManagerTypes.fdepl ) #fidl/CommandInterfaceProviderSomeIp.fdepl	
	ELSE()#Dbus
	    LOAD_COMMONAPI_DBUS_LIB()
	    SET(BINDING_SUFFIX DBus)
	   	SET(SRC_GEN src-gen/v${COMMONAPI_DBUS_VERSION_NUMBER}/${BINDING_SUFFIX})
	    SET(CAPI_BINDING_FIDLS fidl/CommandInterface.fidl fidl/AudioManagerTypes.fidl)	
	ENDIF()
    
    # Find the common-api sources. 
    # The sources can be placed in the build directory (DESTINATION) or in the source tree (ALT_DESTINATION). 
    # The ALT_DESTINATION is used as fall back if the DESTINATION directory dosen't contain the sources or you just don't want to use the generator.
    COMMON_API_GENERATE_SOURCES(
                                # Prefix.
                                TARGET COMMAND_CTR 
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
    	${COMMAND_CTR_GEN_INCLUDE_DIR}
    	${INCLUDE_FOLDER}
    )
 
        # all source files go here
    file(GLOB PLUGINCAPI_SRCS_CXX "src/*.cpp" )

    SET(PLUGINCAPI_SRCS_CXX ${PLUGINCAPI_SRCS_CXX} ${COMMAND_CTR_GEN_COMMON_SOURCES})
    
    INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
            ${COMMONAPI_DBUS_INCLUDE_DIRS}
            ${DBUS_INCLUDE_DIRS}	   
            ${VSOMEIP_INCLUDE_DIRS}
            ${COMMONAPI_SOMEIP_INCLUDE_DIRS})
            
    IF(WITH_COMMAND_INTERFACE_GLUE_LIBS)
        
        SET(STUB_COMMANDCONTROL_BINDING CommandControlStubs-${BINDING_SUFFIX} )
        SET(PROXY_COMMANDCONTROL_BINDING CommandControlProxies-${BINDING_SUFFIX} )
        
        SET(STUB_COMMANDCONTROL_BINDING_LIB lib${STUB_COMMANDCONTROL_BINDING}.so )
        SET(PROXY_COMMANDCONTROL_BINDING_LIB lib${PROXY_COMMANDCONTROL_BINDING}.so )
        
        SET(STUB_COMMANDCONTROL_BINDING_LIB ${STUB_COMMANDCONTROL_BINDING_LIB} PARENT_SCOPE)
        SET(PROXY_COMMANDCONTROL_BINDING_LIB ${PROXY_COMMANDCONTROL_BINDING_LIB} PARENT_SCOPE)
              
        add_library (${STUB_COMMANDCONTROL_BINDING} SHARED ${COMMAND_CTR_GEN_BINDING_STUB_SOURCES} ${COMMAND_CTR_GEN_BINDING_DEPLOYMENT_SOURCES})
        add_library (${PROXY_COMMANDCONTROL_BINDING} SHARED ${COMMAND_CTR_GEN_BINDING_PROXY_SOURCES} ${COMMAND_CTR_GEN_BINDING_DEPLOYMENT_SOURCES})
        
    	target_link_libraries(${STUB_COMMANDCONTROL_BINDING} ${CommonAPI-${BINDING_SUFFIX}_LIBRARY})
	    target_link_libraries(${PROXY_COMMANDCONTROL_BINDING} ${CommonAPI-${BINDING_SUFFIX}_LIBRARY})
        
    	set_target_properties(${STUB_COMMANDCONTROL_BINDING} PROPERTIES 
    	                            LINKER_LANGUAGE CXX 
    	                            LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_OUTPUT_PATH})
    	set_target_properties(${PROXY_COMMANDCONTROL_BINDING} PROPERTIES 
    	                            LINKER_LANGUAGE CXX 
    	                            LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_OUTPUT_PATH})
    	                            
    	INSTALL(TARGETS ${STUB_COMMANDCONTROL_BINDING} ${PROXY_COMMANDCONTROL_BINDING}
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugin) 

         IF(WITH_TESTS)
             INSTALL(TARGETS ${PROXY_COMMANDCONTROL_BINDING}
                        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
                        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
                        COMPONENT plugin-tests)  
             INSTALL(TARGETS ${STUB_COMMANDCONTROL_BINDING}
                        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
                        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
                        COMPONENT plugin-tests)             
         ENDIF(WITH_TESTS)                        
	ENDIF(WITH_COMMAND_INTERFACE_GLUE_LIBS)                          

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
         ENDIF(NOT CONFIG_INSTALLED)   
            
        FILE(READ ${CONFIG_INI_DEPL_PATH} CONFIG_INI)
        FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/cmake/command-stub.ini CONFIG_STUB_SECTION_INI)
        FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/cmake/command-proxy.ini CONFIG_PROXY_SECTION_INI)


        IF(STUB_COMMANDCONTROL_BINDING_LIB AND PROXY_COMMANDCONTROL_BINDING_LIB)       
            FILE(RELATIVE_PATH REL_PATH ${CMAKE_INSTALL_PREFIX}/bin ${CMAKE_INSTALL_PREFIX}/lib/${LIB_INSTALL_SUFFIX} )
            
            STRING(REPLACE "REL_PATH" 
                            "${REL_PATH}/${STUB_COMMANDCONTROL_BINDING_LIB}" 
                           CONFIG_STUB_SECTION 
                           ${CONFIG_STUB_SECTION_INI})
            STRING(REPLACE "REL_PATH" 
                            "${REL_PATH}/${PROXY_COMMANDCONTROL_BINDING_LIB}" 
                           CONFIG_PROXY_SECTION 
                           ${CONFIG_PROXY_SECTION_INI})                             
            STRING(REPLACE "[stub]" 
                           ${CONFIG_STUB_SECTION} 
                           CONFIG_INI_DEPLOYMENT 
                           ${CONFIG_INI})
            STRING(REPLACE "[proxy]" 
                           ${CONFIG_PROXY_SECTION} 
                           CONFIG_INI_DEPLOYMENT 
                           ${CONFIG_INI_DEPLOYMENT})
        ELSE()
             SET(CONFIG_INI_DEPLOYMENT ${CONFIG_INI})                 
        ENDIF()               
        FILE(WRITE ${CONFIG_INI_DEPL_PATH} ${CONFIG_INI_DEPLOYMENT})          
        
        IF(NOT ${EXECUTABLE_OUTPUT_PATH} STREQUAL "")    
            FILE(READ ${CONFIG_INI_BIN_PATH} CONFIG_INI)
            IF(STUB_COMMANDCONTROL_BINDING_LIB AND PROXY_COMMANDCONTROL_BINDING_LIB) 
            
                STRING(REPLACE "REL_PATH" 
                               "plugins/${STUB_COMMANDCONTROL_BINDING_LIB}" 
                               CONFIG_STUB_SECTION 
                               ${CONFIG_STUB_SECTION_INI})
                STRING(REPLACE "REL_PATH" 
                               "plugins/${PROXY_COMMANDCONTROL_BINDING_LIB}" 
                               CONFIG_PROXY_SECTION 
                               ${CONFIG_PROXY_SECTION_INI})    
                STRING(REPLACE "[stub]" 
                               ${CONFIG_STUB_SECTION} 
                               CONFIG_INI_BIN 
                               ${CONFIG_INI})
                STRING(REPLACE "[proxy]" 
                               ${CONFIG_PROXY_SECTION}
                               CONFIG_INI_BIN 
                               ${CONFIG_INI_BIN})
            ELSE()
                 SET(CONFIG_INI_BIN ${CONFIG_INI})                 
            ENDIF() 
            FILE(WRITE ${CONFIG_INI_BIN_PATH} ${CONFIG_INI_BIN})               

            configure_file( ${CONFIG_INI_BIN_PATH}  ${EXECUTABLE_OUTPUT_PATH}/commonapi.ini COPYONLY)
            configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vsomeip.json ${EXECUTABLE_OUTPUT_PATH}/vsomeip.json COPYONLY)

        ENDIF()
   
    IF(WITH_DLT)
        INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES} ${DLT_INCLUDE_DIRS})
        set (AM_LINK_LIBS ${AM_LINK_LIBS} ${DLT_LIBRARIES})
    ENDIF(WITH_DLT)    
        
    add_library(PluginCommandInterfaceCAPI MODULE ${PLUGINCAPI_SRCS_CXX})
    set (AM_LINK_LIBS ${AM_LINK_LIBS} ${CommonAPI_LIBRARY})
    
    TARGET_LINK_LIBRARIES(PluginCommandInterfaceCAPI ${AM_LINK_LIBS})
                 
    install(TARGETS PluginCommandInterfaceCAPI          
            DESTINATION lib/${LIB_INSTALL_SUFFIX}/command
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
            COMPONENT plugin
    )
    
    IF(WITH_TESTS)
    	add_subdirectory (test)
    ENDIF(WITH_TESTS)   

endif (WITH_CAPI_WRAPPER)
