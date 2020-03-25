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

#ifndef MOCKIAMPOLICYRECEIVE_H_
#define MOCKIAMPOLICYRECEIVE_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am {
namespace gc {
class MockIAmPolicyReceive : public IAmPolicyReceive
{

public:
    MockIAmPolicyReceive()
    {
    }

    virtual ~MockIAmPolicyReceive()
    {
    }

    MOCK_METHOD2(setListActions,
        am_Error_e(std::vector<gc_Action_s >&listActions, gc_ActionList_e actionListType));
    MOCK_METHOD1(isDomainRegistrationComplete,
        bool(const std::string &domainName));
    MOCK_METHOD2(isRegistered,
        bool(const gc_Element_e elementType, const std::string &elementName));
    MOCK_METHOD3(getAvailability,
        am_Error_e(const gc_Element_e elementType, const std::string &elementName, am_Availability_s & availability));
    MOCK_METHOD3(getState,
        am_Error_e(const gc_Element_e elementType, const std::string &elementName, int &elementState));
    MOCK_METHOD3(getInterruptState,
        am_Error_e(const gc_Element_e elementType, const std::string &elementName, am_InterruptState_e & interruptState));
    MOCK_METHOD3(getMuteState,
        am_Error_e(const gc_Element_e elementType, const std::string &elementName, am_MuteState_e & MuteState));
    MOCK_METHOD4(getMainSoundProperty,
        am_Error_e(const gc_Element_e elementType, const std::string &elementName,
            const am_CustomMainSoundPropertyType_t propertyType, int16_t & value));
    MOCK_METHOD4(getSoundProperty,
        am_Error_e(const gc_Element_e elementType, const std::string &elementName, const am_CustomSoundPropertyType_t propertyType,
            int16_t & Value));
    MOCK_METHOD2(getSystemProperty,
        am_Error_e(const am_CustomSystemPropertyType_t systempropertyType, int16_t & Value));
    MOCK_METHOD3(getVolume,
        am_Error_e(const gc_Element_e elementType, const std::string &elementName, am_volume_t & DeviceVolume));
    MOCK_METHOD3(getMainVolume,
        am_Error_e(const gc_Element_e elementType, const std::string &elementName, am_mainVolume_t & volume));
    MOCK_METHOD3(getListMainConnections,
        am_Error_e(const gc_Element_e elementType, const std::string &name,
            std::vector<gc_ConnectionInfo_s >&listConnectionInfos));
    MOCK_METHOD3(getListNotificationConfigurations,
        am_Error_e(const gc_Element_e elementType, const std::string &name,
            std::vector<am_NotificationConfiguration_s >&listNotificationConfigurations));
    MOCK_METHOD3(getListMainNotificationConfigurations,
        am_Error_e(const gc_Element_e elementType, const std::string &name,
            std::vector<am_NotificationConfiguration_s >&listMainNotificationConfigurations));
    MOCK_METHOD3(getListMainConnections,
        am_Error_e(const std::string &name, std::vector<gc_ConnectionInfo_s >&listConnectionInfos, gc_Order_e order));
    MOCK_METHOD2(getDomainInfoByID,
        am_Error_e(const am_domainID_t domainID, am_Domain_s & domainInfo));
    MOCK_METHOD2(getListGatewaysOfDomain,
        am_Error_e(const am_domainID_t domainID, am_Domain_s & domainInfo));
    MOCK_CONST_METHOD2(getListGatewaysOfDomain,
        am_Error_e(const am_domainID_t domainID, std::vector<am_gatewayID_t > &listGatewaysIDs));
    MOCK_CONST_METHOD2(getListSinksOfDomain,
        am_Error_e(const am_domainID_t domainID, std::vector<am_sinkID_t > &listSinkIDs));
    MOCK_CONST_METHOD2(getListSourcesOfDomain,
        am_Error_e(const am_domainID_t domainID, std::vector<am_sourceID_t > &listSourceIDs));

};

}
}

#endif /* MOCKIAMPOLICYRECEIVE_H_ */
