/*******************************************************************************
 *  \copyright (c) 2018 - 2020 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \authors Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018\n
 *           Kapildev Patel, Yogesh Sharma <kpatel@jp.adit-jv.com> 2018\n
 *           Martin Koch     <martin.koch@ese.de> 2020
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

#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmElement.h"
#include "CAmSourceElement.h"
#include "CAmSinkElement.h"
#include "CAmDomainElement.h"
#include "CAmClassElement.h"
#include "CAmControllerPlugin.h"
#include "CAmHandleStore.h"
#include "CAmRouteElement.h"
#include "CAmMainConnectionElement.h"
#include "IAmPolicySend.h"
#include "CAmSystemElement.h"
#include "CAmRootAction.h"
#include "CAmActionCommand.h"
//#include "CAmActionContainer.h"
#include "MockIAmControlReceive.h"
#include "MockIAmPolicySend.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "TestObserver.h"
#include "CAmControllerPluginTest.h"
#include "CAmTestConfigurations.h"

#include <signal.h>

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;


/**
 * @class  BlockingAction
 * @brief  Helper class to stop the policy engine from processing triggers.
 *
 *         Append a (heap-)object of this class to the global CAmRootAction
 *         instance. Thus any call to the CAmControllerPlugin::iterateActions()
 *         will first pick this one and let other triggers in the queue untouched.
 *
 *         Release the blockage through:
 *         CAmHandleStore::instance().notifyAsyncResult(pBlocking->handle, E_OK);
 */
class BlockingAction  : public CAmActionCommand
{
    public:
        BlockingAction()
            : CAmActionCommand("GC Unit Test Blocking Action")
            , handle({H_UNKNOWN, 333})
        {
        }

        int _execute(void) override
        {
            CAmHandleStore::instance().saveHandle(handle, this);
            return E_WAIT_FOR_CHILD_COMPLETION;
        }

        int _update(const int result) override
        {
            // unregister the observer
            CAmHandleStore::instance().clearHandle(handle);

            return E_OK;
        }

        const am_Handle_s handle;
};

/***************************************************************************//**
 *@Class : CAmControllerPluginTest
 *@brief : This class is used to test the CAmControllerPlugin class functionality.
 */
CAmControllerPluginTest::CAmControllerPluginTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(nullptr)
    , mpCAmSourceElement(nullptr)
    , mpCAmRouteElement(NULL)
    , mpMockIAmPolicySend(NULL)
    , mpPlugin(NULL)
    , pSocketHandler(NULL)
    , pSystem(nullptr)
    , mpCAmDomainElement(nullptr)
    , testObserver(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , domainInfo("AnyDomain1")
{
#define GC_UNIT_TEST
}

CAmControllerPluginTest::~CAmControllerPluginTest()
{

}

void CAmControllerPluginTest::InitializeCommonStruct()
{
    sink.name                   = "AnySink1";
    source.name                 = "AnySource1";
    type                        = SYP_GLOBAL_LOG_THRESHOLD;
    value                       = 5;
    domain.name                 = "AnyDomain1";
    domain.domainID             = 4;
    state                       = DS_CONTROLLED;
    domainInfo.busname          = "AnyBus";
    domainInfo.nodename         = "AnyNode";
    domainInfo.early            = true;
    domainInfo.complete         = false;
    domainInfo.state            = DS_UNKNOWN;
    domainInfo.registrationType = REG_CONTROLLER;
    listDomains.push_back(domain);
}

void CAmControllerPluginTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " Controller Not Initialized";
    InitializeCommonStruct();
    EXPECT_CALL(*mpMockControlReceiveInterface,
        enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (22), Return(E_OK)));
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    source.name = "AnySource1";
    EXPECT_CALL(*mpMockControlReceiveInterface,
        enterSourceDB(IsSourceNamePresent(sourceInfo), _)).WillOnce(
        DoAll(SetArgReferee < 1 > (23), Return(E_OK)));
    mpCAmSourceElement = CAmSourceFactory::createElement(sourceInfo,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmSourceElement, NotNull())
        << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement->getID());
    sourceID         = mpCAmSourceElement->getID();
    gcRoute.sinkID   = mpCAmSinkElement->getID();
    gcRoute.sourceID = mpCAmSourceElement->getID();
    gcRoute.name     = mpCAmSourceElement->getName() + ":" + mpCAmSinkElement->getName();
    gcRoute.route.push_back(routingElement);
    listRoutes.push_back(gcRoute);

    mainConnectionID = 0;
    // mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID          = mpCAmSinkElement->getID();
    mainConnectionData.sourceID        = mpCAmSourceElement->getID();
    mainConnectionData.delay           = 0;
    mainConnectionData.connectionState = CS_CONNECTED;
    gcRoutingElement.name              = mpCAmSourceElement->getName() + mpCAmSinkElement->getName();
    gcRoutingElement.sourceID          = mpCAmSourceElement->getID();
    gcRoutingElement.sinkID            = mpCAmSinkElement->getID();
    gcRoutingElement.domainID          = 2;
    gcRoutingElement.connectionFormat  = CF_GENIVI_STEREO;
    mpCAmRouteElement                  = new CAmRouteElement(gcRoutingElement,
            mpCAmControlReceive);
    ASSERT_THAT(mpCAmRouteElement, NotNull()) << " Route Element Is not Created";
    TCLAP::CmdLine *cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager1 wishes you a nice day!", ' ', "7.5", true));
    mpMockIAmPolicySend = new MockIAmPolicySend();
    ASSERT_THAT(mpMockIAmPolicySend, NotNull()) << "MockIAmPolicySend Is not Created";

    mpPlugin= new CAmControllerPlugin();
    pSocketHandler   = new CAmSocketHandler();
    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(IsDomainNamePresent(domain), _)).WillOnce(DoAll(SetArgReferee<1>(4), Return(E_OK)));
    mpCAmDomainElement = CAmDomainFactory::createElement(domainInfo, mpCAmControlReceive);
    mpCAmDomainElement->setID(4);
    domainID     = mpCAmDomainElement->getID();
    testObserver = new TestObserver();
}

void CAmControllerPluginTest::TearDown()
{
    CAmSourceFactory::destroyElement();
    CAmSinkFactory::destroyElement();
    CAmSystemFactory::destroyElement();

    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
    }

    CAmDomainFactory::destroyElement();

    if (mpCAmRouteElement != NULL)
    {
        delete (mpCAmRouteElement);
    }

    CAmCommandLineSingleton::deleteInstance();
#undef GC_UNIT_TEST
}

/***************************************************************************//**
 * @brief : Test to verify the startupController functions for Positive scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "startupController" function when sink,source,class elements are initialized with default configuration data,
 *         and set the behavior of enterSinkClassDB,enterSourceClassDB, enterSystemPropertiesListDB, getSocketHandler functions
 *         to return the E_OK and pass the mpMockControlReceiveInterface to startupController function
 *         then will check whether startupController function will Instantiates timer event,Control Receive, policy send,
 *         policy receive,class element and system element, CAmPersistenceWrapper or not . without any gmock error.
 *
 * @result : "Pass" when startupController function return "E_OK" without any Gmock error message
 */
TEST_F(CAmControllerPluginTest, startupControllerPositive)
{
    sink.name                       = "AnySink1";
    source.name                     = "AnySource1";
    classPropertyData.classProperty = CP_GENIVI_SINK_TYPE;
    classPropertyData.value         = 1;
    sinkClass.sinkClassID           = 72;
    sinkClass.name                  = "AnySinkClass1";
    sinkClass.listClassProperties.push_back(classPropertyData);

    classPropertyData.classProperty = CP_GENIVI_SOURCE_TYPE;
    sourceClass.sourceClassID       = 73;
    sourceClass.name                = "AnySourceClass1";
    sourceClass.listClassProperties.push_back(classPropertyData);

    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;

    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(IsSinkClassOk(sinkClass), _))
    .Times(3).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK))).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK))).WillOnce(DoAll(SetArgReferee<1>(72), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, IsSourceClassOk(sourceClass))).Times(3)
    .WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)))
    .WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)))
    .WillOnce(DoAll(SetArgReferee<0>(73), Return(E_OK)));

    // SystemProperty creating
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;
    listSystemProperties.push_back(systemproperty);
    systemConfiguration.name = "System1";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    EXPECT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

}

/**
 * @brief : Test to verify the startupController functions for Negative scenario
 *
 * @pre : Mock the IAmControlReceive,IAmPolicySend classes and  initialized the CAmControlReceive, CAmControllerPlugin,CAmSocketHandler
 *        CAmDomainElement,CAmRouteElement classes,
 *        Source, sink ,route,domain elements are created with required input structure parameters
 *        like name,domainID,bus name,state, node name,registration type etc. source,sink element details is enter in db for behavior set.
 *        and initialize the structure like am_MainConnection_s,gc_RoutingElement_s,gc_System_s,gc_TopologyElement_s etc.
 *
 * @test : verify the "startupController" function when sink,source,class elements are not initialized with default configuration data,
 *         and not set the behavior of enterSinkClassDB,enterSourceClassDB, enterSystemPropertiesListDB, getSocketHandler functions
 *         to return the E_OK and pass the mpMockControlReceiveInterface to startupController function
 *         then will check whether startupController function will Instantiates timer event,Control Receive, policy send,
 *         policy receive,class element and system element, CAmPersistenceWrapper or not . without any gmock error.
 *
 * @result : "Pass" when startupController function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmControllerPluginTest, startupControllerNegative)
{
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_NOT_POSSIBLE)));
    EXPECT_EQ(E_NOT_POSSIBLE, mpPlugin->startupController(mpMockControlReceiveInterface));

}

/**
 * @brief  Verify the registerEarlyMainConnection() function with a
 *         two-segment main connection, presumably pre-registered from
 *         routing side and thus positively returned from the database.
 */
TEST_F(CAmControllerPluginTest, EarlyConnection)
{
    // start with default configurationj
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(_))
        .WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));

    ASSERT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // expectation
    EXPECT_CALL(*mpMockControlReceiveInterface, getSourceInfoDB(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(source), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSinkInfoDB(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(sink), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(1), Return(E_ALREADY_EXISTS)));
    am_MainConnection_s refMainConnection;
    refMainConnection.mainConnectionID = 1;
    refMainConnection.sourceID         = 23;
    refMainConnection.sinkID           = 22;
    refMainConnection.connectionState  = CS_CONNECTED;
    refMainConnection.listConnectionID.push_back(1);
    refMainConnection.listConnectionID.push_back(2);
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(_, _))
        .WillRepeatedly(Invoke([&refMainConnection](const am_mainConnectionID_t id, am_MainConnection_s& data)
                { data = refMainConnection; return E_OK; }));
    vector<am_Connection_s> listSegments;
    listSegments.resize(2);
    listSegments[0].connectionID     = 1;
    listSegments[0].sourceID         = 23;
    listSegments[0].sinkID           = 102;
    listSegments[0].connectionFormat = CF_GENIVI_STEREO;
    listSegments[0].delay            = 0;
    listSegments[0].connectionID     = 2;
    listSegments[0].sourceID         = 103;
    listSegments[0].sinkID           = 22;
    listSegments[0].connectionFormat = CF_GENIVI_STEREO;
    listSegments[0].delay            = 0;
    EXPECT_CALL(*mpMockControlReceiveInterface, getConnectionInfoDB(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(listSegments[0]), Return(E_OK)))
        .WillOnce(DoAll(SetArgReferee<1>(listSegments[1]), Return(E_OK)));

    // test case
    am_domainID_t domainID = 5;
    am_MainConnection_s mainConnectionData = refMainConnection;
    mainConnectionData.mainConnectionID = 0;
    am_Route_s          route;
    route.sourceID = 23;
    route.sinkID   = 22;
    route.route.push_back({23, 102});
    route.route.push_back({1033, 22});

    // block policy engine from processing trigger under test
    auto *pBlocking = new BlockingAction;
    CAmRootAction::getInstance()->append(pBlocking);

    EXPECT_EQ(E_OK, mpPlugin->hookSystemRegisterEarlyMainConnection(domain.domainID, mainConnectionData, route));

    // validate that the proper trigger has been created
    gc_Trigger_e triggerType = TRIGGER_UNKNOWN;
    gc_TriggerElement_s *pTrigger = CAmTriggerQueue::getInstance()->dequeue(triggerType);
    ASSERT_NE(nullptr, pTrigger);
    EXPECT_EQ(SYSTEM_REGISTER_EARLY_CONNECTION, triggerType);
    auto *pRegisterTrigger = static_cast<gc_RegisterElementTrigger_s*>(pTrigger);
    EXPECT_EQ(E_OK, pRegisterTrigger->RegisterationStatus);
    EXPECT_STREQ("AnySource1:AnySink1", pRegisterTrigger->elementName.c_str());

    // release temporary blockage
    CAmHandleStore::instance().notifyAsyncResult(pBlocking->handle, E_OK);

    // cleanup main connection
    EXPECT_CALL(*mpMockControlReceiveInterface, removeMainConnectionDB(_))
        .WillOnce(Return(E_OK));
    CAmMainConnectionFactory::destroyElement(1);
}

/**
 * @brief  Verify the controllerRundown() function without remaining active connections
 *
 */
TEST_F(CAmControllerPluginTest, RundownSimple)
{
    // start with default configurationj
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(_))
        .WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_))
        .WillOnce(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));

    ASSERT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // expectation
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_))
        .WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, setRoutingRundown());
    EXPECT_CALL(*mpMockControlReceiveInterface, setCommandRundown());
    EXPECT_CALL(*mpMockControlReceiveInterface, confirmControllerRundown(E_OK));

    // now rundown
    mpPlugin->setControllerRundown(SIGTERM);
}

/**
 * @brief  Verify the controllerRundown() function with one remaining active connection
 *         and two domains marked as DS_INDEPENDENT_RUNDOW.
 *
 * @test   The Rundown sequence will offer this connection to both domains sequentially,
 *         whereof the first rejects with E_NOT_POSSIBLE, but the second accepts.
 *         Anyway, the Rundown sequence shall be completed asynchronously
 *         until confirmControllerRundown()
 */
TEST_F(CAmControllerPluginTest, RundownTransfer)
{
    vector<am_MainConnection_s> listMainConnections;
    auto getMCInfo = [this, &listMainConnections] (const am_mainConnectionID_t id, am_MainConnection_s& data)
        {
            for (const auto &mc : listMainConnections)
                if (mc.mainConnectionID == id)
                {
                    data = mc;
                    return E_OK;
                }
            return E_NON_EXISTENT;
        };

    // prepare response to informational requests
    EXPECT_CALL(*mpMockControlReceiveInterface, getSocketHandler(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(pSocketHandler), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, getListDomains(_))
        .WillRepeatedly(Invoke([this](vector<am_Domain_s> &outList) -> am_Error_e
                {  outList = listDomains;  return E_OK;  }));  // mock database with our list
    EXPECT_CALL(*mpMockControlReceiveInterface, getListMainConnections(_))
        .WillRepeatedly(Invoke([this, &listMainConnections](vector<am_MainConnection_s> &outList) -> am_Error_e
                {  outList = listMainConnections;  return E_OK;  }));  // mock database with our list
    EXPECT_CALL(*mpMockControlReceiveInterface, getMainConnectionInfoDB(_, _))
        .WillRepeatedly(Invoke(getMCInfo));  // mock database with our list

    // startup with default configuration
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(_))
        .WillOnce(Return(E_OK));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceClassDB(_, _))
       .WillRepeatedly(DoAll(SetArgReferee<0>(73), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkClassDB(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(72), Return(E_OK)));
    ASSERT_EQ(E_OK, mpPlugin->startupController(mpMockControlReceiveInterface));

    // cleanup domain tracking and add 3 domains, whereof 2 are capable for handover
    CAmDomainFactory::destroyElement();
    listDomains.clear();
    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(_, _))
        .WillOnce(DoAll(Assign(&domainInfo.domainID, 4), SetArgReferee<1>(4), Return(E_OK)));
    domainInfo.early = false;
    domainInfo.state = DS_INDEPENDENT_RUNDOWN;
    CAmDomainFactory::createElement(static_cast<const gc_Domain_s&>(domainInfo), mpMockControlReceiveInterface);
    listDomains.push_back(static_cast<am_Domain_s>(domainInfo));
    gc_Domain_s dspDomain;
    dspDomain.domainID         = 0;
    dspDomain.name             = "VirtualDSP";
    dspDomain.busname          = "local";
    dspDomain.nodename         = "Cpu";
    dspDomain.early            = false;
    dspDomain.complete         = true;
    dspDomain.state            = DS_CONTROLLED;
    dspDomain.registrationType = REG_ROUTER;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(_, _))
        .WillOnce(DoAll(Assign(&dspDomain.domainID, 101), SetArgReferee<1>(101), Return(E_OK)));
    CAmDomainFactory::createElement(dspDomain, mpMockControlReceiveInterface);
    listDomains.push_back((am_Domain_s)dspDomain);
    gc_Domain_s appDomain;
    appDomain.domainID         = 0;
    appDomain.name             = "Applications";
    appDomain.busname          = "busName1";
    appDomain.nodename         = "Cpu";
    appDomain.early            = false;
    appDomain.complete         = true;
    appDomain.state            = DS_INDEPENDENT_RUNDOWN;
    appDomain.registrationType = REG_ROUTER;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterDomainDB(_, _))
        .WillOnce(DoAll(Assign(&appDomain.domainID, 104), SetArgReferee<1>(104), Return(E_OK)));
    CAmDomainFactory::createElement(appDomain, mpMockControlReceiveInterface);
    listDomains.push_back((am_Domain_s)appDomain);

    // cleanup mainConnection tracking and add 1 element
    CAmMainConnectionFactory::destroyElement();
    am_MainConnection_s mc;
    mc.mainConnectionID = 0;
    mc.sourceID         = sourceID;
    mc.sinkID           = sinkID;
    mc.connectionState  = CS_CONNECTED;
    mc.delay            = 0;
    mc.listConnectionID = {23, 22};
    am_RoutingElement_s re;
    re.sourceID         = 23;
    re.sinkID           = 22;
    re.domainID         = 4;
    re.connectionFormat = CF_GENIVI_STEREO;
    gc_Route_s   rt;
    rt.name     = "AnySource1:AnySink1";
    rt.sourceID = 23;
    rt.sinkID   = 22;
    rt.route.push_back(re);
    EXPECT_CALL(*mpMockControlReceiveInterface, enterMainConnectionDB(_, _))
        .WillOnce(DoAll(Assign(&mc.mainConnectionID, 5), SetArgReferee<1>(5), Return(E_OK)));
    CAmMainConnectionFactory::createElement(rt, mpMockControlReceiveInterface);
    listMainConnections.push_back(mc);

    // expectations
    am_Handle_s handle1({H_TRANSFERCONNECTION, 177});
    am_Handle_s handle2({H_TRANSFERCONNECTION, 180});
    EXPECT_CALL(*mpMockControlReceiveInterface, transferConnection(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(handle1), Return(E_OK)))
        .WillOnce(DoAll(SetArgReferee<0>(handle2), Return(E_OK)));
    EXPECT_CALL(*mpMockControlReceiveInterface, setRoutingRundown());
    EXPECT_CALL(*mpMockControlReceiveInterface, setCommandRundown());
    EXPECT_CALL(*mpMockControlReceiveInterface, confirmControllerRundown(E_OK));

    // now rundown
    mpPlugin->setControllerRundown(SIGTERM);

    // first domain rejects
    mpPlugin->cbAckTransferConnection(handle1, E_NOT_POSSIBLE);

    // second domain takes over
    mpPlugin->cbAckTransferConnection(handle2, E_OK);
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
