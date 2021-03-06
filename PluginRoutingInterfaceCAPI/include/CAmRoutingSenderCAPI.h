/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013-2015
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#ifndef CAPIROUTINGSENDER_H_
#define CAPIROUTINGSENDER_H_

#include "IAmRouting.h"
#include "CAmCommonAPIWrapper.h"
#include "CAmRoutingService.h"
#include "CAmLookupData.h"

#ifdef UNIT_TEST
#include "../test/IAmRoutingSenderBackdoor.h" //we need this for the unit test
#endif

namespace am
{

#define ROUTING_NODE "routinginterface"

class CAmRoutingSenderCAPI: public IAmRoutingSend
{
    bool mIsServiceStarted;
    CAmLookupData mLookupData; ///<  an object which implements the lookup mechanism
    CAmCommonAPIWrapper *mpCAmCAPIWrapper; ///<  pointer to the common-api wrapper
    IAmRoutingReceive *mpIAmRoutingReceive; ///<  pointer to the routing receive interface
    std::shared_ptr<CAmRoutingService> mService; ///< shared pointer to the routing service implementation
    void checkSourceLookup(const am_sourceID_t id);
    void checkSinkLookup(const am_sinkID_t id);
public:
	CAmRoutingSenderCAPI();
    virtual ~CAmRoutingSenderCAPI();

    /** \brief starts the plugin - registers the routing interface service.
	 *
	 * @param  pIAmRoutingReceive pointer to the routing receive interface.
	 */
    am_Error_e startService(IAmRoutingReceive* pIAmRoutingReceive);

    /** \brief interface method which calls startService.
	 *
	 * @param  pIAmRoutingReceive pointer to the routing receive interface.
	 */
    am_Error_e startupInterface(IAmRoutingReceive* pIAmRoutingReceive);

    /** \brief stops the service - deregister the routing interface service.
	 *
	 * @param  pIAmRoutingReceive pointer to the routing receive interface.
	 */
    am_Error_e tearDownInterface(IAmRoutingReceive*);

    /** \brief sets and annotates the service ready state.
	 *
	 */
    void setRoutingReady(const uint16_t handle);

    /** \brief sets and annotates the service rundown state.
	 *
	 */
    void setRoutingRundown(const uint16_t handle);
    am_Error_e asyncAbort(const am_Handle_s handle);
    am_Error_e asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_CustomConnectionFormat_t connectionFormat);
    am_Error_e asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID);
    am_Error_e asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time);
    am_Error_e asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time);
    am_Error_e asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state);
    am_Error_e asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s>& listSoundProperties);
    am_Error_e asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty);
    am_Error_e asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s>& listSoundProperties);
    am_Error_e asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty);
    am_Error_e asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_CustomRampType_t rampType, const am_time_t time);
    am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState);
    am_Error_e returnBusName(std::string& busName) const;
    void getInterfaceVersion(std::string& version) const;
    am_Error_e asyncSetVolumes(const am_Handle_s handle, const std::vector<am_Volumes_s>& listVolumes) ;
    am_Error_e asyncSetSinkNotificationConfiguration(const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration) ;
    am_Error_e asyncSetSourceNotificationConfiguration(const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration) ;
    am_Error_e resyncConnectionState(const am_domainID_t domainID, std::vector<am_Connection_s>& listOfExistingConnections);
    static const char * ROUTING_INSTANCE;
    static const char * DEFAULT_DOMAIN;

#ifdef UNIT_TEST
    friend class IAmRoutingSenderBackdoor;
     static CAmRoutingSenderCAPI * newRoutingSenderCAPI(CAmCommonAPIWrapper *wrapper)
     {
    	 CAmRoutingSenderCAPI *pRoutingSenderCAPI = new CAmRoutingSenderCAPI();
    	 pRoutingSenderCAPI->mpCAmCAPIWrapper = wrapper;
    	 return pRoutingSenderCAPI;
     };

#endif

};
}

#endif /* CAPIROUTINGSENDER_H_ */
