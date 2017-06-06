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

#include <algorithm>
#include <cstring>

#include "CAmDltWrapper.h"

#include "CAmRoutingSenderPulse.h"
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
    return (new CAmRoutingSenderPulse(nullptr));
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
CAmRoutingSenderPulse::CAmRoutingSenderPulse(pa_context *p_paContext)
{
    mPaSinkNullIndex = -1;
    mPaSourceNullIndex = -1;
    mPaContext = p_paContext;
}

/**
 * Destructor.
 */
CAmRoutingSenderPulse::~CAmRoutingSenderPulse()
{
    //TODO: Disconnect from pulse: quit main loop and free the context and stuff
}

/**
 * Connecting sender & receiver
 * @author Ionut Popa (ionut.popa@windriver.com)
 */
am_Error_e CAmRoutingSenderPulse::startupInterface(IAmRoutingReceive *pRoutingReceiver)
{
    assert(pRoutingReceiver);

    CAmSocketHandler *socketHandler;

    pRoutingReceiver->getSocketHandler(socketHandler);

    assert(socketHandler);

    mShadow = new IAmRoutingReceiverShadow(pRoutingReceiver, socketHandler);

    return E_OK;
}

void CAmRoutingSenderPulse::setRoutingReady(uint16_t handle)
{
    //TODO: do not register sinks with the same name
    loadConfig();

    logInfo("PULSE - routingInterfacesReady");

    mShadow->confirmRoutingReady(handle, E_OK);

    //register pulse sink & sources, sink inputs & source outputs - > start the main PA loop
    routing_sender_create_mainloop((void *) this);
}

void CAmRoutingSenderPulse::setRoutingRundown(uint16_t handle)
{
    mShadow->confirmRoutingRundown(handle, am::E_OK);
    //TODO: implement this
}

am_Error_e CAmRoutingSenderPulse::asyncAbort(const am_Handle_s handle)
{
    (void) handle;

    return E_NOT_USED;
}

am_Error_e CAmRoutingSenderPulse::asyncConnect(
        const am_Handle_s handle,
        const am_connectionID_t connectionID,
        const am_sourceID_t sourceID,
        const am_sinkID_t sinkID,
        const am_CustomConnectionFormat_t connectionFormat)
{
    //TODO: check stuff like connectionFormat
    logInfo("PULSE - asyncConnect");

    //add source,sink & connectionID to a list of connections maintained by Routing Pulse Engine
    RoutingConnection newConnection {};

    newConnection.sinkID = sinkID;
    newConnection.sourceID = sourceID;
    newConnection.connectionID = connectionID;

    if (mSinkToPASink[sinkID] != -1)
    {
        if (mSourceToPASinkInput[sourceID] != -1)
        {
            if (routing_sender_move_sink_input(mPaContext, mSourceToPASinkInput[sourceID], mSinkToPASink[sinkID], this))
            {
                //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation

                logInfo("PULSE - asyncConnect - connectionID:", connectionID,
                        "move sinkInputIndex:", mSourceToPASinkInput[sourceID], "to sinkIndex:", mSinkToPASink[sinkID]);
            }
            else
            {
                mShadow->ackConnect(handle, connectionID, am::E_NOT_POSSIBLE);

                return E_NOT_POSSIBLE;
            }
        }
        else
        {
            logInfo("PULSE - asyncConnect - connectionID:", connectionID, " move pending");

            newConnection.pending = true;
        }
        //else move_sink_input will be called later
    }
    else if (mSourceToPASource[sourceID] != -1)
    {
        if (mSinkToPASourceOutput[sinkID] != -1)
        {
            if (routing_sender_move_source_output(
                    mPaContext,
                    mSinkToPASourceOutput[sinkID],
                    mSourceToPASource[sourceID],
                    this))
            {
                logInfo("PULSE - asyncConnect() - connectionID:", connectionID,
                        "move sourceOutputIndex:", mSinkToPASourceOutput[sinkID], "to sourceIndex:", mSourceToPASource[sourceID]);
            }
            else
            {
                mShadow->ackConnect(handle, connectionID, am::E_NOT_POSSIBLE);

                return am::E_NOT_POSSIBLE;
            }
        }
        else
        {
            newConnection.pending = true;
        }
        //else move_sink_input will be called later
    }
    else
    {
        logError("Sink and source for connection not identified:",
                sinkID, sourceID, connectionID);

        mShadow->ackConnect(handle, connectionID, am::E_NOT_POSSIBLE);

        return am::E_NOT_POSSIBLE;
    }

    mConnections.push_back(newConnection);

    mShadow->ackConnect(handle, connectionID, am::E_OK);

/**
 * TODO: connection is always possible ? check that
*/

    return am::E_OK;
}


am_Error_e CAmRoutingSenderPulse::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    //get connection by ID ... not to many connections, therefore linear search is fast enough
    vector<RoutingConnection>::iterator iter    = mConnections.begin();
    vector<RoutingConnection>::iterator iterEnd = mConnections.end();

    for (; iter < iterEnd; ++iter)
    {
        if (iter->connectionID == connectionID)
        {
            if (mSourceToPASinkInput[iter->sourceID] != -1)
            {
                if (mPaSinkNullIndex != -1)
                {
                    //if null sink is defined - disconnect = move sink input to null
                    logInfo("PULSE - asyncDisconnect() - connection found - move sinkInputIndex:",
                            mSourceToPASinkInput[iter->sourceID], "to NULL sinkIndex:", mPaSinkNullIndex);

                    routing_sender_move_sink_input(
                            mPaContext,
                            mSourceToPASinkInput[iter->sourceID],
                            mPaSinkNullIndex,
                            this);
                    //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation
                    mShadow->ackDisconnect(handle, connectionID, am::E_OK);

                }
            }
            else if (mSinkToPASourceOutput[iter->sinkID] != -1)
            {
                if (mPaSourceNullIndex != -1)
                {
                    //if null source is defined - disconnect = move source output to null
                    logInfo("PULSE - asyncDisconnect() - connection found - move sourceOutputIndex:",
                            mSinkToPASourceOutput[iter->sinkID], "to NULL sourceIndex:", mPaSourceNullIndex);

                    routing_sender_move_source_output(
                            mPaContext,
                            mSourceToPASinkInput[iter->sourceID],
                            mPaSinkNullIndex,
                            this);
                    //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation
                    mShadow->ackDisconnect(handle, connectionID, am::E_OK);

                    //remove connection from the list of active connections
                    iter = mConnections.erase(iter);

                    break;
                }
            }
            else
            {
                logInfo("PULSE - asyncDisconnect() - connection found - but no sink input or source");
                mShadow->ackDisconnect(handle, connectionID, am::E_OK);
            }
            //remove connection from the list of active connections
            iter = mConnections.erase(iter);

            break;
        }
    }
    return am::E_OK;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSinkVolume(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const am_volume_t volume,
        const am_CustomRampType_t ramp,
        const am_time_t time)
{
    (void) ramp;
    (void) time;

    logInfo("PULSE - asyncSetSinkVolume() - volume:", volume, "sink index:", mSinkToPASink[sinkID]);

    routing_sender_sink_volume(mPaContext, mSinkToPASink[sinkID], pa_sw_volume_from_dB(volume*0.1), this);
    mShadow->ackSetSinkVolumeChange(handle, volume, E_OK);

    return E_OK;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSourceVolume(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const am_volume_t volume,
        const am_CustomRampType_t ramp,
        const am_time_t time)
{
    (void) ramp;
    (void) time;

    pa_volume_t crt_volume = mSourceToVolume[sourceID];
    mSourceToVolume[sourceID] = pa_sw_volume_from_dB(volume*0.1);

    logInfo("PULSE - asyncSetSourceVolume() - volume:", volume, "sink input index:", mSourceToPASinkInput[sourceID]);
    if (mSourceToPASinkInput[sourceID] != -1)
    {
#if 0
        if (time == 0)
        {/* without ramp time */
#endif
            routing_sender_sink_input_volume(
                    mPaContext,
                    mSourceToPASinkInput[sourceID],
                    mSourceToVolume[sourceID],
                    this);
#if 0
        }
        else
        {/* with ramp time */
            routing_sender_sink_input_volume_ramp(
                    mPaContext,
                    mSourceToPASinkInput[sourceID],
                    crt_volume,
                    mSourceToVolume[sourceID],
                    (uint16_t)time,
                    this);
        }
#endif
    }
    else
    {
        logInfo("PULSE - sink input not registered yet - should wait for registration before update the volume");
    }
    mShadow->ackSetSourceVolumeChange(handle, volume, E_OK);

}

am_Error_e CAmRoutingSenderPulse::asyncSetSourceState(
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
                    mPaContext,
                    mSourceToPASinkInput[sourceID],
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
                    mPaContext,
                    mSourceToPASinkInput[sourceID],
                    true,
                    this
            );
            break;
        }
        default:
        {
            logError("RoutingSenderPULSE::asyncSetSourceState - wrong source state\n");
            mShadow->ackSetSourceState(handle, E_NOT_POSSIBLE);
            return E_NOT_POSSIBLE;
        }
    }

    mShadow->ackSetSourceState(handle, E_OK);

    return E_OK;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSinkSoundProperties(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const vector<am_SoundProperty_s>& listSoundProperties)
{
    (void) handle;
    (void) sinkID;
    (void) listSoundProperties;

    return E_NOT_USED;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSinkSoundProperty(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const am_SoundProperty_s& soundProperty)
{
    (void) handle;
    (void) sinkID;
    (void) soundProperty;

    return E_NOT_USED;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSourceSoundProperties(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const vector<am_SoundProperty_s>& listSoundProperties)
{
    (void) handle;
    (void) sourceID;
    (void) listSoundProperties;

    return E_NOT_USED;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSourceSoundProperty(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const am_SoundProperty_s& soundProperty)
{
    (void) handle;
    (void) sourceID;
    (void) soundProperty;

    return E_NOT_USED;
}

am_Error_e CAmRoutingSenderPulse::asyncCrossFade(
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

am_Error_e CAmRoutingSenderPulse::setDomainState(
        const am_domainID_t domainID,
        const am_DomainState_e domainState)
{
    (void) domainID;
    (void) domainState;

    return E_NOT_USED;
}

am_Error_e CAmRoutingSenderPulse::returnBusName(string& BusName) const
{

    BusName = "PulseRoutingPlugin";

    return E_OK;
}

void CAmRoutingSenderPulse::getInterfaceVersion(string& version) const

{
    version = RoutingVersion;
}

am_Error_e CAmRoutingSenderPulse::asyncSetVolumes(const am_Handle_s handle, const vector<am_Volumes_s>& listVolumes)
{
    (void) handle;
    (void) listVolumes;
    //todo: implement asyncSetVolumes;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderPulse::asyncSetSinkNotificationConfiguration(const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) handle;
    (void) sinkID;
    (void) notificationConfiguration;
    //todo: implement asyncSetSinkNotificationConfiguration;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderPulse::asyncSetSourceNotificationConfiguration(const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) handle;
    (void) sourceID;
    (void) notificationConfiguration;
    //todo: implement asyncSetSourceNotificationConfiguration;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderPulse::resyncConnectionState(const am_domainID_t domainID, vector<am_Connection_s>& listOfExistingConnections)
{
    return E_OK;
}

/*******************************************************************************
 * Private methods
 ******************************************************************************/


void CAmRoutingSenderPulse::getSinkInputInfoCallback(pa_context *ctx, const pa_sink_input_info *info, void *userdata)
{
    if (info == NULL)
    {
        return;
    }

    auto sourceIt = find_if(mSources.begin(), mSources.end(), [info, this](const rp_Source_s& source)
                            { return cmpProperty(info->proplist, source.propertyName, source.propertyValue); });

    if (sourceIt != mSources.end())
    {
        logInfo("PULSE - sink input registered:", " sinkInputIndex:", info->index, "sourceID:", sourceIt->id);

        mSourceToPASinkInput[sourceIt->id] = info->index;

        //iterate pending connection request
        // -> if there is a connection pending such that sink input "i" matches source from Connect() - create the connection in pulse

        for (auto connection  : mConnections)
        {
            if (connection.sourceID == sourceIt->id)
            {
                logInfo("PULSE - asyncConnect - connectionID:", connection.connectionID,
                        "move sinkInputIndex:", mSourceToPASinkInput[connection.sourceID], "to sinkIndex:", mSinkToPASink[connection.sinkID]);

                routing_sender_move_sink_input(mPaContext,
                        mSourceToPASinkInput[connection.sourceID],
                        mSinkToPASink[connection.sinkID],
                        this);
            }
        }

        checkSourceVolume(info->volume, sourceIt->id);

        //TODO: check mute state was requested by controller.
    }
}

void CAmRoutingSenderPulse::getSourceOutputInfoCallback(pa_context *ctx, const pa_source_output_info *info, void *userdata)
{
    if (info == NULL)
    {
        return;
    }

    auto sinkIt = find_if(mSinks.begin(), mSinks.end(), [info, this](const rp_Sink_s& sink)
                            { return cmpProperty(info->proplist, sink.propertyName, sink.propertyValue); });

    if (sinkIt != mSinks.end())
    {
        logInfo("PULSE - source output registered:", " sourceOutputIndex:", info->index, "sinkID:", sinkIt->id);

        mSinkToPASourceOutput[sinkIt->id] = info->index;

        //iterate pending connection request
        // -> if there is a connection pending such that sink input "i" matches source from Connect() - create the connection in pulse
        for (auto connection : mConnections)
        {
            if (connection.sinkID == sinkIt->id)
            {
                logInfo("PULSE - asyncConnect() - connectionID:", connection.connectionID,
                        "move sourceOutputIndex:", mSinkToPASourceOutput[connection.sinkID], "to sourceIndex:", mSourceToPASource[connection.sourceID]);

                routing_sender_move_source_output(
                        mPaContext,
                        mSinkToPASourceOutput[connection.sinkID],
                        mSourceToPASource[connection.sourceID],
                        this);
            }
        }
    }
}

void CAmRoutingSenderPulse::getSinkInfoCallback(pa_context *ctx, const pa_sink_info *info, int isLast, void *userdata)
{
    if (info != NULL)
    {
        if (strcmp("null", info->name) == 0)
        {
            mPaSinkNullIndex = info->index;
        }

        for (auto sink : mSinks)
        {
            if (sink.deviceName == string(info->name))
            {
                logInfo("PULSE - PA sink:", info->index, "corresponding to AMGR sink:", sink.id, " - found");

                mSinkToPASink[sink.id] = info->index;
            }
            else
            {
                //try to match sink PulseAudio properties against config properties
                if (cmpProperty(info->proplist, sink.propertyName, sink.propertyValue))
                {
                    logInfo("PULSE - PA sink:", info->index, "corresponding to AMGR sink:", sink.id, " - found");

                    mSinkToPASink[sink.id] = info->index;
                }
            }
        }
    }
    else if (isLast)
    {
        routing_sender_get_source_info(mPaContext, this);
        logInfo("PULSE - PA sinks registration completed");
    }
}

void CAmRoutingSenderPulse::getSourceInfoCallback(pa_context *ctx, const pa_source_info *info, int isLast, void *userdata)
{
    if (info != NULL)
    {
        if (strcmp("null", info->name) == 0)
        {
            mPaSourceNullIndex = info->index;
        }

        //search for corresponding (already registered) Source
        for (auto source : mSources)
        {
            //first try to match the sink name from pulse audio sink name
            if (source.name == string(info->name))
            {
                logInfo("PULSE - PA source:", info->index, "corresponding to AMGR source:", source.id, " - found");

                mSourceToPASource[source.id] = info->index;
            }
            else
            {
                //try to match source PulseAudio properties against config properties
                if (cmpProperty(info->proplist, source.propertyName, source.propertyValue))
                {
                    logInfo("PULSE - PA source:", info->index, "corresponding to AMGR source:", source.id, " - found");

                    mSourceToPASource[source.id] = info->index;
                }
            }
        }
    }
    else if (isLast)
    {
        mShadow->hookDomainRegistrationComplete(mDomain.id);

        logInfo("PULSE - PA sinks and source registration completed");
        //TODO: - search for existing sink inputs & sources outputs
    }
}

bool CAmRoutingSenderPulse::cmpProperty(pa_proplist *propList, const string& name, const string& value)
{
    //try to match source PulseAudio properties agains config properties
    const char *propertyValue = pa_proplist_gets(propList, name.c_str());

    if (propertyValue &&
        (string::npos != string(propertyValue).find(value) ||
         string::npos != value.find(propertyValue)))
    {
        logInfo("PULSE - matches property name:", name, " propValue: ", propertyValue, " value: ", value);

        return true;
    }

    return false;
}

void CAmRoutingSenderPulse::checkSourceVolume(pa_cvolume volume, am_sourceID_t sourceID)
{
    //check of controller already requested vol adjustment  for this source
    bool requiresVolUpdate = false;

    for (int i = 0; i < volume.channels; i++)
    {
        if (volume.values[i] != mSourceToVolume[sourceID])
        {
            requiresVolUpdate = true;

            logInfo("PULSE - sink registerd with vol:", volume.values[i],
                    "; should be changed to:", mSourceToVolume[sourceID]);

            break;
        }
    }

    if (requiresVolUpdate)
    {
        routing_sender_sink_input_volume(
                mPaContext,
                mSourceToPASinkInput[sourceID],
                mSourceToVolume[sourceID],
                this);
    }
}

void CAmRoutingSenderPulse::loadConfig()
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

void CAmRoutingSenderPulse::registerDomain(const rp_Domain_s& rpDomain)
{
    am_Domain_s amDomain;

    amDomain.name     = rpDomain.name;
    returnBusName(amDomain.busname);//set domain bus name = current interface bus name
    amDomain.nodename = rpDomain.nodeName;
    amDomain.early    = false;
    amDomain.complete = true;
    amDomain.state    = am::DS_CONTROLLED;
    amDomain.domainID = rpDomain.id;

    mDomain = rpDomain;

    mShadow->registerDomain(amDomain, mDomain.id);
}

void CAmRoutingSenderPulse::registerSource(const rp_Source_s& rpSource)
{
    am_Source_s amSource {};
    am_sourceID_t sourceID = 0;

    amSource.sourceID = rpSource.id;
    amSource.name = rpSource.name;
    // Do we need it ?
    amSource.sourceState = SS_ON;
    amSource.domainID = mDomain.id;
    amSource.visible = true;
    amSource.volume = 0;
    amSource.listConnectionFormats.push_back(am::CF_GENIVI_STEREO);

    mShadow->peekSourceClassID(rpSource.className, amSource.sourceClassID);

    if (mShadow->registerSource(amSource, sourceID) == E_OK)
    {
        logInfo("PULSE - register source:", rpSource.name,
                "(", rpSource.propertyName , ", ", rpSource.propertyValue, ")");
        mSources.push_back(rpSource);
        mSources.back().id = sourceID;

        mSourceToPASinkInput[sourceID] = -1;
        mSourceToPASource[sourceID] = -1;
        mSourceToVolume[sourceID] = pa_sw_volume_from_dB(0);

        logInfo("PULSE - register source:", rpSource.name,
                "(", rpSource.propertyName , ", ", rpSource.propertyValue, ")");
    }
}

void CAmRoutingSenderPulse::registerSink(const rp_Sink_s& rpSink)
{
    am_Sink_s amSink {};
    am_sinkID_t sinkID = 0;

    am_SoundProperty_s l_spTreble;
    l_spTreble.type = SP_GENIVI_BASS;
    l_spTreble.value = 0;

    am_SoundProperty_s l_spMid;
    l_spMid.type = SP_GENIVI_MID;
    l_spMid.value = 0;

    am_SoundProperty_s l_spBass;
    l_spBass.type = SP_GENIVI_BASS;
    l_spBass.value = 0;

    amSink.sinkID = sinkID;
    amSink.name = rpSink.name;
    amSink.muteState = am::MS_MUTED;
    amSink.domainID = mDomain.id;
    amSink.visible = true;

    amSink.listSoundProperties.push_back(l_spTreble);
    amSink.listSoundProperties.push_back(l_spMid);
    amSink.listSoundProperties.push_back(l_spBass);
    amSink.listConnectionFormats.push_back(am::CF_GENIVI_STEREO);

    mShadow->peekSinkClassID(rpSink.className, amSink.sinkClassID);
    if (mShadow->registerSink(amSink, sinkID) == E_OK)
    {
        mSinks.push_back(rpSink);
        mSinks.back().id = sinkID;

        mSinkToPASourceOutput[sinkID] = -1;
        mSinkToPASink[sinkID] = -1;

        logInfo("PULSE - register sink:", rpSink.name,
                "(", rpSink.propertyName , ", ", rpSink.propertyValue, ")");
    }
}

}
