/*******************************************************************************
 *  \copyright (c) 2018 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
 *           Kapildev Patel, Satish Dwivedi <kpatel@jp.adit-jv.com> 2018
 *
 *  \copyright The MIT License (MIT)
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 ******************************************************************************/

#include <map>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmElement.h"
#include "CAmClassElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmActionSuspend.h"
#include "CAmMainConnectionElement.h"
#include "CAmRouteElement.h"
#include "CAmSystemElement.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmClassActionSuspendTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmClassActionSuspendTest
 *@brief : This class is used to test the CAmClassActionSuspend class functionality.
 */
CAmClassActionSuspendTest::CAmClassActionSuspendTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpSinkElement(NULL)
    , mpSourceElement(NULL)
    , mpClassElement(NULL)
    , mpMainConnection(NULL)
    , mpRouteElement(NULL)
    , mpActionClassSuspend(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
{
}

CAmClassActionSuspendTest::~CAmClassActionSuspendTest()
{
}

void CAmClassActionSuspendTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpControlReceive              = mpMockControlReceiveInterface;

    // This will check whether controller is initialized or not
    ASSERT_THAT(mpControlReceive, NotNull()) << " Controller Not Initialized";

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _))
    .WillRepeatedly(DoAll(SetArgReferee<1 >(gc_ut_MainConnection_s(CS_CONNECTED)), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(23, _))
    .WillRepeatedly(DoAll(SetArgReferee<1>(sourceInfo), Return(E_OK)));

    // Initialize Sink Element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpSinkElement = CAmSinkFactory::createElement(sinkInfo, mpControlReceive);
    ASSERT_THAT(mpSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpSinkElement->getID());

    // Initialize Source Element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(23), Return(E_OK)));
    mpSourceElement = CAmSourceFactory::createElement(sourceInfo, mpControlReceive);

    ASSERT_THAT(mpSourceElement, NotNull()) << " Source Element Is not Created";
    sourceIDList.push_back(mpSourceElement->getID());
    RoutingElement.connectionFormat = CF_GENIVI_STEREO;
    RoutingElement.domainID         = mpSinkElement->getDomainID();
    RoutingElement.sinkID           = mpSinkElement->getID();
    RoutingElement.sourceID         = mpSourceElement->getID();
    GCRoute.sinkID                  = mpSinkElement->getID();
    GCRoute.sourceID                = mpSourceElement->getID();
    GCRoute.name                    = mpSourceElement->getName() + ":" + mpSinkElement->getName();
    GCRoute.route.push_back(RoutingElement);
    mRouteList.push_back(GCRoute);

    mMainConnectionData.mainConnectionID = 0;
    mMainConnectionData.sinkID           = mpSinkElement->getID();
    mMainConnectionData.sourceID         = mpSourceElement->getID();
    mMainConnectionData.delay            = 0;
    mMainConnectionData.connectionState  = CS_CONNECTED;
    classPropertyData.classProperty      = CP_GENIVI_SINK_TYPE;
    classPropertyData.value              = 1;

    sinkClass.sinkClassID = 73;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);
    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 74;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    // Class Element
    std::vector < std::vector<gc_TopologyElement_s > > listTopologies;
    std::vector<gc_TopologyElement_s >                 GClistTopolologyElement;
    // source Element belongs to class
    mGcClassConnection.name     = "AnyClass1";
    mGcClassConnection.type     = C_CAPTURE;
    mGcClassConnection.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 5;
    std::vector<am_SystemProperty_s > listSystemProperties;
    listSystemProperties.push_back(systemproperty);
    // System element
    gc_System_s systemConfiguration;
    systemConfiguration.name = "System";
    std::shared_ptr<CAmSystemElement > pSystem;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(
        Return(E_OK));
    pSystem = CAmSystemFactory::createElement(systemConfiguration, mpControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getListSystemProperties(_)).WillRepeatedly(
        DoAll(SetArgReferee<0 >(listSystemProperties), Return(E_OK)));

    // creating the class connection element
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(
        DoAll(SetArgReferee<1 >(73), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(
        DoAll(SetArgReferee<0 >(74), Return(E_OK)));
    mpClassElement = CAmClassFactory::createElement(mGcClassConnection, mpControlReceive);

    ASSERT_THAT(mpClassElement, NotNull()) << " Class Element Is not Created";

    // creating the routing element
    gc_RoutingElement_s gcRoutingElement;
    gcRoutingElement.name             = mpSourceElement->getName() + ":" + mpSinkElement->getName();
    gcRoutingElement.sourceID         = mpSourceElement->getID();
    gcRoutingElement.sinkID           = mpSinkElement->getID();
    gcRoutingElement.domainID         = 2;
    gcRoutingElement.connectionFormat = CF_GENIVI_STEREO;

    mpRouteElement = CAmRouteFactory::createElement(gcRoutingElement, mpControlReceive);
    ASSERT_THAT(mpRouteElement, NotNull()) << " Route Element Is not Created";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(_, _))
    .WillOnce(DoAll(SetArgReferee<1>((am_mainConnectionID_t)44), Return(E_OK)));

    mpMainConnection = CAmMainConnectionFactory::createElement(GCRoute, mpControlReceive);
    ASSERT_THAT(mpMainConnection, NotNull()) << " MainConnection Element Is not Created";

    mpMainConnection->attach(mpSourceElement);
    mpMainConnection->attach(mpSinkElement);
    mpClassElement->attach(mpMainConnection);

    mpRouteElement->setID(44);
    mpRouteElement->setState(CS_CONNECTED);

    std::vector<std::shared_ptr<CAmMainConnectionElement > > listMainConnections;
    listMainConnections.push_back(mpMainConnection);
    mpActionClassSuspend = new CAmActionSuspend();
    ASSERT_THAT(mpActionClassSuspend, NotNull()) << " Class Action SetVolume Is not Created";

    CAmActionParam<std::string > classNameParam(mpClassElement->getName());
    mpActionClassSuspend->setParam(ACTION_PARAM_CLASS_NAME, &classNameParam);
}

void CAmClassActionSuspendTest::TearDown()
{
    CAmRouteFactory::destroyElement();
    CAmMainConnectionFactory::destroyElement();
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    CAmClassFactory::destroyElement();
    CAmSystemFactory::destroyElement();

    if (mpActionClassSuspend != NULL)
    {
        delete (mpActionClassSuspend);
    }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief : Test to verify the _execute function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Source, sink , route,system & also class Elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc.
 *        and Main connection element is also created and push the configuration in db,
 *        sink, source element is attached with main connect and then main connection is attached to class element.
 *        and connection set is also set and also push the main connection element in list of main connections
 *        and CAmActionClassSuspend is initialized
 *
 * @test : verify the "_execute" function when Sink element and source element is set along with other configuration details
 *         and set the behavior of getMainConnectionInfoDB as E_OK with main connection state as CS_Connected & set the behavior for
 *          getSourceInfoDB function to make sure that it will return E_OK and set source state as SS_PAUSED then will check
 *         whether _execute function will get the main connection details from the sink/source element or not,
 *         from the _getTargetConnection function and then get main connection element.
 *         finally check this _execute function  creates the child action object if connection is found which need to be suspended.
 *         It appends the child to base class for execution or not , without any Gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmClassActionSuspendTest, _executePositiveCase)
{
    mSourceNameParam.setParam(mpSourceElement->getName());
    mSinkNameParam.setParam(mpSinkElement->getName());

    mpActionClassSuspend->setParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    mpActionClassSuspend->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _)).WillRepeatedly(
        DoAll(SetArgReferee<1 >(gc_ut_MainConnection_s(CS_CONNECTED)), Return(E_OK)));
    EXPECT_EQ(mpActionClassSuspend->execute(), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
}

/**
 * @brief : Test to verify the _execute function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Source, sink , route,system & also class Elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc.
 *        and Main connection element is also created and push the configuration in db,
 *        sink, source element is attached with main connect and then main connection is attached to class element.
 *        and connection set is also set and also push the main connection element in list of main connections
 *        and CAmActionClassSuspend is initialized
 *
 * @test : verify the "_execute" function when different Sink element and source element is set along with other configuration details
 *         and set the behavior of getMainConnectionInfoDB as E_OK with main connection state as CS_Connected then will check
 *         whether _execute function will get the main connection details from the sink/source element or not,
 *         from the _getTargetConnection function and then get main connection element.
 *         finally check this _execute function creates the child action object if connection is found which need to be suspended.
 *         It appends the child to base class for execution or not , without any Gmock error.
 *
 * @result : "Pass" when execute function return "E_OK" without any Gmock error message
 */
TEST_F(CAmClassActionSuspendTest, _executeNegativeCase)
{
    mSourceNameParam.setParam("Media");
    mSinkNameParam.setParam("Speaker");

    mpActionClassSuspend->setParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    mpActionClassSuspend->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);

    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(44, _)).WillRepeatedly(
        DoAll(SetArgReferee<1 >(gc_ut_MainConnection_s(CS_CONNECTED)), Return(E_OK)));
    EXPECT_EQ(mpActionClassSuspend->execute(), E_OK);
}

int main(int argc, char * *argv)
{
    // initialize logging environment
    am::CAmLogWrapper::instantiateOnce("UTST", "Unit test for generic controller"
            , LS_ON, LOG_SERVICE_STDOUT);
    LOG_FN_CHANGE_LEVEL(LL_WARN);

    // redirect configuration path
    gc_utest::ConfigDocument config(gc_utest::ConfigDocument::Default);

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
