# Copyright (c) 2018 GENIVI Alliance
# Copyright (c) 2018 Advanced Driver Information Technology
#
#  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
#           Kapildev Patel, Yogesh sharma <kpatel@jp.adit-jv.com> 2018
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

cmake_minimum_required(VERSION 3.0)

PROJECT(CAmPersistenceDefaultTest VERSION 7.4.0)

set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

FIND_PACKAGE (Threads)
FIND_PACKAGE(PkgConfig)

INCLUDE_DIRECTORIES(${CONTROLLER_UTEST_INCLUDE_DIRECTORIES})

file(GLOB CAmPersistenceDefaultTest_SRCS_CXX 
    "CAmPersistenceDefaultTest.cpp"
)

FOREACH(SRC_FILE_ABSOLUTE_PATH IN LISTS CAmPersistenceDefaultTest_SRCS_CXX)
    GET_FILENAME_COMPONENT(SRC_FILE_NAME ${SRC_FILE_ABSOLUTE_PATH} NAME)
    SET_PROPERTY(SOURCE ${SRC_FILE_ABSOLUTE_PATH} PROPERTY COMPILE_DEFINITIONS "__FILENAME__=\"${SRC_FILE_NAME}\"")
ENDFOREACH()

ADD_EXECUTABLE(CAmPersistenceDefaultTest ${CAmPersistenceDefaultTest_SRCS_CXX})


TARGET_LINK_LIBRARIES(CAmPersistenceDefaultTest 
    ${CONTROLLER_UTEST_TARGET_LIBRARIES}
)

INSTALL(TARGETS CAmPersistenceDefaultTest 
        DESTINATION ${TEST_EXECUTABLE_INSTALL_PATH}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
        COMPONENT plugin-tests
)

