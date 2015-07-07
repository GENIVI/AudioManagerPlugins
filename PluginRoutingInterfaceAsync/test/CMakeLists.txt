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

PROJECT(AmRoutingReceiverAsync)

set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

FIND_PACKAGE(Threads)
FIND_PACKAGE(PkgConfig)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -DUNIT_TEST=1 -DDLT_CONTEXT=AudioManager")


IF(WITH_DLT)
    pkg_check_modules(DLT REQUIRED automotive-dlt)
ENDIF(WITH_DLT)

IF(WITH_DATABASE_STORAGE)
	pkg_check_modules(SQLITE REQUIRED sqlite3)
ENDIF(WITH_DATABASE_STORAGE)

INCLUDE_DIRECTORIES(   
    ${STD_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${CMAKE_SOURCE_DIR} 
    ${INCLUDE_FOLDER}
    ${GOOGLE_TEST_INCLUDE_DIR}
    ${GMOCK_INCLUDE_DIR}
    ${AUDIOMANAGER_UTILITIES_INCLUDE}
    "${AUDIOMANAGER_DAEMON_FOLDER}/include"
)

IF(WITH_DLT)
INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES}
    ${DLT_INCLUDE_DIRS}
)
ENDIF(WITH_DLT)

 
file(GLOB ASYNC_PLUGIN_INTERFACE_SRCS_CXX 
     "${AUDIOMANAGER_UTILITIES_SRC}/CAmSocketHandler.cpp"
     "${AUDIOMANAGER_UTILITIES_SRC}/CAmDltWrapper.cpp"
     "../src/*.cpp"  
     "CAmRoutingReceiverAsync.cpp" 
)


ADD_EXECUTABLE(AmRoutingReceiveAsyncTest ${ASYNC_PLUGIN_INTERFACE_SRCS_CXX})

TARGET_LINK_LIBRARIES(AmRoutingReceiveAsyncTest 
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    rt
    gtest
    gmock
)

ADD_DEPENDENCIES(AmRoutingReceiveAsyncTest gtest gmock)

IF(WITH_DLT)
    TARGET_LINK_LIBRARIES(AmRoutingReceiveAsyncTest
        ${DLT_LIBRARIES} 
    )
ENDIF(WITH_DLT) 

INSTALL(TARGETS AmRoutingReceiveAsyncTest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT tests
)

