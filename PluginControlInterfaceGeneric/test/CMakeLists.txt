# Copyright (c) 2018 GENIVI Alliance
# Copyright (c) 2018 Advanced Driver Information Technology
#
#  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
#           Kapildev Patel, Satish Dwivedi <kpatel@jp.adit-jv.com> 2018
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

set(EXECUTABLE_OUTPUT_PATH ${TEST_EXECUTABLE_OUTPUT_PATH})

FIND_PACKAGE (Threads)
FIND_PACKAGE(PkgConfig)

include(FindPkgConfig)
pkg_search_module(GTEST REQUIRED gtest>=1.6)
pkg_search_module(GMOCK REQUIRED gmock >= 1.6)

link_directories(
     ${LINK_DIRECTORIES} 
     ${CMAKE_BINARY_DIR}
     ${GTEST_LIBRARY_DIRS}
     ${GMOCK_LIBRARY_DIRS}
     ${CC_LIBDIR}
)

set (CONTROLLER_UTEST_INCLUDE_DIRECTORIES
    ${CONTROLLER_INCLUDE_DIR}
    ${STD_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR} 
    ${INCLUDE_FOLDER}
    "../../include"
    "../test"
    "../"
)

set (CONTROLLER_UTEST_TARGET_LIBRARIES
    ${LIB_CONTROLLER_UTEST}
    ${DLT_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GMOCK_LIBRARIES}
    ${GTEST_LIBRARIES}
    libgtest_main.a
    rt
    xml2
    pthread
    ${AudioManagerUtilities_LIBRARIES}
)

add_subdirectory (CAmControllerPluginTest)

