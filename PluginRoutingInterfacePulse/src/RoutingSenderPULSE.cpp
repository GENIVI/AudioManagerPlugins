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
 *
 *
 * DESCRIPTION
 *
 * This module is handling requests form AudioManager daemon and redirect them to Pulse Audio server.
 * It keeps track of existing audio sink, sources, sink-input, sources-input and performs connection/disconnection
 * and other operations upon AudioManager daemon request.
 *
 * The modul is configured with a static list of sink and sources.
 *   Sinks are: audio output(e.g. speakers) or recording applications.
 *   Sources are: playback applications or audio input(e.g. microphone)
 *
 * In PulseAudio server those are classified as follows:
 *  - audio output             - sinks:   identified by name or properties: device.api (e.g = "alsa") & device.class(e.g. = "sound")
 *  - audio input              - sources: identified by name or properties: device.api (e.g = "alsa") & device.class(e.g. = "sound")
 *  - playback applications    - sink inputs: identified by application.name (e.g. "ALSA plug-in [chromium-browser]",
 *  - recording applications   - source outputs: identified by application.name (e.g. "ALSA plug-in [chromium-browser]",
 *    application.process.user = "popai", application.process.binary = "chromium-browser")
 *
 */

#include <cstring>

#include "CAmDltWrapper.h"

#include "RoutingSenderPULSE.h"
#include "RoutingSenderMainloopPULSE.h"

using namespace std;

namespace am
{

/* Globals */

/* Defines */
DLT_DECLARE_CONTEXT(routingPulse)

/**
 * Factory function for the plug-in to be used by audio manager daemon with dlopen & dlsym functions.
 * Pattern "libraryName"FACTORY
 *
 * @author  Ionut Popa (ionut.popa@windriver.com)
 * @return  an instance of RoutingSendInterface or type RoutingSenderPULSE.
 */
extern "C" IAmRoutingSend* PluginRoutingInterfacePULSEFactory()
{
    return (new RoutingSenderPULSE(nullptr));
}

/**
 * Destructor function for the plug-in to be used by audio manager daemon with dl_open & dl_sym functions.
 *
 * @param  routingSendInterface - the instance created by PluginRoutingInterfaceDbusFactory
 * @author Ionut Popa (ionut.popa@windriver.com)
 */
extern "C" void destroyPluginRoutingInterfacePULSE(IAmRoutingSend* routingSendInterface)
{
    delete routingSendInterface;//virtual destructor -> our constructor will be called too
}

/**
 * Constructor.
 * @param p_paContext - reference to PulseAudio context
 */
RoutingSenderPULSE::RoutingSenderPULSE(pa_context *p_paContext)
{
    m_paSinkNullIndex = -1;
    m_paSourceNullIndex = -1;
    m_paContext = p_paContext;
}

/**
 * Destructor.
 */
RoutingSenderPULSE::~RoutingSenderPULSE()
{
    //TODO: Disconnect from pulse: quit main loop and free the context and stuff
}

/**
 * Connecting sender & receiver
 * @author Ionut Popa (ionut.popa@windriver.com)
 */
am_Error_e RoutingSenderPULSE::startupInterface(am::IAmRoutingReceive *p_routingReceiver)
{
    assert(p_routingReceiver);
    CAmSocketHandler *l_socketHandler;
    p_routingReceiver->getSocketHandler(l_socketHandler);
    assert(l_socketHandler);

    m_shadow = new IAmRoutingReceiverShadow(p_routingReceiver, l_socketHandler);
    return am::E_OK;
}

void RoutingSenderPULSE::setRoutingReady(uint16_t handle)
{
    //TODO: do not register sinks with the same name
    loadConfig();

    logInfo("PULSE - routingInterfacesReady");
    m_shadow->confirmRoutingReady(handle, am::E_OK);

    //register pulse sink & sources, sink inputs & source outputs - > start the main PA loop
    routing_sender_create_mainloop((void *) this);
}

void RoutingSenderPULSE::setRoutingRundown(uint16_t handle)
{
    m_shadow->confirmRoutingRundown(handle, am::E_OK);
    //TODO: implement this
}

am_Error_e RoutingSenderPULSE::asyncAbort(const am_Handle_s handle)
{
    (void) handle;
    return E_NOT_USED;
}

am_Error_e RoutingSenderPULSE::asyncConnect(
        const am_Handle_s handle,
        const am_connectionID_t connectionID,
        const am_sourceID_t sourceID,
        const am_sinkID_t sinkID,
        const am_CustomConnectionFormat_t connectionFormat)
{
    //TODO: check stuff like connectionFormat
    logInfo("PULSE - asyncConnect() - start");

    //add source,sink & connectionID to a list of connections maintained by Routing Pulse Engine
    RoutingSenderPULSEConnection l_newConnection {};

    l_newConnection.sinkID = sinkID;
    l_newConnection.sourceID = sourceID;
    l_newConnection.connectionID = connectionID;

    if (m_sinkToPASink[sinkID] != -1)
    {
        if (m_sourceToPASinkInput[sourceID] != -1)
        {
            if (routing_sender_move_sink_input(m_paContext, m_sourceToPASinkInput[sourceID], m_sinkToPASink[sinkID], this))
            {
                //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation

                logInfo("PULSE - asyncConnect() - connectionID:", connectionID,
                        "move sinkInputIndex:", m_sourceToPASinkInput[sourceID], "to sinkIndex:", m_sinkToPASink[sinkID]);
            }
            else
            {
                m_shadow->ackConnect(handle, connectionID, am::E_NOT_POSSIBLE);
                return am::E_NOT_POSSIBLE;
            }
        }
        else
        {
            logInfo("PULSE - asyncConnect() - connectionID:", connectionID, " pending");

            l_newConnection.pending = true;
        }
        //else move_sink_input will be called later
    }
    else if (m_sourceToPASource[sourceID] != -1)
    {
        if (m_sinkToPASourceOutput[sinkID] != -1)
        {
            if (routing_sender_move_source_output(
                    m_paContext,
                    m_sinkToPASourceOutput[sinkID],
                    m_sourceToPASource[sourceID],
                    this))
            {
                logInfo("PULSE - asyncConnect() - connectionID:", connectionID,
                        "move sourceOutputIndex:", m_sinkToPASourceOutput[sinkID], "to sourceIndex:", m_sourceToPASource[sourceID]);
            }
            else
            {
                m_shadow->ackConnect(handle, connectionID, am::E_NOT_POSSIBLE);
                return am::E_NOT_POSSIBLE;
            }
        }
        else
        {
            l_newConnection.pending = true;
        }
        //else move_sink_input will be called later
    }
    else
    {
        logError("Sink and source for connection not identified:",
                sinkID, sourceID, connectionID);

        m_shadow->ackConnect(handle, connectionID, am::E_NOT_POSSIBLE);
        return am::E_NOT_POSSIBLE;
    }

    m_activeConnections.push_back(l_newConnection);

    m_shadow->ackConnect(handle, connectionID, am::E_OK);

/**
 * TODO: connection is always possible ? check that
*/

    return am::E_OK;
}


am_Error_e RoutingSenderPULSE::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    //get connection by ID ... not to many connections, therefore linear search is fast enough
    vector<RoutingSenderPULSEConnection>::iterator iter    = m_activeConnections.begin();
    vector<RoutingSenderPULSEConnection>::iterator iterEnd = m_activeConnections.end();
    for (; iter < iterEnd; ++iter)
    {
        if (iter->connectionID == connectionID)
        {
            if (m_sourceToPASinkInput[iter->sourceID] != -1)
            {
                if (m_paSinkNullIndex >= 0)
                {
                    //if null sink is defined - disconnect = move sink input to null
                    logInfo("PULSE - asyncDisconnect() - connection found - move sinkInputIndex:",
                            m_sourceToPASinkInput[iter->sourceID], "to NULL sinkIndex:", m_paSinkNullIndex);

                    routing_sender_move_sink_input(
                            m_paContext,
                            m_sourceToPASinkInput[iter->sourceID],
                            m_paSinkNullIndex,
                            this);
                    //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation
                    m_shadow->ackDisconnect(handle, connectionID, am::E_OK);

                }
            }
            else if (m_sinkToPASourceOutput[iter->sinkID] != -1)
            {
                if (m_paSourceNullIndex >= 0)
                {
                    //if null source is defined - disconnect = move source output to null
                    logInfo("PULSE - asyncDisconnect() - connection found - move sourceOutputIndex:",
                            m_sinkToPASourceOutput[iter->sinkID], "to NULL sourceIndex:", m_paSourceNullIndex);

                    routing_sender_move_source_output(
                            m_paContext,
                            m_sourceToPASinkInput[iter->sourceID],
                            m_paSinkNullIndex,
                            this);
                    //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation
                    m_shadow->ackDisconnect(handle, connectionID, am::E_OK);

                    //remove connection from the list of active connections
                    iter = m_activeConnections.erase(iter);

                    break;
                }
            }
            else
            {
                logInfo("PULSE - asyncDisconnect() - connection found - but no sink input or source");
                m_shadow->ackDisconnect(handle, connectionID, am::E_OK);
            }
            //remove connection from the list of active connections
            iter = m_activeConnections.erase(iter);

            break;
        }
    }
    return am::E_OK;
}

am_Error_e RoutingSenderPULSE::asyncSetSinkVolume(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const am_volume_t volume,
        const am_CustomRampType_t ramp,
        const am_time_t time)
{
    (void) ramp;
    (void) time;

    logInfo("PULSE - asyncSetSinkVolume() - volume:", volume, "sink index:", m_sinkToPASink[sinkID]);

    routing_sender_sink_volume(
            m_paContext,
            m_sinkToPASink[sinkID],
            volume,
            this);
    m_shadow->ackSetSinkVolumeChange(handle, volume, E_OK);
    return E_OK;
}

am_Error_e RoutingSenderPULSE::asyncSetSourceVolume(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const am_volume_t volume,
        const am_CustomRampType_t ramp,
        const am_time_t time)
{
    (void) ramp;
    (void) time;

    pa_volume_t crt_volume = m_sourceToVolume[sourceID];
    m_sourceToVolume[sourceID] = pa_sw_volume_from_dB(volume*0.1);

    logInfo("PULSE - asyncSetSourceVolume() - volume:", volume, "sink input index:", m_sourceToPASinkInput[sourceID]);
    if (m_sourceToPASinkInput[sourceID] != -1)
    {
#if 0
        if (time == 0)
        {/* without ramp time */
#endif
            routing_sender_sink_input_volume(
                    m_paContext,
                    m_sourceToPASinkInput[sourceID],
                    m_sourceToVolume[sourceID],
                    this);
#if 0
        }
        else
        {/* with ramp time */
            routing_sender_sink_input_volume_ramp(
                    m_paContext,
                    m_sourceToPASinkInput[sourceID],
                    crt_volume,
                    m_sourceToVolume[sourceID],
                    (uint16_t)time,
                    this);
        }
#endif
    }
    else
    {
        logInfo("PULSE - sink input not registered yet - should wait for registration before update the volume");
    }
    m_shadow->ackSetSourceVolumeChange(handle, volume, E_OK);

}

am_Error_e RoutingSenderPULSE::asyncSetSourceState(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const am_SourceState_e state)
{
    logInfo("PULSE - asyncSetSourceState", state);
    switch (state)
    {
        case SS_ON:
        {
            routing_sender_sink_input_mute(
                    m_paContext,
                    m_sourceToPASinkInput[sourceID],
                    false,
                    this
            );
            break;
        }
        case SS_OFF:
        case SS_PAUSED:
        {
            //TODO: mute source in case of PAUSE or OFF - is there a better way to pause ? maybe suspending the associated sink?
            routing_sender_sink_input_mute(
                    m_paContext,
                    m_sourceToPASinkInput[sourceID],
                    true,
                    this
            );
            break;
        }
        default:
        {
            logError("RoutingSenderPULSE::asyncSetSourceState - wrong source state\n");
            m_shadow->ackSetSourceState(handle, E_NOT_POSSIBLE);
            return E_NOT_POSSIBLE;
        }
    }
    m_shadow->ackSetSourceState(handle, E_OK);
    return E_OK;
}

am_Error_e RoutingSenderPULSE::asyncSetSinkSoundProperties(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const vector<am_SoundProperty_s>& listSoundProperties)
{
    (void) handle;
    (void) sinkID;
    (void) listSoundProperties;
    return E_NOT_USED;
}

am_Error_e RoutingSenderPULSE::asyncSetSinkSoundProperty(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const am_SoundProperty_s& soundProperty)
{
    (void) handle;
    (void) sinkID;
    (void) soundProperty;
    return E_NOT_USED;
}

am_Error_e RoutingSenderPULSE::asyncSetSourceSoundProperties(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const vector<am_SoundProperty_s>& listSoundProperties)
{
    (void) handle;
    (void) sourceID;
    (void) listSoundProperties;
    return E_NOT_USED;
}

am_Error_e RoutingSenderPULSE::asyncSetSourceSoundProperty(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const am_SoundProperty_s& soundProperty)
{
    (void) handle;
    (void) sourceID;
    (void) soundProperty;
    return E_NOT_USED;
}

am_Error_e RoutingSenderPULSE::asyncCrossFade(
        const am_Handle_s handle,
        const am_crossfaderID_t crossfaderID,
        const am_HotSink_e hotSink,
        const am_CustomRampType_t rampType,
        const am_time_t time)
{
    (void) handle;
    (void) crossfaderID;
    (void) hotSink;
    (void) rampType;
    (void) time;
    return E_NOT_USED;
}

am_Error_e RoutingSenderPULSE::setDomainState(
        const am_domainID_t domainID,
        const am_DomainState_e domainState)
{
    (void) domainID;
    (void) domainState;
    return E_NOT_USED;
}

am_Error_e RoutingSenderPULSE::returnBusName(string& BusName) const {
    BusName = "PulseRoutingPlugin";
    return E_OK;
}

void RoutingSenderPULSE::getInterfaceVersion(string& out_ver) const

{
    out_ver = RoutingVersion;
}

am_Error_e RoutingSenderPULSE::asyncSetVolumes(const am_Handle_s handle, const vector<am_Volumes_s>& listVolumes)
{
    (void) handle;
    (void) listVolumes;
    //todo: implement asyncSetVolumes;
    return (E_NOT_USED);
}

am_Error_e RoutingSenderPULSE::asyncSetSinkNotificationConfiguration(const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) handle;
    (void) sinkID;
    (void) notificationConfiguration;
    //todo: implement asyncSetSinkNotificationConfiguration;
    return (E_NOT_USED);
}

am_Error_e RoutingSenderPULSE::asyncSetSourceNotificationConfiguration(const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) handle;
    (void) sourceID;
    (void) notificationConfiguration;
    //todo: implement asyncSetSourceNotificationConfiguration;
    return (E_NOT_USED);
}

am_Error_e RoutingSenderPULSE::resyncConnectionState(const am_domainID_t domainID, vector<am_Connection_s>& listOfExistingConnections)
{
    return E_OK;
}

/*******************************************************************************
 * Private methods
 ******************************************************************************/


void RoutingSenderPULSE::getSinkInputInfoCallback(pa_context *ctx, const pa_sink_input_info *info, void *userdata)
{
    if (info == NULL)
    {
        return;
    }

    for (auto source : m_sources)
    {
        //try to match source PulseAudio properties against config properties
        const char *property_value = pa_proplist_gets(info->proplist, source.propertyName.c_str());

        if (property_value &&
            (string::npos != string(property_value).find(source.propertyValue) ||
             string::npos != source.propertyValue.find(property_value)))
        {
            logInfo("PULSE - sink input registered:", " sinkInputIndex:", info->index, "sourceID:", source.id);
            logInfo("PULSE - sink input details:", " prop_val: ", property_value, " source.prop_val: ", source.propertyValue);

            m_sourceToPASinkInput[source.id] = info->index;

            //iterate pending connection request
            // -> if there is a connection pending such that sink input "i" matches source from Connect() - create the connection in pulse

            for (auto connection  : m_activeConnections)
            {
                if (connection.sourceID == source.id)
                {
                    logInfo("PULSE - asyncConnect() - connectionID:", connection.connectionID,
                            "move sinkInputIndex:", m_sourceToPASinkInput[connection.sourceID], "to sinkIndex:", m_sinkToPASink[connection.sinkID]);

                    routing_sender_move_sink_input(m_paContext,
                            m_sourceToPASinkInput[connection.sourceID],
                            m_sinkToPASink[connection.sinkID],
                            this);
                }
            }

            //check of controller already requested vol adjustment  for this source
            bool requiresVolUpdate = false;
            for (int j = 0; j < info->volume.channels; j++)
            {
                if (info->volume.values[j] != m_sourceToVolume[source.id])
                {
                    requiresVolUpdate = true;
                    logInfo("PULSE - sink registerd with vol:", info->volume.values[j],
                            "; should be changed to:",
                            m_sourceToVolume[source.id]);
                    break;
                }
            }
            if (requiresVolUpdate)
            {
                routing_sender_sink_input_volume(
                        m_paContext,
                        m_sourceToPASinkInput[source.id],
                        m_sourceToVolume[source.id],
                        this);
            }
            //TODO: check mute state was requested by controller.
            break;
        }
    }
}

void RoutingSenderPULSE::getSourceOutputInfoCallback(pa_context *ctx, const pa_source_output_info *info, void *userdata)
{
    if (info == NULL)
    {
        return;
    }

    for (auto sink : m_sinks)
    {
        //try to match source PulseAudio properties agains config properties
        const char *property_value = pa_proplist_gets(info->proplist, sink.propertyName.c_str());

        if (property_value &&
            (string::npos != string(property_value).find(sink.propertyValue) ||
             string::npos != sink.propertyValue.find(property_value)) )
        {
            logInfo("PULSE - source output registered:", " sourceOutputIndex:", info->index, "sinkID:", sink.id);

            m_sinkToPASourceOutput[sink.id] = info->index;

            //iterate pending connection request
            // -> if there is a connection pending such that sink input "i" matches source from Connect() - create the connection in pulse
            for (auto connection : m_activeConnections)
            {
                if (connection.sinkID == sink.id)
                {
                    logInfo("PULSE - asyncConnect() - connectionID:", connection.connectionID,
                            "move sourceOutputIndex:", m_sinkToPASourceOutput[connection.sinkID], "to sourceIndex:", m_sourceToPASource[connection.sourceID]);

                    routing_sender_move_source_output(
                            m_paContext,
                            m_sinkToPASourceOutput[connection.sinkID],
                            m_sourceToPASource[connection.sourceID],
                            this);
                }
            }

            break;
        }
    }
}

void RoutingSenderPULSE::getSinkInfoCallback(pa_context *ctx, const pa_sink_info *info, int isLast, void *userdata)
{
    if (info != NULL)
    {
        if (strcmp("null", info->name) == 0)
        {
            m_paSinkNullIndex = info->index;
        }

        for (auto sink : m_sinks)
        {
            if (sink.deviceName == string(info->name))
            {
                logInfo("PULSE - PA sink:", info->index, "corresponding to AMGR sink:", sink.id, " - found");

                m_sinkToPASink[sink.id] = info->index;
            }
            else
            {
                //try to match sink PulseAudio properties against config properties
                const char *property_value = pa_proplist_gets(info->proplist, sink.propertyName.c_str());

                if (!property_value)
                {
                    continue;
                }

                if (string::npos != sink.propertyValue.find(property_value))
                {
                    logInfo("PULSE - PA sink:", info->index, "corresponding to AMGR sink:", sink.id, " - found");

                    m_sinkToPASink[sink.id] = info->index;
                }
            }
        }
    }
    else if (isLast)
    {
        routing_sender_get_source_info(m_paContext, this);
        logInfo("PULSE - PA sinks registration completed");
    }
}

void RoutingSenderPULSE::getSourceInfoCallback(pa_context *ctx, const pa_source_info *info, int isLast, void *userdata)
{
    if (info != NULL)
    {
        if (strcmp("null", info->name) == 0)
        {
            m_paSourceNullIndex = info->index;
        }

        //search for corresponding (already registered) Source
        for (auto source : m_sources)
        {
            //first try to match the sink name from pulse audio sink name
            if (source.name == string(info->name))
            {
                logInfo("PULSE - PA source:", info->index, "corresponding to AMGR source:", source.id, " - found");

                m_sourceToPASource[source.id] = info->index;
            }
            else
            {
                //try to match source PulseAudio properties against config properties
                const char *property_value = pa_proplist_gets(info->proplist, source.propertyName.c_str());

                if (!property_value)
                {
                    continue;
                }

                if (string::npos != source.propertyValue.find(property_value))
                {
                    logInfo("PULSE - PA source:", info->index, "corresponding to AMGR source:", source.id, " - found");

                    m_sourceToPASource[source.id] = info->index;
                }
            }
        }
    }
    else if (isLast)
    {
        m_shadow->hookDomainRegistrationComplete(m_domain.id);

        logInfo("PULSE - PA sinks and source registration completed");
        //TODO: - search for existing sink inputs & sources outputs
    }
}

void RoutingSenderPULSE::loadConfig()
{
    rp_Configuration_s config;
    CAmXmlConfigParser xmlConfigParser;

    xmlConfigParser.parse(config);

    registerDomain(config.domain);

    for(auto source : config.listSources)
    {
        registerSource(source);
    }

    for(auto sink : config.listSinks)
    {
        registerSink(sink);
    }
}

void RoutingSenderPULSE::registerDomain(const rp_Domain_s& rp_domain)
{
    am_Domain_s l_amDomain;

    l_amDomain.name     = rp_domain.name;
    returnBusName(l_amDomain.busname);//set domain bus name = current interface bus name
    l_amDomain.nodename = rp_domain.nodeName;
    l_amDomain.early    = false;
    l_amDomain.complete = true;
    l_amDomain.state    = am::DS_CONTROLLED;
    l_amDomain.domainID = rp_domain.id;

    m_domain = rp_domain;

    m_shadow->registerDomain(l_amDomain, m_domain.id);
}

void RoutingSenderPULSE::registerSource(const rp_Source_s& rp_source)
{
    am_Source_s l_amSource {};
    am_sourceID_t l_sourceID = 0;

    l_amSource.sourceID = rp_source.id;
    l_amSource.name = rp_source.name;
    // Do we need it ?
    l_amSource.sourceState = SS_ON;
    l_amSource.domainID = m_domain.id;
    l_amSource.visible = true;
    l_amSource.volume = 0;
    l_amSource.listConnectionFormats.push_back(am::CF_GENIVI_STEREO);

    m_shadow->peekSourceClassID(rp_source.className, l_amSource.sourceClassID);

    if (m_shadow->registerSource(l_amSource, l_sourceID) == E_OK)
    {
        logInfo("PULSE - register source:", rp_source.name,
                "(", rp_source.propertyName , ", ", rp_source.propertyValue, ")");
        m_sources.push_back(rp_source);
        m_sources.back().id = l_sourceID;

        m_sourceToPASinkInput[l_sourceID] = -1;
        m_sourceToPASource[l_sourceID] = -1;
        m_sourceToVolume[l_sourceID] = pa_sw_volume_from_dB(0);

        logInfo("PULSE - register source:", rp_source.name,
                "(", rp_source.propertyName , ", ", rp_source.propertyValue, ")");
    }
}

void RoutingSenderPULSE::registerSink(const rp_Sink_s& rp_sink)
{
    am_Sink_s l_amSink {};
    am_sinkID_t l_sinkID = 0;

    am_SoundProperty_s l_spTreble;
    l_spTreble.type = SP_GENIVI_BASS;
    l_spTreble.value = 0;

    am_SoundProperty_s l_spMid;
    l_spMid.type = SP_GENIVI_MID;
    l_spMid.value = 0;

    am_SoundProperty_s l_spBass;
    l_spBass.type = SP_GENIVI_BASS;
    l_spBass.value = 0;

    l_amSink.sinkID = l_sinkID;
    l_amSink.name = rp_sink.name;
    l_amSink.muteState = am::MS_MUTED;
    l_amSink.domainID = m_domain.id;
    l_amSink.visible = true;

    l_amSink.listSoundProperties.push_back(l_spTreble);
    l_amSink.listSoundProperties.push_back(l_spMid);
    l_amSink.listSoundProperties.push_back(l_spBass);
    l_amSink.listConnectionFormats.push_back(am::CF_GENIVI_STEREO);

    m_shadow->peekSinkClassID(rp_sink.className, l_amSink.sinkClassID);
    if (m_shadow->registerSink(l_amSink, l_sinkID) == E_OK)
    {
        m_sinks.push_back(rp_sink);
        m_sinks.back().id = l_sinkID;

        m_sinkToPASourceOutput[l_sinkID] = -1;
        m_sinkToPASink[l_sinkID] = -1;

        logInfo("PULSE - register sink:", rp_sink.name,
                "(", rp_sink.propertyName , ", ", rp_sink.propertyValue, ")");
    }
}

}
