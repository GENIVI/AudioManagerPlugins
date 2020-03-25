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

#ifndef PLUGINCONTROLINTERFACEGENERIC_TEST_MOCKIAMPOLICYSEND_H_
#define PLUGINCONTROLINTERFACEGENERIC_TEST_MOCKIAMPOLICYSEND_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am {
namespace gc {
class MockIAmPolicySend : public IAmPolicySend
{

public:
    MockIAmPolicySend()
    {
    }

    virtual ~MockIAmPolicySend()
    {
    }

    MOCK_METHOD1(startupInterface,
        am_Error_e(IAmPolicyReceive * pPolicyReceive));
    MOCK_METHOD2(hookRegisterDomain,
        am_Error_e(const std::string &domainName, const am_Error_e status));
    MOCK_METHOD2(hookRegisterSource,
        am_Error_e(const std::string &sourceName, const am_Error_e status));
    MOCK_METHOD2(hookRegisterSink,
        am_Error_e(const std::string &sinkName, const am_Error_e status));
    MOCK_METHOD2(hookRegisterGateway,
        am_Error_e(const std::string &gatewayName, const am_Error_e status));
    MOCK_METHOD2(hookDeregisterDomain,
        am_Error_e(const std::string &domainName, const am_Error_e status));
    MOCK_METHOD2(hookDeregisterSource,
        am_Error_e(const std::string &sourceName, const am_Error_e status));
    MOCK_METHOD2(hookDeregisterSink,
        am_Error_e(const std::string &sinkName, const am_Error_e status));
    MOCK_METHOD2(hookDeregisterGateway,
        am_Error_e(const std::string &gatewayName, const am_Error_e status));
    MOCK_METHOD1(hookDomainRegistrationComplete,
        am_Error_e(const std::string &domainName));
    MOCK_METHOD3(hookConnectionRequest,
        am_Error_e(const std::string &className, const std::string &sourceName, const std::string &sinkName));
    MOCK_METHOD3(hookDisconnectionRequest,
        am_Error_e(const std::string &className, const std::string &sourceName, const std::string &sinkName));
    MOCK_METHOD2(hookSourceAvailabilityChange,
        am_Error_e(const std::string &sourceName, const am_Availability_s &Availability));
    MOCK_METHOD2(hookSinkAvailabilityChange,
        am_Error_e(const std::string &sinkName, const am_Availability_s &Availability));
    MOCK_METHOD2(hookSourceInterruptStateChange,
        am_Error_e(const std::string &sourceName, const am_InterruptState_e interruptState));
    MOCK_METHOD2(hookSetMainSourceSoundProperty,
        am_Error_e(const std::string &sourceName, const am_MainSoundProperty_s &soundProperty));
    MOCK_METHOD2(hookSetMainSinkSoundProperty,
        am_Error_e(const std::string &sinkName, const am_MainSoundProperty_s &soundProperty));
    MOCK_METHOD1(hookSetSystemProperty,
        am_Error_e(const am_SystemProperty_s &systemProperty));
    MOCK_METHOD2(hookSetMainSinkNotificationConfiguration,
        am_Error_e(const std::string &sinkName, const am_NotificationConfiguration_s &notificationConfiguration));
    MOCK_METHOD2(hookSetMainSourceNotificationConfiguration,
        am_Error_e(const std::string &sourceName, const am_NotificationConfiguration_s &notificationConfiguration));
    MOCK_METHOD2(hookSinkNotificationDataChanged,
        am_Error_e(const std::string &sinkName, const am_NotificationPayload_s &payload));
    MOCK_METHOD2(hookSourceNotificationDataChanged,
        am_Error_e(const std::string &sourceName, const am_NotificationPayload_s &payload));
    MOCK_METHOD2(hookVolumeChange,
        am_Error_e(const std::string &sinkName, const am_mainVolume_t mainVolume));
    MOCK_METHOD2(hookSetSinkMuteState,
        am_Error_e(const std::string &sinkName, const am_MuteState_e muteState));
    MOCK_METHOD3(hookConnectionStateChange,
        am_Error_e(const std::string &connectionName, const am_ConnectionState_e &connectionState, am_Error_e & status));
    MOCK_METHOD2(getListElements,
        am_Error_e(const std::vector<std::string >&listNames, std::vector<gc_Sink_s >&listSinks));
    MOCK_METHOD2(getListElements,
        am_Error_e(const std::vector<std::string >&listNames, std::vector<gc_Source_s >&listSources));
    MOCK_METHOD2(getListElements,
        am_Error_e(const std::vector<std::string >&listNames, std::vector<gc_Gateway_s >&listGateways));
    MOCK_METHOD2(getListElements,
        am_Error_e(const std::vector<std::string >&listNames, std::vector<gc_Domain_s >&listDomain));
    MOCK_METHOD1(getListSystemProperties,
        am_Error_e(std::vector<am_SystemProperty_s>&listSystemProperties));
    MOCK_METHOD1(getListClasses,
        am_Error_e(std::vector<gc_Class_s >&listClasses));
    MOCK_METHOD1(hookAllDomainRegistrationComplete,
        am_Error_e(const am_Error_e &error));
    MOCK_METHOD1(getListDomains,
        am_Error_e(std::vector<gc_Domain_s >&listDomains));

    MOCK_METHOD1(rundownInterface, void(const int16_t signal));

    MOCK_METHOD1(hookSetSystemProperties,
        am_Error_e(const std::vector<am_SystemProperty_s> &listSystemProperties));

    MOCK_METHOD2(hookSetMainSourceSoundProperties,
        am_Error_e(const std::string &sourceName, const std::vector<am_MainSoundProperty_s> &listMainsoundProperty));

    MOCK_METHOD2(hookSetMainSinkSoundProperties,
        am_Error_e(const std::string &sinkName, const std::vector<am_MainSoundProperty_s> &listMainsoundProperty));

    MOCK_METHOD3(hookVolumeChange,
        am_Error_e(const std::string &sinkName, const am_mainVolume_t mainVolume, bool isStep));

    MOCK_METHOD1(getListSystemProperties,
        am_Error_e(std::vector<gc_SystemProperty_s > &listSystemProperties));

};

}
}

#endif /* PLUGINCONTROLINTERFACEGENERIC_TEST_MOCKIAMPOLICYSEND_H_ */
