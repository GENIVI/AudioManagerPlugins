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

#ifndef ROUTINGSENDERPULSE_H_
#define ROUTINGSENDERPULSE_H_

/* Includes */
#include "IAmRouting.h"
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <locale.h>
#include <map>
#include <pulse/pulseaudio.h>

using namespace am;

struct RoutingSenderPULSEConnection
{
    am_connectionID_t   connectionID;
    am_sourceID_t       sourceID;
    am_sinkID_t         sinkID;
    am_Handle_s         handle;
    bool                pending;
};


struct RoutingSenderPULSESourceSinkConfig
{
    am_Source_s source;
    am_Sink_s   sink;

    std::string name;//e.g. {"gst-launch-0.10",  "mono", "aplay"};
    std::string clazz;//e.g. {"Entertainment", "Navigation", "TTS"};
    std::string propertyName;//e.g. {"application.process.binary", "application.process.binary",    "application.process.app"};
    std::string propertyValue;//
};


/* Prototypes */
class RoutingSenderPULSE : public IAmRoutingSend
{
public:
    RoutingSenderPULSE(pa_context *p_paContext);
    ~RoutingSenderPULSE();

    am::am_Error_e startupInterface(am::IAmRoutingReceive* p_routingReceiver);
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
    void getInterfaceVersion(std::string& out_ver) const;

    void setPAContext(pa_context *p_paContext) {
        this->m_paContext = p_paContext;
    }
    am_Error_e asyncSetVolumes(const am_Handle_s handle, const std::vector<am_Volumes_s>& listVolumes);
    am_Error_e asyncSetSinkNotificationConfiguration(const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration);
    am_Error_e asyncSetSourceNotificationConfiguration(const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration);
	am_Error_e resyncConnectionState(const am_domainID_t domainID, std::vector<am_Connection_s>& listOfExistingConnections);

//Pulse Audio callbacks
    void getSinkInfoCallback(pa_context *c, const pa_sink_info *i, int is_last, void *userdata);
    void getSourceInfoCallback(pa_context *c, const pa_source_info *i, int is_last, void *userdata);
    void getSinkInputInfoCallback(pa_context *c, const pa_sink_input_info *i, void *userdata);
    void getSourceOutputInfoCallback(pa_context *c, const pa_source_output_info *i, void *userdata);

private:
    void loadConfig();

    am_Domain_s                                     m_domain;

    std::vector<RoutingSenderPULSESourceSinkConfig> m_sinks;
    std::vector<RoutingSenderPULSESourceSinkConfig> m_sources;

    std::map<uint16_t, uint32_t>                    m_sourceToPASinkInput;
    std::map<uint16_t, uint32_t>                    m_sourceToPASource;
    std::map<uint16_t, uint32_t>                    m_sinkToPASourceOutput;
    std::map<uint16_t, uint32_t>                    m_sinkToPASink;

    uint16_t                                        m_paSinkNullIndex;
    uint16_t                                        m_paSourceNullIndex;

    IAmRoutingReceive                               *m_routingReceiver;
    pa_context                                      *m_paContext;

/**
 * Maintain a list of pending actions: there is a high change that the HMI first call connect,
 * then the audio client start to play, therefore, sink-input is not yet created by the time "connect" method was called.
 * same for volume? not sure - probably the sink input is created when the user change the volume.
 * same for disconnect? not sure - probably the sink input was already created by the time the user is calling disconnect
 */
    std::vector<RoutingSenderPULSEConnection>       m_activeConnections;
    std::map<uint16_t, uint16_t>                    m_sinkToVolume;
    std::map<uint16_t, uint16_t>                    m_sourceToVolume;
};

#endif
