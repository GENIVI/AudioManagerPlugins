/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2014, Wind River Systems
 * Copyright (C) 2014, GENIVI Alliance
 *
 * This file is part of Pulse Audio Interface Routing Plugin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License (MPL), v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 *
 * List of changes:
 *
 * 21.08.2014, Adrian Scarlat, First version of the code;
 *                             Porting code from AM ver1.x to AM ver3.0;
 *                             Added Copyright and License information;
 */

#ifndef CAMROUTINGSENDERPULSE_H_
#define CAMROUTINGSENDERPULSE_H_

/* Includes */

#include <mutex>
#include <unordered_map>

#include <pulse/pulseaudio.h>

#include "IAmRoutingReceiverShadow.h"
#include "CAmXmlConfigParser.h"
#include "CAmMainloopPulse.h"
#include "CAmConnectionManager.h"
#include "CAmVolumeManager.h"

namespace am
{

struct rp_Connection_s
{
    am_Handle_s         handle;
    am_connectionID_t   connectionID;
    am_sourceID_t       sourceID;
    am_sinkID_t         sinkID;
    bool                pending;
};

/* Prototypes */
class CAmRoutingSenderPulse : public CAmPulseClientItf, public IAmRoutingSend
{
public:
    CAmRoutingSenderPulse();
    ~CAmRoutingSenderPulse();

    am::am_Error_e startupInterface(am::IAmRoutingReceive* pRoutingReceiver);
    void setRoutingReady(uint16_t handle);
    void setRoutingRundown(uint16_t handle);
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
    am_Error_e returnBusName(std::string& BusName) const;
    void getInterfaceVersion(std::string& version) const;

    am_Error_e asyncSetVolumes(const am_Handle_s handle, const std::vector<am_Volumes_s>& listVolumes);
    am_Error_e asyncSetSinkNotificationConfiguration(const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration);
    am_Error_e asyncSetSourceNotificationConfiguration(const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration);
	am_Error_e resyncConnectionState(const am_domainID_t domainID, std::vector<am_Connection_s>& listOfExistingConnections);

	//Pulse Audio callbacks
    void onNewSource(const pa_source_info *info);
    void onRemoveSource(uint32_t idx) override;
    void onNewSink(const pa_sink_info *info) override;
    void onRemoveSink(uint32_t idx) override;
    void onNewSinkInput(const pa_sink_input_info *info) override;
    void onRemoveSinkInput(uint32_t idx) override;
    void onNewSourceOutput(const pa_source_output_info *info) override;
    void onRemoveSourceOutput(uint32_t idx) override;


private:

    bool cmpProperty(pa_proplist *proplist, const std::string& name, const std::string& value);

    am_sourceID_t getSourceIdByIdx(uint32_t idx, bool isVirtual);
    am_sinkID_t getSinkIdByIdx(uint32_t idx, bool isVirtual);
    am_sourceID_t getSourceIdByConfig(const char* name, pa_proplist *proplist);
    am_sinkID_t getSinkIdByConfig(const char* name, pa_proplist *proplist);

    void setSourceAvailable(am_sourceID_t sourceID, bool isAvailable);
    void setSinkAvailable(am_sinkID_t sourceID, bool isAvailable);

    void registerDomain(const rp_ConfigDomain_s& config);
    void registerSources(const std::vector<rp_ConfigSource_s>& listSources);
    void registerSinks(const std::vector<rp_ConfigSink_s>& listSinks);

    am_Error_e registerSource(const rp_ConfigSource_s& rpSource);
    am_Error_e deregisterSource(am_sourceID_t sourceID);
    am_Error_e registerSink(const rp_ConfigSink_s& rpSink);
    am_Error_e deregisterSink(am_sinkID_t sinkID);

    CAmMainloopPulse                                        mPulse;
    CAmVolumeManager                                        mVolumeManager;
    CAmConnectionManager                                    mConnectionManager;

    am_domainID_t                                           mDomainID;

    std::unordered_map<uint32_t, am_sourceID_t>             mSourceIdxToSourceId;
    std::unordered_map<uint32_t, am_sourceID_t>             mSinkInputIdxToSourceId;
    std::unordered_map<uint32_t, am_sinkID_t>               mSinkIdxToSinkId;
    std::unordered_map<uint32_t, am_sinkID_t>               mSourceOutputIdxToSinkId;

    std::unordered_map<am_sourceID_t, rp_ConfigSource_s>    mRegisteredSources;
    std::unordered_map<am_sourceID_t, rp_ConfigSink_s>      mRegisteredSinks;

    IAmRoutingReceiverShadow*                               mShadow;
};

}

#endif
