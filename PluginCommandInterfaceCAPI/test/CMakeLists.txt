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

PROJECT(CAmCommandSenderCAPITests)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -DUNIT_TEST=1 -DDLT_CONTEXT=AudioManager")

set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

FIND_PACKAGE(Threads)
FIND_PACKAGE(PkgConfig)
FIND_PACKAGE(PythonLibs REQUIRED)

IF(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)   
ENDIF(WITH_DLT)

INCLUDE_DIRECTORIES(   
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${CMAKE_SOURCE_DIR} 
    ${COMMONAPI_INCLUDE_DIRS}
	${COMMAND_CTR_GEN_INCLUDE_DIR}
    ${INCLUDE_FOLDER}
    ${PYTHON_INCLUDE_DIRS}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    "${AUDIOMANAGER_DAEMON_FOLDER}/include"
    "../include"
)

   
file(GLOB CAPI_PLUGIN_INTERFACE_SRCS_CXX 
     "${AUDIOMANAGER_UTILITIES_SRC}/CAmCommonAPIWrapper.cpp"
     "${AUDIOMANAGER_UTILITIES_SRC}/CAmSocketHandler.cpp"
     "${AUDIOMANAGER_UTILITIES_SRC}/CAmDltWrapper.cpp" 
     "CAmCommandSenderCAPITest.cpp"
     "CAmTestCAPIWrapper.cpp"
     "CAmCommandSenderCommon.cpp"
     "../src/*.cpp"
)

SET(CAPI_PLUGIN_INTERFACE_SRCS_CXX ${CAPI_PLUGIN_INTERFACE_SRCS_CXX} ${COMMAND_CTR_GEN_SOURCES})
#if(${SELECTED_CAPI_BINDING} EQUAL 0)# CommonAPI-DBus
    INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
            ${COMMONAPI_DBUS_INCLUDE_DIRS}
            ${DBUS_INCLUDE_DIRS})	
  	        
#else()# CommonAPI-SomeIP
   INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
            ${VSOMEIP_INCLUDE_DIRS}
            ${COMMONAPI_SOMEIP_INCLUDE_DIRS})    
#endif()

link_directories(
        ${LINK_DIRECTORIES}
 
)
ADD_EXECUTABLE(AmCommandSenderCAPITest ${CAPI_PLUGIN_INTERFACE_SRCS_CXX})

TARGET_LINK_LIBRARIES(AmCommandSenderCAPITest 
    ${DLT_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PYTHON_LIBRARY}
    ${PC_GSTREAMER_LIBRARIES}
    ${CommonAPI_LIBRARY}
    ${DBUS_LIBRARIES}
    gtest
    gmock
)

TARGET_LINK_LIBRARIES(AmCommandSenderCAPITest  ${CommonAPI-${BINDING_SUFFIX}_LIBRARY} )
    

INSTALL(TARGETS AmCommandSenderCAPITest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT plugin-tests
)

IF(CONFIG_INI_TEST_PATH)
    FILE(READ ${CONFIG_INI_TEST_PATH} CONFIG_INI_TEST)
    STRING(REPLACE "[stub]" 
                    "[stub]\n${CommandControl}=../plugins/${STUB_COMMANDCONTROL_BINDING_LIB}" 
                   CONFIG_INI_TEST 
                   ${CONFIG_INI})
    STRING(REPLACE "[proxy]" 
                   "[proxy]\n${CommandControl}=../plugins/${PROXY_COMMANDCONTROL_BINDING_LIB}" 
                   CONFIG_INI_TEST 
                   ${CONFIG_INI_TEST})
    FILE(WRITE ${CONFIG_INI_TEST_PATH} ${CONFIG_INI_TEST}) 
    
    configure_file( ${CONFIG_INI_TEST_PATH} ${TEST_EXECUTABLE_OUTPUT_PATH}/commonapi.ini COPYONLY)
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/vsomeip.json ${TEST_EXECUTABLE_OUTPUT_PATH}/vsomeip.json COPYONLY)  
    
    GET_PROPERTY(TEST_CONFIG_INSTALLED GLOBAL PROPERTY CAPI_TEST_CONFIG_INSTALLED)
    IF(NOT TEST_CONFIG_INSTALLED)
        SET_PROPERTY(GLOBAL PROPERTY CAPI_TEST_CONFIG_INSTALLED ON)
        INSTALL(FILES ${TEST_EXECUTABLE_OUTPUT_PATH}/commonapi.ini ${TEST_EXECUTABLE_OUTPUT_PATH}/vsomeip.json
                                DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
                                PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
                                COMPONENT plugin-tests)
    ENDIF(NOT TEST_CONFIG_INSTALLED) 
ENDIF(CONFIG_INI_TEST_PATH)
