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

pkg_check_modules (GTEST REQUIRED "gtest >= 1.6.0")
pkg_check_modules (GMOCK REQUIRED "gmock >= 1.6.0")


link_directories(
     ${LINK_DIRECTORIES} 
     ${CMAKE_BINARY_DIR}
     ${CC_LIBDIR}
)

set (CONTROLLER_UTEST_INCLUDE_DIRECTORIES
    ${CONTROLLER_INCLUDE_DIR}
    ${STD_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${AUDIO_INCLUDE_FOLDER}
    ${CMAKE_SOURCE_DIR} 
    ${DLT_INCLUDE_DIRS}
    ${INCLUDE_FOLDER}
    "${AUDIOMANAGER_DAEMON_FOLDER}/include"
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
    rt
    xml2
    AudioManagerUtilities
)

add_subdirectory(CAmActionCommandTest)
add_subdirectory(CAmActionContainerTest)
add_subdirectory(CAmRouteActionConnectTest)
add_subdirectory(CAmRouteActionDisconnectTest)
add_subdirectory(CAmSourceActionSetStateTest)
add_subdirectory(CAmSourceSinkActionSetSoundPropertyTest)
add_subdirectory(CAmMainConnectionActionConnectTest)
add_subdirectory (CAmMainConnectionActionDisconnectTest)
add_subdirectory (CAmMainConnectionActionSuspendTest)  
add_subdirectory (CAmClassActionConnectTest)
add_subdirectory (CAmClassActionDisconnectTest)
add_subdirectory (CAmClassActionInterruptTest)
add_subdirectory (CAmClassActionSuspendTest)
add_subdirectory (CAmSystemActionRegisterTest)
add_subdirectory (CAmSourceSinkActionSetNotificationConfigurationTest)
add_subdirectory (CAmPolicyReceiveTest)
add_subdirectory (CAmControllerPluginTest)
add_subdirectory (CAmTriggerQueueTest)
add_subdirectory (CAmClassElementTest)
add_subdirectory (CAmSinkElementTest)
add_subdirectory (CAmMainConnectionElementTest)
add_subdirectory (CAmSourceElementTest)
add_subdirectory (CAmDomainElementTest)
add_subdirectory (CAmRouteElementTest)
add_subdirectory (CAmEventSubjectTest)
add_subdirectory (CAmTimerEventTest)
add_subdirectory (CAmPolicyActionTest)
add_subdirectory (CAmSystemActionSetPropertyTest) 
add_subdirectory (CAmSystemElementTest)
add_subdirectory (CAmPersistenceDefaultTest)
add_subdirectory (CAmElementTest)
add_subdirectory (CAmPolicySendTest)
add_subdirectory (CAmPolicyEngineTest)


