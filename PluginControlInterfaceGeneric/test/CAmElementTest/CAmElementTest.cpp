/*******************************************************************************
 *  \copyright (c) 2018 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
 *           Kapildev Patel, Yogesh Sharma <kpatel@jp.adit-jv.com> 2018
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
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <time.h>
#include "IAmAction.h"
#include "CAmTypes.h"
#include "CAmLogger.h"
#include "CAmSystemElement.h"
#include "CAmSourceElement.h"
#include "CAmClassElement.h"
#include "CAmSystemElement.h"
#include "CAmSinkElement.h"
#include "CAmElement.h"
#include "IAmPolicySend.h"
#include "MockIAmControlReceive.h"
#include "CGmockCommonFunctions.h"
#include "CAmGcCommonFunctions.h"
#include "CAmElementTest.h"
#include "CAmTestConfigurations.h"

using namespace std;
using namespace testing;
using namespace am;
using namespace gc;

/**
 *@Class : CAmElementTest
 *@brief : This class is used to test the CAmElement class functionality.
 */
CAmElementTest::CAmElementTest()
    : mpMockControlReceiveInterface(NULL)
    , mpCAmControlReceive(NULL)
    , mpCAmSinkElement(NULL)
    , sinkInfo("AnySink1")
    , sourceInfo("AnySource1")
    , mpCAmSourceElement(NULL)
    , mpCAmElement(NULL)
    , mpCAmClassElement(NULL)
{
}

CAmElementTest::~CAmElementTest()
{
}

void CAmElementTest::InitializeCommonStruct()
{
    sink.name        = "AnySink1";
    sink.sinkClassID = 78;
    sinkID           = 100;
    sourceID         = 100;
}

void CAmElementTest::SetUp()
{
    mpMockControlReceiveInterface = new MockIAmControlReceive();
    mpCAmControlReceive           = mpMockControlReceiveInterface;
    // This will check whether controller is initialized or not
    ASSERT_THAT(mpCAmControlReceive, NotNull()) << " Controller Not Initialized";
    // Initializing Common Struture
    InitializeCommonStruct();
    mpCAmElement = new CAmElement(ET_SINK, (string)sink.name, mpCAmControlReceive);
}

void CAmElementTest::TearDown()
{
    if (mpMockControlReceiveInterface != NULL)
    {
        delete (mpMockControlReceiveInterface);
        mpMockControlReceiveInterface = NULL;
    }

    if (mpCAmElement != NULL)
    {
        delete mpCAmElement;
        mpCAmElement = NULL;
    }
}

/**
 * @brief : Test to verify the attach function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *
 * @test : verify the "attach" function when sink element is created and pass as input parameter to attach function
 *         and along with other configuration parameters then will check whether attach function
 *         will register the element and push into the list or not.
 *
 * @result : "Pass" when attach function return without any Gmock error message
 */
TEST_F(CAmElementTest, attachPositive)
{
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID       = mpCAmSinkElement->getID();
    mpCAmElement = new CAmElement(ET_SINK, (string)sink.name, mpCAmControlReceive);
    mpCAmElement->attach(mpCAmSinkElement);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_NO_THROW(mpCAmElement->attach(mpCAmSinkElement));
}

/**
 * @brief : Test to verify the attach function when Subject Already Present for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *
 * @test : verify the "attach" function when sink element is created and pass as input parameter to attachSubject which
 *         already Present and along with other configuration parameters then will check whether attach function
 *         will register the element and push into the list or not which is already attached.
 *
 * @result : "Pass" when attach function return without any Gmock error message.
 */
TEST_F(CAmElementTest, attachSubjectAlreadyPresentPositive)
{
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID       = mpCAmSinkElement->getID();
    mpCAmElement = new CAmElement(ET_SINK, (string)sink.name, mpCAmControlReceive);
    mpCAmElement->attach(mpCAmSinkElement);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_NO_THROW(mpCAmElement->attach(mpCAmSinkElement));
}

/**
 * @brief : Test to verify the attach function when Subject Already Present for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *
 * @test : verify the "attach" function when sink element is not created and pass as input parameter to attach function
 *         and along with other configuration parameters then will check whether attach function
 *         will register the element and push into the list or not.
 *
 * @result : "Pass" when attach function return "E_NOT_POSSIBLE" without any Gmock error message.
 */
TEST_F(CAmElementTest, attachSubjectNegative)
{

    // mpCAmElement = new CAmElement((string)sink.name, mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    // CAmSinkElement *mpCAmSinkElement = NULL;
    mpCAmSinkElement = nullptr;
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmElement->attach(mpCAmSinkElement));
}

/**
 * @brief : Test to verify the detach function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *
 * @test : verify the detach function when sink element is created and pass as input parameter to attach function
 *         and along with other configuration parameters then will check whether
 *         detach function  will de-register the same element when pass to it and it will removed from the element from subject list or not.
 *
 * @result : "Pass" when detach function return "E_OK" without any Gmock error message
 */
TEST_F(CAmElementTest, detachPositive)
{
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    // mpCAmElement = new CAmElement((string)sink.name, mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmElement->attach(mpCAmSinkElement));
    EXPECT_EQ(E_OK, mpCAmElement->detach(mpCAmSinkElement));
}

/**
 * @brief : Test to verify the detach function With different element for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *
 * @test : verify the detach function when different sink element is created and pass as input parameter to attach function
 *         and along with other configuration parameters. Now detach function is passed with other element [any source element ]then
 *         will check whether detach function  will de-register the different element which is register and will removed from the element from subject list or not.
 *
 * @result : "Pass" when detach function return "E_OK" without any Gmock error message
 */
TEST_F(CAmElementTest, detachWithdifferentelementPositive)
{
    sink.name = "AnySink3";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    // mpCAmElement = new CAmElement((string)sink.name, mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    source.name = "AnySource3";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSourceDB(IsSourceNamePresent(sourceInfo), _)).Times(1).WillOnce(
        DoAll(SetArgReferee<1 >(22), Return(E_OK)));
    mpCAmSourceElement = CAmSourceFactory::createElement(sourceInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSourceElement, NotNull()) << " Source Element Is not Created";
    sourceIDList.push_back(mpCAmSourceElement->getID());
    sourceID = mpCAmSourceElement->getID();
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmElement->attach(mpCAmSinkElement));
    EXPECT_EQ(E_OK, mpCAmElement->detach(mpCAmSourceElement));

}

/**
 * @brief : Test to verify the detach function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *
 * @test : verify the detach function when sink element is created and pass as input parameter to attach function
 *         and along with other configuration parameters then will check whether
 *         detach function  will not de-register the other element when pass to it
 *         which is not register in list. de-registraion must not done for added sink element.
 *
 * @result : "Pass" when detach function return "E_NOT_POSSIBLE" without any Gmock error message
 */
TEST_F(CAmElementTest, detachNegative)
{
    // mpCAmElement = new CAmElement("AnySink4", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";

    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    // CAmSourceElement *pSourceElement = NULL;
    mpCAmSourceElement = nullptr;
    EXPECT_NO_THROW(mpCAmElement->attach(mpCAmSinkElement));
    EXPECT_EQ(E_NOT_POSSIBLE, mpCAmElement->detach(mpCAmSourceElement));
}

/**
 * @brief : Test to verify the getVolume function for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created.
 *
 * @test : verify the getVolume function when volume is set using the setVolume function first then check getVolume function will return
 *         set volume or not from child volume list
 *
 * @result : "Pass" when getVolume function return "expected volume" without any Gmock error message
 */
#if VOLUME_RELATED_TESTS
TEST_F(CAmElementTest, getVolumePositive)
{
    // mpCAmElement = new CAmElement("AnySink5", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    mpCAmElement->addVolumeElement(mpCAmSinkElement);
    am_volume_t volume = 10;
    mpCAmSinkElement->setVolume(volume);
    EXPECT_EQ(volume, mpCAmElement->getVolume());
}

/**
 * @brief : Test to verify the getVolume function for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created.
 *
 * @test : verify the getVolume function when volume is not set first then check getVolume function will return
 *         zero volume or not from child volume list
 *
 * @result : "Pass" when getVolume function return "zero volume" without any Gmock error message
 */
TEST_F(CAmElementTest, getVolumeNegative)
{
    // mpCAmElement = new CAmElement("AnySink6", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    am_volume_t volume = 0;
    EXPECT_EQ(volume, mpCAmElement->getVolume());
}

/**
 * @brief : Test to verify the getVolumeChangeElements function when Child Volume list is empty for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getVolumeChangeElements function when setInUse flag is enable and child volume list is empty then check
 *          getVolumeChangeElements function will get the volume and set in the list of Volume element.
 *
 * @result : "Pass" when getVolumeChangeElements function will not return any Gmock error message
 */
TEST_F(CAmElementTest, getVolumeChangeElementsPositiveChildVolumelistEmpty)
{
    // mpCAmElement = new CAmElement("AnySink8", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    am_volume_t requestedVolume = 150;
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

    ////std::map<CAmElement *, gc_volume_s > listVolumeElements;
    // std::map<std::shared_ptr<CAmElement >, gc_volume_s > listVolumeElements;

    // listVolumeElements.insert(pair<std::shared_ptr<CAmElement>, gc_volume_s>(mpCAmSinkElement, gcVolume));
    // mpCAmSinkElement->getVolumeChangeElements(requestedVolume, listVolumeElements);
}

/**
 * @brief : Test to verify the getVolumeChangeElements function when Child Volume list is empty for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getVolumeChangeElements function when setInUse flag is enable and child volume list is filled using AddVolumeElement function call
 *        then check getVolumeChangeElements function will get the volume and set in the list of Volume element.
 *
 * @result : "Pass" when getVolumeChangeElements function will not return any Gmock error message
 */
TEST_F(CAmElementTest, getVolumeChangeElementsPositive)
{
    // mpCAmElement = new CAmElement("AnySink8", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    mpCAmElement->addVolumeElement(mpCAmSinkElement);
    am_volume_t volume = 10;
    mpCAmSinkElement->setVolume(volume);
    am_volume_t requestedVolume = 150;
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

    // std::map<CAmElement *, gc_volume_s > listVolumeElements;
    // listVolumeElements.insert(pair<CAmElement *, gc_volume_s>(mpCAmSinkElement, gcVolume));
    // mpCAmElement->getVolumeChangeElements(requestedVolume, listVolumeElements);
}

/**
 * @brief : Test to verify the getOffsetVolume for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getOffsetVolume function when volume is set using the setOffsetVolume function
 *        then check getOffsetVolume function will get the volume as per expected volume or not
 *        from list of child element OffsetVolume.
 *
 * @result : "Pass" when getOffsetVolume function will return "expected volume" without any Gmock error message
 */
TEST_F(CAmElementTest, getOffsetVolumePositive)
{
    // mpCAmElement = new CAmElement("AnySink9", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    mpCAmElement->addVolumeElement(mpCAmSinkElement);
    am_volume_t volume = 100;
    mpCAmSinkElement->setOffsetVolume(volume);
    EXPECT_EQ(volume, mpCAmElement->getOffsetVolume());

}

/**
 * @brief : Test to verify the getOffsetVolume for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getOffsetVolume function when volume is not set then check getOffsetVolume function
 *         will get the zero volume or not from list of child element OffsetVolume.
 *
 * @result : "Pass" when getOffsetVolume function will return "zero volume" without any Gmock error message
 */
TEST_F(CAmElementTest, getOffsetVolumeNegative)
{
    // mpCAmElement = new CAmElement("AnySink10", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    am_volume_t volume = 0;
    EXPECT_EQ(volume, mpCAmElement->getOffsetVolume());
}

/**
 * @brief : Test to verify the getRootVolumeElements for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getRootVolumeElements function when sink element is push in the list of child elements volume and
 *          volume is set and also map details of root element is pass as input parameter to getRootVolumeElements then
 *          check the getRootVolumeElements function will get the root volume details of the sink element type.
 *
 * @result : "Pass" when getRootVolumeElements function will return, without any Gmock error message
 */
TEST_F(CAmElementTest, getRootVolumeElementsPositive)
{
    // mpCAmElement = new CAmElement("AnyClass1", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    mpCAmElement->addVolumeElement(mpCAmSinkElement);
    am_volume_t volume = 25;
    mpCAmSinkElement->setVolume(volume);
    std::map<std::shared_ptr<CAmElement>, am_volume_t>mapRootElements;
    mapRootElements.insert(pair<std::shared_ptr<CAmElement>, am_volume_t>(mpCAmSinkElement, volume));
    mpCAmElement->getRootVolumeElements(mapRootElements);

}

/**
 * @brief : Test to verify the getRootVolumeElements for Negative scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getRootVolumeElements function when sink element is not push in the list of child elements volume and
 *          volume is set and also map details of root element is pass as input parameter to getRootVolumeElements then
 *          check the getRootVolumeElements function will get the root volume details of the sink element type.
 *
 * @result : "Pass" when getRootVolumeElements function will return , without any Gmock error message
 */
TEST_F(CAmElementTest, getRootVolumeElementsNegative)
{
    // mpCAmElement = new CAmElement("AnyClass1", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
////Class Element
    std::vector<std::vector<gc_TopologyElement_s > > listTopologies;
    std::vector<gc_TopologyElement_s >               GClistTopolologyElement;
// gc_TopologyElement_s                             topologyElementsource;
// topologyElementsource.name   = "AnySource1";
// topologyElementsource.codeID = MC_SOURCE_ELEMENT;
// GClistTopolologyElement.push_back(topologyElementsource);
    // source Element belongs to class
    gc_Class_s classElement;
    classElement.name     = "AnyClass1";
    classElement.type     = C_PLAYBACK;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
    // sink Element belongs to class
// gc_TopologyElement_s topologyElementsink;
// topologyElementsink.name   = "AnySink1";
// topologyElementsink.codeID = MC_SINK_ELEMENT;
// GClistTopolologyElement.push_back(topologyElementsink);
    classElement.name     = "AnyClass1";
    classElement.type     = C_CAPTURE;
    classElement.priority = 1;
    classElement.listTopologies.push_back(GClistTopolologyElement);
// classElement.registrationType = REG_CONTROLLER;

    // SystemProperty creating
    am_SystemProperty_s systemproperty;
    systemproperty.type  = SYP_GLOBAL_LOG_THRESHOLD;
    systemproperty.value = 10;

    std::vector<am_SystemProperty_s > listSystemProperties;
    listSystemProperties.push_back(systemproperty);
    // System element
    gc_System_s systemConfiguration;
    systemConfiguration.name = "System1";
    // systemConfiguration.listSystemProperties = listSystemProperties;
    std::shared_ptr<CAmSystemElement> pSystem;
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSystemPropertiesListDB(setproperty(systemproperty))).WillOnce(Return(E_OK));
    pSystem = CAmSystemFactory::createElement(systemConfiguration, mpCAmControlReceive);
    ASSERT_THAT(pSystem, NotNull()) << " pSystem is Not Initialized";
////Creating Class Element instancce
    mpCAmClassElement = new CAmClassElement(classElement, mpCAmControlReceive);
    ASSERT_THAT(mpCAmClassElement, NotNull()) << " mpCAmClassElement is Not Initialized";

    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    am_volume_t volume = 25;
    mpCAmSinkElement->setVolume(volume);
    std::map< CAmElement *, am_volume_t>mapRootElements;
    mapRootElements.insert(pair<CAmElement *, am_volume_t>(mpCAmElement, volume));
// mpCAmElement->getRootVolumeElements(mapRootElements);

}

/**
 * @brief : Test to verify the getLimitVolumeElements for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getLimitVolumeElements function when the list of child elements volume is empty and
 *          set InUse flag is true and also set min volume using setMinVolume function and
 *          list of volume element and limit volume are passed as input parameters to getLimitVolumeElements function then
 *          check the getLimitVolumeElements function will return the limit volume details or not base on calculation with Max limit volume
 *          and list of volume elements.
 *
 * @result : "Pass" when getLimitVolumeElements function will return limit volume, without any Gmock error message
 */
TEST_F(CAmElementTest, getLimitVolumeElementsPositiveChildVolumelistEmpty)
{
    // mpCAmElement = new CAmElement("AnySink8", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();
    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    am_volume_t minVolume = 20;
    mpCAmSinkElement->setMinVolume(minVolume);
    am_volume_t limitVolume = 5;
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;
    // std::map<CAmElement *, gc_volume_s > listVolumeElements;
    // listVolumeElements.insert(pair<CAmElement *, gc_volume_s>(mpCAmSinkElement, gcVolume));
    // mpCAmSinkElement->getLimitVolumeElements(limitVolume, listVolumeElements);

}

/**
 * @brief : Test to verify the getLimitVolumeElements for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getLimitVolumeElements function when sink element is push in the list of child elements volume and
 *          set InUse flag is true, set min volume using setMinVolume function and also
 *          empty list of volume element and limit volume are passed as input parameters to getLimitVolumeElements function then
 *          check the getLimitVolumeElements function will return the limit volume details or not base on calculation with
 *          Max limit volume and getVolume.
 *
 * @result : "Pass" when getLimitVolumeElements function will return limit volume, without any Gmock error message
 */
TEST_F(CAmElementTest, getLimitVolumeElementsPositiveVolumeLimitElementMapEmpty)
{
    // mpCAmElement = new CAmElement("AnySink14", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    am_volume_t minVolume = 20;
    mpCAmSinkElement->setMinVolume(minVolume);
    am_volume_t limitVolume = 5;
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;
    // std::map<CAmElement *, gc_volume_s > listVolumeElements;
    // mpCAmSinkElement->getLimitVolumeElements(limitVolume, listVolumeElements);
}

/**
 * @brief : Test to verify the getLimitVolumeElements when limit and offset volume same for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getLimitVolumeElements function when sink element is push in the list of child elements volume and
 *          limit volume and offset volume is set same and also list of volume element and limit volume are passed as
 *          input parameters to getLimitVolumeElements function then check the getLimitVolumeElements function will assign the limit volume
 *          to Offset Volume of volume element and limit volume and set offset volume to true and limit volume is return base on getOffsetVolume details.
 *
 * @result : "Pass" when getLimitVolumeElements function will return limit volume, without any Gmock error message
 */
TEST_F(CAmElementTest, getLimitVolumeElementsPositiveOffsetlimitvolumeEqual)
{
    // mpCAmElement = new CAmElement("AnySink15", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    am_volume_t minVolume = 20;
    mpCAmSinkElement->setMinVolume(minVolume);
    am_volume_t volume = 10;
    mpCAmSinkElement->setOffsetVolume(volume);
    am_volume_t limitVolume = 5;
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

    // std::map<CAmElement *, gc_volume_s > listVolumeElements;
    // listVolumeElements.insert(pair<CAmElement *, gc_volume_s>(mpCAmSinkElement, gcVolume));
    // mpCAmSinkElement->getLimitVolumeElements(limitVolume, listVolumeElements);
}

/**
 * @brief : Test to verify the getLimitVolumeElements when limit and offset volume same for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the getLimitVolumeElements function when sink element is push in the list of child elements volume and
 *          limit volume and offset volume is set different  and also list of volume element and limit volume are passed as
 *          input parameters to getLimitVolumeElements function then check the getLimitVolumeElements function will return the limit volume using the
 *          OffSetVolume from list of child element volume.
 *
 * @result : "Pass" when getLimitVolumeElements function will return limit volume, without any Gmock error message
 */
TEST_F(CAmElementTest, getLimitVolumeElementsPositiveElementUsage)
{
    // mpCAmElement = new CAmElement("AnySink1", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    mpCAmSinkElement->setInUse(inUse);
    am_volume_t minVolume = 20;
    mpCAmSinkElement->setMinVolume(minVolume);
    am_volume_t volume = 10;
    mpCAmSinkElement->setOffsetVolume(volume);
    am_volume_t limitVolume = 5;
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

    // std::map<CAmElement *, gc_volume_s > listVolumeElements;
    // listVolumeElements.insert(pair<CAmElement *, gc_volume_s>(mpCAmSinkElement, gcVolume));
    // mpCAmSinkElement->getLimitVolumeElements(limitVolume, listVolumeElements);
}

/**
 * @brief : Test to verify the clearLimitVolume when child volume element list is empty for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the clearLimitVolume function when list of child volume element is empty and InUse flag and min volume
 *          and offSet Volume is set then check clearLimitVolume function will set the gc_volume_s isOffsetSet as true and isvolumeSet as false or not.
 *          and in list of volume element set isOffSetSet for child element to true an volume zero.
 *
 * @result : "Pass" when clearLimitVolume function will return void, without any Gmock error message
 */
TEST_F(CAmElementTest, clearLimitVolumePositiveChildvolumeElementEmpty)
{
    // mpCAmElement = new CAmElement("AnySink18", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    am_volume_t minVolume = 20;
    mpCAmSinkElement->setMinVolume(minVolume);
    am_volume_t volume = -3000;
    mpCAmSinkElement->setOffsetVolume(volume);
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

    std::map<std::shared_ptr<CAmElement >, gc_volume_s > listVolumeElements;
    listVolumeElements.insert(pair<std::shared_ptr<CAmElement >, gc_volume_s>(mpCAmSinkElement, gcVolume));
    mpCAmSinkElement->clearLimitVolume(listVolumeElements);
}

/**
 * @brief : Test to verify the clearLimitVolume when child volume element list is empty for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the clearLimitVolume function when InUse flag and min volume and offSet Volume is set
 *         and also map of volume element is empty then check clearLimitVolume function will
 *         set the gc_volume_s isOffsetSet as true and isvolumeSet as false or not.
 *         and in list of volume element set isOffSetSet for child element to true an volume zero.
 *
 * @result : "Pass" when clearLimitVolume function will return void, without any Gmock error message
 */
TEST_F(CAmElementTest, clearLimitVolumePositiveVolumelimitElementMapEmpty)
{
    // mpCAmElement = new CAmElement("AnySink8", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    am_volume_t minVolume = 20;
    mpCAmSinkElement->setMinVolume(minVolume);
    am_volume_t volume = -3000;
    mpCAmSinkElement->setOffsetVolume(volume);
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;
    std::map<std::shared_ptr<CAmElement>, gc_volume_s > listVolumeElements;
    mpCAmSinkElement->clearLimitVolume(listVolumeElements);
}

/**
 * @brief : Test to verify the clearLimitVolume when child volume element list is not empty for Positive scenario
 *
 * @pre : Mock the IAmControlReceive class and initialized the CAmControlReceive class,
 *        CAmElement is created and Sink Element is created and sinkID is push in Sink id list.
 *
 * @test : verify the clearLimitVolume function when InUse flag and min volume and offSet Volume is set
 *         and also map of volume element is not empty then check clearLimitVolume function will recursive call the clearLimitVolume and
 *         set the gc_volume_s isOffsetSet as true and isvolumeSet as false or not.
 *         and in list of volume element set isOffSetSet for child element to true an volume zero.
 *
 * @result : "Pass" when clearLimitVolume function will return void, without any Gmock error message
 */
TEST_F(CAmElementTest, clearLimitVolumePositiveChildvolumeElementnotEmpty)
{
    // mpCAmElement = new CAmElement("AnySink8", mpCAmControlReceive);
    ASSERT_THAT(mpCAmElement, NotNull()) << " CAmElement is Not Initialized";
    EXPECT_CALL(*mpMockControlReceiveInterface, enterSinkDB(IsSinkNamePresent(sinkInfo), _)).Times(0);
    mpCAmSinkElement = CAmSinkFactory::createElement(sinkInfo, mpCAmControlReceive);
    ASSERT_THAT(mpCAmSinkElement, NotNull()) << " Sink Element Is not Created";
    sinkIDList.push_back(mpCAmSinkElement->getID());
    sinkID = mpCAmSinkElement->getID();

    mpCAmElement->addVolumeElement(mpCAmSinkElement);
    bool inUse = true;
    mpCAmSinkElement->setInUse(inUse);
    am_volume_t minVolume = 20;
    mpCAmSinkElement->setMinVolume(minVolume);
    am_volume_t volume = -3000;
    mpCAmSinkElement->setOffsetVolume(volume);
    gc_volume_s gcVolume;
    gcVolume.isvolumeSet  = true;
    gcVolume.volume       = 10;
    gcVolume.isOffsetSet  = true;
    gcVolume.offsetVolume = 5;

    std::map<std::shared_ptr<CAmElement >, gc_volume_s > listVolumeElements;
    listVolumeElements.insert(pair<std::shared_ptr<CAmElement >, gc_volume_s>(mpCAmSinkElement, gcVolume));
    mpCAmElement->clearLimitVolume(listVolumeElements);
}
#endif
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
