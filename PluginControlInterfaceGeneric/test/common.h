/*
 * common.h
 *
 *  Created on: 16-Dec-2016
 *      Author: dipesh
 */

#ifndef COMMON_H_
# define COMMON_H_
// #include "CAmGcCommonFunctions.h"

# include "gtest/gtest.h"
# include "gmock/gmock.h"

/*namespace am
 * {
 * namespace gc{
 * class common :public :: CAmClassActionSuspendTest
 * {
 * public:
 *  common();
 *  ~common();
 *  void createMainConnection()
 *  {
 *
 *      mainConnectionData.sinkID=pSinkElement->getID();
 *          mainConnectionData.sourceID=pSourceElement->getID();
 *          mainConnectionData.delay=0;
 *          mainConnectionData.connectionState=CS_CONNECTED;
 *
 *          //gc_Route_s route1;
 *          am_RoutingElement_s routingElement;
 *          route1.sinkID = pSinkElement->getID();
 *          route1.sourceID = pSourceElement->getID();
 *
 *
 *          route1.name = pSourceElement->getName() + ":" + pSinkElement->getName();
 *          routingElement.connectionFormat = CF_GENIVI_STEREO;
 *          routingElement.domainID = pSinkElement->getDomainID();
 *          routingElement.sinkID = pSinkElement->getID();
 *          routingElement.sourceID = pSourceElement->getID();
 *          route1.route.push_back(routingElement);
 *
 *          //am_MainConnection_s mainConnectionData;
 *          mainConnectionData.mainConnectionID=0;
 *          mainConnectionData.sinkID=pSinkElement->getID();
 *          mainConnectionData.sourceID=pSourceElement->getID();
 *          mainConnectionData.delay=0;
 *          mainConnectionData.connectionState=CS_CONNECTED;
 *
 *          MockIAmControlReceive *controlReceiveInterfaceMainConnect= new MockIAmControlReceive();
 *          CAmControlReceive* mpControlReceiveMainConnect = new gc::CAmControlReceive(controlReceiveInterfaceMainConnect);
 *
 *          //CAmMainConnectionElement* pMainConnection = NULL;
 *
 *          EXPECT_CALL(*controlReceiveInterfaceMainConnect,enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData),_)).WillOnce(DoAll(SetArgReferee<1>(44), Return(E_OK)));
 *          pMainConnection = CAmMainConnectionFactory::createElement(route1, mpControlReceiveMainConnect);
 *  }
 *
 *
 *
 * };
 * }
 * }*/

/*void CAmClassActionSuspendTest::createMainConnection()
 * {
 *  //mainConnectionData.mainConnectionID=0;
 *  mainConnectionData.sinkID=pSinkElement->getID();
 *  mainConnectionData.sourceID=pSourceElement->getID();
 *  mainConnectionData.delay=0;
 *  mainConnectionData.connectionState=CS_CONNECTED;
 *
 *  //gc_Route_s route1;
 *  am_RoutingElement_s routingElement;
 *  route1.sinkID = pSinkElement->getID();
 *  route1.sourceID = pSourceElement->getID();
 *
 *
 *  route1.name = pSourceElement->getName() + ":" + pSinkElement->getName();
 *  routingElement.connectionFormat = CF_GENIVI_STEREO;
 *  routingElement.domainID = pSinkElement->getDomainID();
 *  routingElement.sinkID = pSinkElement->getID();
 *  routingElement.sourceID = pSourceElement->getID();
 *  route1.route.push_back(routingElement);
 *
 *  //am_MainConnection_s mainConnectionData;
 *  mainConnectionData.mainConnectionID=0;
 *  mainConnectionData.sinkID=pSinkElement->getID();
 *  mainConnectionData.sourceID=pSourceElement->getID();
 *  mainConnectionData.delay=0;
 *  mainConnectionData.connectionState=CS_CONNECTED;
 *
 *  MockIAmControlReceive *controlReceiveInterfaceMainConnect= new MockIAmControlReceive();
 *  CAmControlReceive* mpControlReceiveMainConnect = new gc::CAmControlReceive(controlReceiveInterfaceMainConnect);
 *
 *  //CAmMainConnectionElement* pMainConnection = NULL;
 *
 *  EXPECT_CALL(*controlReceiveInterfaceMainConnect,enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData),_)).WillOnce(DoAll(SetArgReferee<1>(44), Return(E_OK)));
 *  pMainConnection = CAmMainConnectionFactory::createElement(route1, mpControlReceiveMainConnect);
 *
 *
 *
 *
 * }*/
#endif // ifndef COMMON_H_

#if 0
# ifndef COMMON_H_
#  define COMMON_H_
// #include "CAmGcCommonFunctions.h"
#  include "gtest/gtest.h"
#  include "gmock/gmock.h"

namespace am
{
namespace gc {
class common : public ::testing::Test
{
public:
    common();
    ~common();
    CAmMainConnectionAcionConnectCommon(std::vector<am_Route_s>amRouteList);
    CAmGcCommonFunctions   pCF;
    CAmSinkElement        *pSinkElement;
    CAmSourceElement      *pSourceElement;
    CAmClassElement       *pClassElement;
    CAmClassActionConnect *ActionConnectClassConnect;
    am_MainConnection_s    mainConnectionData;
    am_MainConnection_s    mainConnectionData1;
    am_Handle_s            handle;
    am_Handle_s            handleSetSourceState;
    am_Source_s            source_out1;
    am_Source_s            source_out2;

};

}
}

namespace am
{
namespace gc {
class common : public ::testing::Test
{
public:
    common();
    ~common();
    CAmMainConnectionAcionConnectCommon(std::vector<am_Route_s>amRouteList);
    CAmGcCommonFunctions   pCF;
    CAmSinkElement        *pSinkElement;
    CAmSourceElement      *pSourceElement;
    CAmClassElement       *pClassElement;
    CAmClassActionConnect *ActionConnectClassConnect;
    am_MainConnection_s    mainConnectionData;
    am_MainConnection_s    mainConnectionData1;
    am_Handle_s            handle;
    am_Handle_s            handleSetSourceState;
    am_Source_s            source_out1;
    am_Source_s            source_out2;

};

}
}

common::CAmMainConnectionAcionConnectCommon(std::vector<am_Route_s>amRouteList)
{

    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID           = pSinkElement->getID();
    mainConnectionData.sourceID         = pSourceElement->getID();
    mainConnectionData.delay            = 0;
    mainConnectionData.connectionState  = CS_CONNECTING;

    mainConnectionData1.mainConnectionID = 0;
    mainConnectionData1.sinkID           = pSinkElement->getID();
    mainConnectionData1.sourceID         = pSourceElement->getID();
    mainConnectionData1.delay            = 0;
    mainConnectionData1.connectionState  = CS_DISCONNECTED;

    handle.handleType = H_CONNECT;
    handle.handle     = 50;

    handleSetSourceState.handleType = H_SETSOURCESTATE;
    handleSetSourceState.handle     = 50

        source_out1.sourceState = SS_UNKNNOWN;
    source_out2.sourceState     = SS_ON;

    EXPECT_CALL(*controlReceiveInterfaceClassConnect, getRoute(_, pSourceElement->getID(), pSinkElement->getID(), _)).WillOnce(DoAll(SetArgReferee<3>(amRouteList), Return(E_OK)));
    EXPECT_CALL(*controlReceiveInterfaceClassConnect, enterMainConnectionDB(IsMainConnSinkSourceOk(mainConnectionData), _)).WillOnce(DoAll(SetArgReferee<1>(44), Return(E_OK)));
    EXPECT_CALL(*controlReceiveInterfaceClassConnect, getMainConnectionInfoDB(44, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData1), Return(E_OK)));
    EXPECT_CALL(*controlReceiveInterfaceClassConnect, changeMainConnectionStateDB(44, CS_CONNECTING)).WillOnce(Return(E_OK));
    EXPECT_CALL(*controlReceiveInterfaceClassConnect, connect(_, _, CF_GENIVI_STEREO, pSourceElement->getID(), pSinkElement->getID())).WillOnce(DoAll(SetArgReferee<0>(handle), SetArgReferee<1>(44), Return(E_OK)));

    EXPECT_EQ(ActionConnectClassConnect->execute(), E_OK);
    mpControlReceiveClassConnect->notifyAsyncResult(handle, E_OK);
    EXPECT_CALL(*controlReceiveInterfaceSource, getSourceInfoDB(pSourceElement->getID(), _)).Times(2).WillOnce(DoAll(SetArgReferee<1>(source_out1), Return(E_OK))).WillOnce(DoAll(SetArgReferee<1>(source_out2), Return(E_OK)));
    EXPECT_CALL(*controlReceiveInterfaceSource, setSourceState(_, pSourceElement->getID(), SS_ON)).WillOnce(DoAll(SetArgReferee<0>(handleSetSourceState), Return(E_OK)));
    EXPECT_CALL(*controlReceiveInterfaceClassConnect, getMainConnectionInfoDB(44, _)).WillRepeatedly(DoAll(SetArgReferee<1>(mainConnectionData), Return(E_OK)));
    EXPECT_CALL(*controlReceiveInterfaceClassConnect, changeMainConnectionStateDB(44, CS_CONNECTED)).WillOnce(Return(E_OK));
    EXPECT_EQ(ActionConnectClassConnect->execute(), E_OK);
    mpControlReceiveSource->notifyAsyncResult(handleSetSourceState, E_OK);

}
# endif /* COMMON_H_ */

#endif // if 0
