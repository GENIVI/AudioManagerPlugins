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
#include "CAmControllerPlugin.h"
#include "CAmActionConnect.h"
#include "CAmMainConnectionElement.h"
#include "CAmRouteElement.h"
#include "CAmSystemElement.h"
#include "CAmHandleStore.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmClassActionConnectTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmClassActionConnectTest
 *@brief : This class is used to test the CAmClassActionConnect functionality.
 */
CAmClassActionConnectTest::CAmClassActionConnectTest()
    : mpMockControlReceiveInterface(NULL)
    , mpControlReceive(NULL)
    , mpSinkElement(nullptr)
    , mpSourceElement(nullptr)
    , mpClassElement(nullptr)
    , mpClassActionConnect(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , pSystem(nullptr)
{
}

CAmClassActionConnectTest::~CAmClassActionConnectTest()
{
}

void CAmClassActionConnectTest::SetUp()
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

    GCRoute.sinkID   = mpSinkElement->getID();
    GCRoute.sourceID = mpSourceElement->getID();
    GCRoute.name     = mpSourceElement->getName() + ":" + mpSinkElement->getName();
    GCRoute.route.push_back(RoutingElement);
    mRouteList.push_back(GCRoute);

    mMainConnectionData.mainConnectionID = 0;
    mMainConnectionData.sinkID           = mpSinkElement->getID();
    mMainConnectionData.sourceID         = mpSourceElement->getID();
    mMainConnectionData.delay            = 0;
    mMainConnectionData.connectionState  = CS_UNKNOWN;

    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;

    sinkClass.sinkClassID = 73;
    sinkClass.name        = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;

    sourceClass.sourceClassID = 74;
    sourceClass.name          = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);
    classConnection.name     = "AnyClass1";
    classConnection.type     = C_CAPTURE;
    classConnection.priority = 1;
    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 5;
    listSystemProperties.push_back(systemproperty);

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _)).WillOnce(
        DoAll(SetArgReferee<1 >(73), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).WillOnce(
        DoAll(SetArgReferee<0 >(74), Return(E_OK)));
    mpClassElement = CAmClassFactory::createElement(classConnection, mpControlReceive);

    // Initialize System Element
    systemConfiguration.name = "SystemProp";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem = CAmSystemFactory::createElement(systemConfiguration, mpControlReceive);

    mpClassElement = CAmClassFactory::createElement(classConnection, mpControlReceive);
    CAmActionParam<std::string > classNameParam;
    classNameParam.setParam(classConnection.name);

    ASSERT_THAT(mpClassElement, NotNull()) << " Class Element Is not Created";

    mpClassElement->attach(mpSourceElement);
    mpClassElement->attach(mpSinkElement);

    mpClassActionConnect = new CAmActionConnect();
    ASSERT_THAT(mpClassActionConnect, NotNull()) << " Class Action Connect Is not Created";
    mpClassActionConnect->setParam(ACTION_PARAM_CLASS_NAME, &classNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_SOURCE_NAME, &mSourceNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_SINK_NAME, &mSinkNameParam);
    mpClassActionConnect->setParam(ACTION_PARAM_ORDER, &mOrderParam);
    mpClassActionConnect->setParam(ACTION_PARAM_CONNECTION_FORMAT, &mConnectionFormatParam);

}

void CAmClassActionConnectTest::TearDown()
{
    sourceIDList.clear();
    CAmSourceFactory::destroyElement();
    sinkIDList.clear();
    CAmSinkFactory::destroyElement();
    CAmClassFactory::destroyElement();
    CAmSystemFactory::destroyElement();

    if (mpClassActionConnect != NULL)
    {
        delete (mpClassActionConnect);
    }

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }
}

/**
 * @brief  :  Test to verify the _execute_updatedCaseCS_CONNECTED function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source, routing Elements are created with required input structure parameters like name,domainID,class name,
 *        registration type,volume etc. Also system and class element is created with required input configurations and source,sink elements
 *        are attached.CAmClassActionConnect class is initialized with few setParameters like source,sink name,order and connection format.
 *
 * @test : verify the "execute" function when main connection info details is inserted in the DB and get source info and main connection info
 *         from initial set value and then create the connection using source, sink details and set the source state as CS_CONNECTED
 *         and change the main connection db again then check whether execute function will get the main connection element db or not , then
 *         create the main connection action connect or not and then append that connect action or not without any gmock error.
 *
 * @result : "Pass" when execute function return the expected output "E_OK".
 */
TEST_F(CAmClassActionConnectTest, _execute_updatedCaseCS_CONNECTED)
{
    EXPECT_EQ(mpClassActionConnect->execute(), E_OK);
    EXPECT_EQ(mpClassActionConnect->update(CS_CONNECTED), E_OK);
}

/**
 * @brief  :  Test to verify the _execute_UpdatedCaseCS_SUSPENDED function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source, routing Elements are created with required input structure parameters like name,domainID,class name,
 *        registration type,volume etc. Also system and class element is created with required input configurations and source,sink elements
 *        are attached.CAmClassActionConnect class is initialized with few setParameters like source,sink name,order and connection format.
 *
 * @test : verify the "execute" function when limit volume param and ramp type param  etc are configured and main connection info details is inserted in the DB and get source info and main connection info
 *         from initial set value and then create the connection using source, sink details and set the source state as CS_SUSPENDED
 *         and change the main connection db again then check whether execute function will get the main connection element db or not , then
 *         create the main connection action connect or not and then append that connect action or not without any gmock error.
 *
 * @result : "Pass" when execute function return the expected output "E_OK".
 */
TEST_F(CAmClassActionConnectTest, _execute_UpdatedCaseCS_SUSPENDED)
{
    mLimitVolumeParam = -1920;
    mRampTypeParam    = RAMP_GENIVI_DIRECT;
    mRampTimeParam    = 200;
    EXPECT_EQ(mpClassActionConnect->execute(), E_OK);
    EXPECT_EQ(mpClassActionConnect->update(CS_SUSPENDED), E_OK);
    CAmHandleStore::instance().notifyAsyncResult(mHandle, E_OK);
}

/**
 * @brief  :  Test to verify the _Undo function Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        Sink, Source, routing Elements are created with required input structure parameters like name,domainID,class name,
 *        registration type,volume etc. Also system and class element is created with required input configurations and source,sink elements
 *        are attached.CAmClassActionConnect class is initialized with few setParameters like source,sink name,order and connection format.
 *
 * @test : verify the "_Undo" function when limit volume param and ramp type param  etc are configured and
 *         CAmControlsend and  CAmSocketHandler, CAmTimerEvent are initialized with required inputs data.
 *         And set the undo flag as true and append the action in the action container,
 *         Also main connection info details is inserted in the DB and get source info and change the main connection db  with different state
 *         change like CS_UNKNOWN, CS_CONNECTING,CS_DISCONNECTING etc. and also set the sink volume using the setSinkVolume function call
 *         then check whether _Undo function will dispose Main Connection i.e to destroy the main connection object or not for AS_UNDO_COMPLETE state
 *         and also  update the updateMainConnectionQueue or not without any gmock error.
 *
 * @result : "Pass" when _Undo function return the expected output "E_OK".
 */
TEST_F(CAmClassActionConnectTest, _Undo)
{
    TCLAP::CmdLine *cmd(
        CAmCommandLineSingleton::instanciateOnce("AudioManager!", ' ', "7.5", true));

    CAmControllerPlugin *pPlugin        = new CAmControllerPlugin();
    CAmSocketHandler    *pSocketHandler = new CAmSocketHandler();

    CAmTimerEvent *mpTimerEvent = CAmTimerEvent::getInstance();
    mpTimerEvent->setSocketHandle(pSocketHandler, pPlugin);

    CAmActionContainer pCAmActionContainer;
    mpClassActionConnect->setUndoRequried(true);
    pCAmActionContainer.append(mpClassActionConnect);
    pCAmActionContainer.setUndoRequried(true);

    mLimitVolumeParam = -1920;
    mRampTypeParam    = RAMP_GENIVI_DIRECT;
    mRampTimeParam    = 200;
    EXPECT_EQ(mpClassActionConnect->execute(), E_OK);
    EXPECT_EQ(mpClassActionConnect->undo(), E_OK);

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
