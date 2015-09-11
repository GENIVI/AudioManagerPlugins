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
#FIND_PACKAGE(CommonAPI REQUIRED)

IF(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)   
ENDIF(WITH_DLT)

INCLUDE_DIRECTORIES(   
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${CMAKE_SOURCE_DIR} 
    ${COMMONAPI_INCLUDE_DIRS}
	${COMMON_API_COMMAND_INTERFACE_GEN_INCLUDE_DIR}
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

SET(CAPI_PLUGIN_INTERFACE_SRCS_CXX ${CAPI_PLUGIN_INTERFACE_SRCS_CXX} ${COMMON_API_COMMAND_INTERFACE_GEN_SOURCES})
if(${COMMONAPI_USED_BINDING} EQUAL 0)# CommonAPI-DBus
    INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
            ${COMMONAPI_DBUS_INCLUDE_DIRS}
            ${DBUS_INCLUDE_DIRS})	
  	        
else()# CommonAPI-SomeIP
   INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
            ${VSOMEIP_INCLUDE_DIRS}
            ${COMMONAPI_SOMEIP_INCLUDE_DIRS})
#    SET(DEFAULT_SOMEIP_BINDING_LIB local__org_genivi_am_commandinterface_CommandControl__org_genivi_audiomanager)    
#    if(NOT TARGET ${DEFAULT_SOMEIP_BINDING_LIB})     
#		add_library (${DEFAULT_SOMEIP_BINDING_LIB} SHARED ${COMMON_API_COMMAND_INTERFACE_GEN_SOURCES})
#		target_link_libraries(${DEFAULT_SOMEIP_BINDING_LIB} ${CommonAPI-SomeIP_LIBRARY})
#		set_target_properties(${DEFAULT_SOMEIP_BINDING_LIB} PROPERTIES LINKER_LANGUAGE CXX)
#	else()
#		link_libraries (${DEFAULT_SOMEIP_BINDING_LIB})
#	endif()	
endif()

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

if(${COMMONAPI_USED_BINDING} EQUAL 0)
		TARGET_LINK_LIBRARIES(AmCommandSenderCAPITest  ${CommonAPI-DBus_LIBRARY} )
else()
		TARGET_LINK_LIBRARIES(AmCommandSenderCAPITest  ${CommonAPI-SomeIP_LIBRARY} )
 endif()
    

INSTALL(TARGETS AmCommandSenderCAPITest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT plugin-tests
)

