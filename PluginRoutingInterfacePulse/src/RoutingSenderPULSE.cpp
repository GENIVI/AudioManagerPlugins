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

#include <stdio.h>
#include <string.h>
#include <iostream>

#include "CAmDltWrapper.h"

#include "RoutingSenderPULSE.h"
#include "RoutingSenderMainloopPULSE.h"



#define LIBNAME "libPluginRoutingInterfacePULSE.so"
#define CFGNAME "libPluginRoutingInterfacePULSE.conf"

/* Globals */


/* Defines */
DLT_DECLARE_CONTEXT(routingPulse)
/* Maximum source volume measured in percentage. Minimum value is 0% */
#define MAX_SOURCE_VOLUME   (100)


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
    this->m_paSinkNullIndex = -1;
    this->m_paSourceNullIndex = -1;
    this->m_paContext = p_paContext;
}


void RoutingSenderPULSE::loadConfig()
{
    //get current library path - search: /proc/< getpid() >/maps
    char proc_maps_file_name[256];
    char line[256];
    char lib_name[256];
    char *tmp;
    pid_t pid = getpid();
    snprintf(proc_maps_file_name, 256,  "/proc/%d/maps", pid);
    FILE *proc_maps = fopen(proc_maps_file_name, "r");

    while (!feof(proc_maps))
    {
        char *cnt = fgets(line, 256, proc_maps);
        if (strlen(line) == 0 || line[0] == '#')
        {
            continue;
        }
        if (cnt == NULL) continue;
        //tmp0 tmp1 tmp2 tmp3 tmp4 lib_name);
        tmp = strtok(line, " ");//address-interval
        if(tmp == NULL) continue;

        tmp = strtok(NULL, " ");//rights
        if(tmp == NULL) continue;

        tmp = strtok(NULL, " ");//offset
        if(tmp == NULL) continue;

        strtok(NULL, " ");//dev
        if(tmp == NULL) continue;

        tmp = strtok(NULL, " \n");//inode
        if(tmp == NULL) continue;

        tmp = strtok(NULL, " \n");
        if(tmp == NULL) continue;

        strcpy(lib_name, tmp);
        if ((lib_name != NULL) && (strstr(lib_name, LIBNAME) >= lib_name))
        {
            strcpy(strrchr(lib_name, '/') + 1, CFGNAME);
            logInfo("PULSE - config file name: %s\n", lib_name);

            FILE *config = fopen(lib_name, "r");

            while (config && !feof(config))
            {
                char *cnt = fgets(line, 256, config);
                if (!line || strlen(line) == 0) continue;
                //config format line: TYPE|PULSE TYPE|NAME|CLASS|PROPERTY_NAME|PROPERTY_VALUE
                //TYPE="Source" or "Sink"

                char *tmp = strtok(line, "|");//type
                if (strcmp("Sink", tmp) == 0)
                {
                    //add sink config
                    RoutingSenderPULSESourceSinkConfig sinkConfig;

                    tmp = strtok(NULL, "|");//pulse type - not used for the moment

                    tmp = strtok(NULL, "|");//class
                    sinkConfig.clazz = std::string(tmp);

                    tmp = strtok(NULL, "|");//name
                    sinkConfig.name = std::string(tmp);

                    tmp = strtok(NULL, "|");//property name
                    sinkConfig.propertyName = std::string(tmp);

                    tmp = strtok(NULL, "|\n");//property value
                    sinkConfig.propertyValue = std::string(tmp);

                    m_sinks.push_back(sinkConfig);
                    logInfo("sinkConfig: sinkConfig.clazz=", sinkConfig.clazz, " sinkConfig.name=", sinkConfig.name, " sinkConfig.propertyName=", sinkConfig.propertyName, " sinkConfig.propertyValue=", sinkConfig.propertyValue);
                }
                if (strcmp("Source", tmp) == 0)
                {
                    //add source config
                    RoutingSenderPULSESourceSinkConfig sourceConfig;

                    tmp = strtok(NULL, "|");//pulse type - not used for the moment

                    tmp = strtok(NULL, "|");//class
                    sourceConfig.clazz = std::string(tmp);

                    tmp = strtok(NULL, "|");//name
                    sourceConfig.name = std::string(tmp);

                    tmp = strtok(NULL, "|");//property name
                    sourceConfig.propertyName = std::string(tmp);

                    tmp = strtok(NULL, "|\n");//property value
                    sourceConfig.propertyValue = std::string(tmp);

                    m_sources.push_back(sourceConfig);
                    logInfo("sourceConfig: sourceConfig.clazz=", sourceConfig.clazz, " sourceConfig.name=", sourceConfig.name, " sourceConfig.propertyName=", sourceConfig.propertyName, " sourceConfig.propertyValue=", sourceConfig.propertyValue);
                }
            }

            if (config)
                fclose(config);
            break;
        }
    }

    fclose(proc_maps);
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
    this->m_routingReceiver = p_routingReceiver;
    return am::E_OK;
}

void RoutingSenderPULSE::setRoutingReady(uint16_t handle)
{
    //TODO: do not register sinks with the same name

    int i;
    this->loadConfig();
    //first register Domain = PulseAudio
    this->m_domain.name     = "PulseAudio";
    this->returnBusName(this->m_domain.busname);//set domain bus name = current interface bus name
    this->m_domain.nodename = "PulseAudio";
    this->m_domain.early    = false;
    this->m_domain.complete = true;
    this->m_domain.state    = am::DS_CONTROLLED;

    this->m_domain.domainID = 0;
    this->m_routingReceiver->registerDomain(this->m_domain, this->m_domain.domainID);

    am_SoundProperty_s l_spTreble;
    l_spTreble.type = SP_GENIVI_BASS;
    l_spTreble.value = 0;

    am_SoundProperty_s l_spMid;
    l_spMid.type = SP_GENIVI_MID;
    l_spMid.value = 0;

    am_SoundProperty_s l_spBass;
    l_spBass.type = SP_GENIVI_BASS;
    l_spBass.value = 0;

    //register sources (sink inputs & sinks)
    for (i = 0; i < m_sources.size(); i++)
    {
        am_sourceID_t l_newSourceID = 0;
        this->m_sources[i].source.sourceID = l_newSourceID;
        this->m_sources[i].source.name = m_sources[i].name;
        this->m_sources[i].source.sourceState = am::SS_ON;
        this->m_sources[i].source.domainID = this->m_domain.domainID;
        this->m_sources[i].source.visible = true;
        this->m_sources[i].source.volume = MAX_SOURCE_VOLUME; /* initialize source volume to 100% */

        this->m_sources[i].source.listConnectionFormats.push_back(am::CF_GENIVI_STEREO);
        this->m_routingReceiver->peekSourceClassID(
                this->m_sources[i].clazz,
                this->m_sources[i].source.sourceClassID);

        this->m_routingReceiver->registerSource(this->m_sources[i].source, l_newSourceID);

        this->m_sources[i].source.sourceID = l_newSourceID;
        m_sourceToPASinkInput[l_newSourceID] = -1;
        m_sourceToPASource[l_newSourceID] = -1;

        logInfo("PULSE - register source:"
            ,m_sources[i].name
            , "(", m_sources[i].propertyName , ", ", m_sources[i].propertyValue, ")");
        m_sourceToVolume[l_newSourceID] = MAX_SOURCE_VOLUME;//initially all the sources are at 100%
    }

    //register sinks (source outputs & sources)
    for (i = 0; i < m_sinks.size(); i++)
    {
        am_sinkID_t l_newsinkID = 0;
        this->m_sinks[i].sink.sinkID = l_newsinkID;
        this->m_sinks[i].sink.name = this->m_sinks[i].name;
        this->m_sinks[i].sink.muteState = am::MS_MUTED;
        this->m_sinks[i].sink.domainID = this->m_domain.domainID;
        this->m_sinks[i].sink.visible = true;

        this->m_sinks[i].sink.listSoundProperties.push_back(l_spTreble);
        this->m_sinks[i].sink.listSoundProperties.push_back(l_spMid);
        this->m_sinks[i].sink.listSoundProperties.push_back(l_spBass);
        this->m_sinks[i].sink.listConnectionFormats.push_back(am::CF_GENIVI_STEREO);

        this->m_routingReceiver->peekSinkClassID(
                this->m_sinks[i].clazz,
                this->m_sinks[i].sink.sinkClassID);
        this->m_routingReceiver->registerSink(this->m_sinks[i].sink, l_newsinkID);
        this->m_sinks[i].sink.sinkID = l_newsinkID;
        m_sinkToPASourceOutput[l_newsinkID] = -1;
        m_sinkToPASink[l_newsinkID] = -1;

        logInfo("PULSE - register sink:"
            ,m_sinks[i].name
            , "(", m_sinks[i].propertyName , ", ", m_sinks[i].propertyValue, ")");
    }

    logInfo("PULSE - routingInterfacesReady");
    this->m_routingReceiver->confirmRoutingReady(handle, am::E_OK);

    //register pulse sink & sources, sink inputs & source outputs - > start the main PA loop
    routing_sender_create_mainloop((void *) this);
}

void RoutingSenderPULSE::setRoutingRundown(uint16_t handle)
{
    this->m_routingReceiver->confirmRoutingRundown(handle, am::E_OK);
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
    RoutingSenderPULSEConnection l_newConnection;
    l_newConnection.sinkID = sinkID;
    l_newConnection.sourceID = sourceID;
    l_newConnection.connectionID = connectionID;
    l_newConnection.handle = handle;

    //by default - sources ar connected at 100% -> controller is responsible to setSourcevolume if needed

    m_sourceToVolume[sourceID] = MAX_SOURCE_VOLUME;


    if (m_sinkToPASink[sinkID] != -1)
    {
        if (m_sourceToPASinkInput[sourceID] != -1)
        {
            if (routing_sender_move_sink_input(
                    this->m_paContext,
                    m_sourceToPASinkInput[sourceID],
                    m_sinkToPASink[sinkID],
                    this))
            {
                //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation

                logInfo("PULSE - asyncConnect() - connectionID:", connectionID,
                        "move sinkInputIndex:", m_sourceToPASinkInput[sourceID], "to sinkIndex:", m_sinkToPASink[sinkID]);
            }
            else
            {
                this->m_routingReceiver->ackConnect(handle, connectionID, am::E_NOT_POSSIBLE);
                return am::E_NOT_POSSIBLE;
            }
        }//else move_sink_input will be called later
    }
    else if (m_sourceToPASource[sourceID] != -1)
    {
        if (m_sinkToPASourceOutput[sinkID] != -1)
        {
            if (routing_sender_move_source_output(
                    this->m_paContext,
                    m_sinkToPASourceOutput[sinkID],
                    m_sourceToPASource[sourceID],
                    this))
            {
                //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation

                logInfo("PULSE - asyncConnect() - connectionID:", connectionID,
                        "move sourceOutputIndex:", m_sinkToPASourceOutput[sinkID], "to sourceIndex:", m_sourceToPASource[sourceID]);

            }
            else
            {
                this->m_routingReceiver->ackConnect(handle, connectionID, am::E_NOT_POSSIBLE);
                return am::E_NOT_POSSIBLE;
            }
        }//else move_sink_input will be called later
    }
    else
    {
        logError("Sink and source for connection not identified:",
                sinkID, sourceID, connectionID);
        return am::E_NOT_POSSIBLE;
    }

    m_activeConnections.push_back(l_newConnection);

    this->m_routingReceiver->ackConnect(handle, connectionID, am::E_OK);

/**
 * TODO: connection is always possible ? check that
*/

    return am::E_OK;
}


am_Error_e RoutingSenderPULSE::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    //get connection by ID ... not to many connections, therefore linear search is fast enough
    std::vector<RoutingSenderPULSEConnection>::iterator iter    = m_activeConnections.begin();
    std::vector<RoutingSenderPULSEConnection>::iterator iterEnd = m_activeConnections.end();
    for (; iter < iterEnd; ++iter)
    {
        if (iter->connectionID == connectionID)
        {
            if (m_sourceToPASinkInput[iter->sourceID] != -1)
            {
                if (this->m_paSinkNullIndex >= 0)
                {
                    //if null sink is defined - disconnect = move sink input to null
                    logInfo("PULSE - asyncDisconnect() - connection found - move sinkInputIndex:",
                            m_sourceToPASinkInput[iter->sourceID], "to NULL sinkIndex:", this->m_paSinkNullIndex);

                    routing_sender_move_sink_input(
                            this->m_paContext,
                            this->m_sourceToPASinkInput[iter->sourceID],
                            this->m_paSinkNullIndex,
                            this);
                    //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation
                    this->m_routingReceiver->ackDisconnect(handle, connectionID, am::E_OK);

                }
            }
            else if (m_sinkToPASourceOutput[iter->sinkID] != -1)
            {
                if (this->m_paSourceNullIndex >= 0)
                {
                    //if null source is defined - disconnect = move source output to null
                    logInfo("PULSE - asyncDisconnect() - connection found - move sourceOutputIndex:",
                            m_sinkToPASourceOutput[iter->sinkID], "to NULL sourceIndex:", this->m_paSourceNullIndex);

                    routing_sender_move_source_output(
                            this->m_paContext,
                            m_sourceToPASinkInput[iter->sourceID],
                            this->m_paSinkNullIndex,
                            this);
                    //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation
                    this->m_routingReceiver->ackDisconnect(handle, connectionID, am::E_OK);

                    //remove connection from the list of active connections
                    iter = m_activeConnections.erase(iter);

                    break;
                }
            }
            else
            {
                logInfo("PULSE - asyncDisconnect() - connection found - but no sink input or source");
                this->m_routingReceiver->ackDisconnect(handle, connectionID, am::E_OK);
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

    logInfo("PULSE - asyncSetSinkVolume() - volume:", volume, "sink index:", this->m_sinkToPASink[sinkID]);

    routing_sender_sink_volume(
            this->m_paContext,
            this->m_sinkToPASink[sinkID],
            volume,
            this);
    this->m_routingReceiver->ackSetSinkVolumeChange(handle, volume, E_OK);
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

    am_volume_t crt_volume = this->m_sourceToVolume[sourceID];
    this->m_sourceToVolume[sourceID] = volume;

    logInfo("PULSE - asyncSetSourceVolume() - volume:", volume, "sink input index:", this->m_sourceToPASinkInput[sourceID]);
    if (m_sourceToPASinkInput[sourceID] != -1)
    {
        if (time == 0)
        {/* without ramp time */
            routing_sender_sink_input_volume(
                    this->m_paContext,
                    this->m_sourceToPASinkInput[sourceID],
                    volume,
                    this);
        }
        else
        {/* with ramp time */
            routing_sender_sink_input_volume_ramp(
                    this->m_paContext,
                    this->m_sourceToPASinkInput[sourceID],
                    crt_volume,
                    volume,
                    (uint16_t)time,
                    this);
        }
    }
    else
    {
        logInfo("PULSE - sink input not registered yet - should wait for registration before update the volume");
    }
    this->m_routingReceiver->ackSetSourceVolumeChange(handle, volume, E_OK);

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
                    this->m_paContext,
                    this->m_sourceToPASinkInput[sourceID],
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
                    this->m_paContext,
                    this->m_sourceToPASinkInput[sourceID],
                    true,
                    this
            );
            break;
        }
        default:
        {
            logError("RoutingSenderPULSE::asyncSetSourceState - wrong source state\n");
            this->m_routingReceiver->ackSetSourceState(handle, E_NOT_POSSIBLE);
            return E_NOT_POSSIBLE;
        }
    }
    this->m_routingReceiver->ackSetSourceState(handle, E_OK);
    return E_OK;
}

am_Error_e RoutingSenderPULSE::asyncSetSinkSoundProperties(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const std::vector<am_SoundProperty_s>& listSoundProperties)
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
        const std::vector<am_SoundProperty_s>& listSoundProperties)
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

am_Error_e RoutingSenderPULSE::returnBusName(std::string& BusName) const {
    BusName = "RoutingPULSE";
    return E_OK;
}

void RoutingSenderPULSE::getInterfaceVersion(std::string& out_ver) const
{
    out_ver = RoutingVersion;
}

am_Error_e RoutingSenderPULSE::asyncSetVolumes(const am_Handle_s handle, const std::vector<am_Volumes_s>& listVolumes)
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

am_Error_e RoutingSenderPULSE::resyncConnectionState(const am_domainID_t domainID, std::vector<am_Connection_s>& listOfExistingConnections)
{
    return E_OK;
}

/*******************************************************************************
 * Private methods
 ******************************************************************************/


void RoutingSenderPULSE::getSinkInputInfoCallback(pa_context *c, const pa_sink_input_info *i, void *userdata)
{
    if (i == NULL)
    {
        return;
    }

    //search for corresponding Source
    std::vector<RoutingSenderPULSESourceSinkConfig>::iterator iter    = m_sources.begin();
    std::vector<RoutingSenderPULSESourceSinkConfig>::iterator iterEnd = m_sources.end();
    for (; iter < iterEnd; ++iter)
    {
        //try to match source PulseAudio properties against config properties
        const char *property_value = pa_proplist_gets(i->proplist, iter->propertyName.c_str());

        if (property_value &&
            ( std::string::npos != std::string(property_value).find(iter->propertyValue) ||
              std::string::npos != iter->propertyValue.find(property_value)) )
        {
            logInfo("PULSE - sink input registered:"
                , " sinkInputIndex:", i->index, "sourceID:", iter->source.sourceID);

            logInfo("PULSE - sink input details:"
                    , " prop_val: ", property_value, " iter->prop_val: ", iter->propertyValue);

            m_sourceToPASinkInput[iter->source.sourceID] = i->index;

            //iterate pending connection request
            // -> if there is a connection pending such that sink input "i" matches source from Connect() - create the connection in pulse
            std::vector<RoutingSenderPULSEConnection>::iterator iterConn    = m_activeConnections.begin();
            std::vector<RoutingSenderPULSEConnection>::iterator iterConnEnd = m_activeConnections.end();
            for (; iterConn < iterConnEnd; ++iterConn)
            {
                if (iterConn->sourceID == iter->source.sourceID)
                {
                    logInfo("PULSE - asyncConnect() - connectionID:", iterConn->connectionID,
                            "move sinkInputIndex:", m_sourceToPASinkInput[iterConn->sourceID], "to sinkIndex:", m_sinkToPASink[iterConn->sinkID]);

                    routing_sender_move_sink_input(
                            this->m_paContext,
                            m_sourceToPASinkInput[iterConn->sourceID],
                            m_sinkToPASink[iterConn->sinkID],
                            this);

                    //TODO: add callback for pulse move sink input -> to send confirmation; for the moment directly send confirmation
                    this->m_routingReceiver->ackConnect(iterConn->handle, iterConn->connectionID, am::E_OK);
                }
            }
            //check of controller already requested vol adjustment  for this source
            bool requiresVolUpdate = false;
            for (int j = 0; j < i->volume.channels; j++)
            {
                if ((i->volume.values[j]*MAX_SOURCE_VOLUME / MAX_PULSE_VOLUME) != m_sourceToVolume[iter->source.sourceID])
                {
                    requiresVolUpdate = true;
                    logInfo("PULSE - sink registerd with vol:", (i->volume.values[j]*MAX_SOURCE_VOLUME / MAX_PULSE_VOLUME),
                            "; should be changed to:",
                            m_sourceToVolume[iter->source.sourceID]);
                    break;
                }
            }
            if (requiresVolUpdate)
            {
                routing_sender_sink_input_volume(
                        this->m_paContext,
                        m_sourceToPASinkInput[iter->source.sourceID],
                        m_sourceToVolume[iter->source.sourceID],
                        this);
            }
            //TODO: check mute state was requested by controller.
            break;
        }
    }
}


void RoutingSenderPULSE::getSourceOutputInfoCallback(pa_context *c, const pa_source_output_info *i, void *userdata)
{
    if (i == NULL)
    {
        return;
    }

    //search for corresponding Source
    std::vector<RoutingSenderPULSESourceSinkConfig>::iterator iter    = m_sinks.begin();
    std::vector<RoutingSenderPULSESourceSinkConfig>::iterator iterEnd = m_sinks.end();
    for (; iter < iterEnd; ++iter)
    {
        //try to match source PulseAudio properties agains config properties
        const char *property_value = pa_proplist_gets(i->proplist, iter->propertyName.c_str());

        if (property_value &&
            ( std::string::npos != std::string(property_value).find(iter->propertyValue) ||
              std::string::npos != iter->propertyValue.find(property_value)) )
        {
            logInfo("PULSE - source output registered:"
                , " sourceOutputIndex:", i->index, "sinkID:", iter->sink.sinkID);

            m_sinkToPASourceOutput[iter->sink.sinkID] = i->index;

            //iterate pending connection request
            // -> if there is a connection pending such that sink input "i" matches source from Connect() - create the connection in pulse
            std::vector<RoutingSenderPULSEConnection>::iterator iterConn    = m_activeConnections.begin();
            std::vector<RoutingSenderPULSEConnection>::iterator iterConnEnd = m_activeConnections.end();
            for (; iterConn < iterConnEnd; ++iterConn)
            {
                if (iterConn->sinkID == iter->sink.sinkID)
                {
                    logInfo("PULSE - asyncConnect() - connectionID:", iterConn->connectionID,
                            "move sourceOutputIndex:", m_sinkToPASourceOutput[iterConn->sinkID], "to sourceIndex:", m_sourceToPASource[iterConn->sourceID]);

                    routing_sender_move_source_output(
                            this->m_paContext,
                            m_sinkToPASourceOutput[iterConn->sinkID],
                            m_sourceToPASource[iterConn->sourceID],
                            this);

                    //TODO: add callback for pulse move source output -> to send confirmation; for the moment directly send confirmation
                    this->m_routingReceiver->ackConnect(iterConn->handle, iterConn->connectionID, am::E_OK);
                }
            }

            break;
        }
    }
}


void RoutingSenderPULSE::getSinkInfoCallback(pa_context *c, const pa_sink_info *i, int is_last, void *userdata)
{
    if (i != NULL)
    {
        if (strcmp("null", i->name) == 0)
        {
            this->m_paSinkNullIndex = i->index;
        }

        //search for corresponding (already registered) Sink
        std::vector<RoutingSenderPULSESourceSinkConfig>::iterator iter    = m_sinks.begin();
        std::vector<RoutingSenderPULSESourceSinkConfig>::iterator iterEnd = m_sinks.end();
        for (; iter < iterEnd; ++iter)
        {
            //first try to match the sink name from pulse audio sink name
            if (iter->sink.name == std::string(i->name))
            {
                logInfo("PULSE sink name PA:", i->name, "config name:" ,iter->sink.name);
                logInfo("PULSE - PA sink:", i->index,
                        "corresponding to AMGR sink:", iter->sink.sinkID, " - found");
                m_sinkToPASink[iter->sink.sinkID] = i->index;
            }
            else
            {
                //try to match sink PulseAudio properties against config properties
                const char *property_value = pa_proplist_gets(i->proplist, iter->propertyName.c_str());

                if (!property_value) continue;

                if (std::string::npos != iter->propertyValue.find(property_value))
                {
                    logInfo("PULSE - PA sink:", i->index,
                            "corresponding to AMGR sink:", iter->sink.sinkID, " - found");

                    m_sinkToPASink[iter->sink.sinkID] = i->index;
                }
            }
        }
    }
    else if (is_last)
    {
        routing_sender_get_source_info(this->m_paContext, this);
        logInfo("PULSE - PA sinks registration completed");
    }
}

void RoutingSenderPULSE::getSourceInfoCallback(pa_context *c, const pa_source_info *i, int is_last, void *userdata)
{
    if (i != NULL)
    {
        if (strcmp("null", i->name) == 0)
        {
            this->m_paSourceNullIndex = i->index;
        }

        //search for corresponding (already registered) Source
        std::vector<RoutingSenderPULSESourceSinkConfig>::iterator iter    = m_sources.begin();
        std::vector<RoutingSenderPULSESourceSinkConfig>::iterator iterEnd = m_sources.end();
        for (; iter < iterEnd; ++iter)
        {
            //first try to match the sink name from pulse audio sink name
            if (iter->sink.name == std::string(i->name))
            {
                logInfo("PULSE - PA source:", i->index,
                        "corresponding to AMGR source:", iter->source.sourceID, " - found");
                m_sourceToPASource[iter->source.sourceID] = i->index;
            }
            else
            {
                //try to match source PulseAudio properties against config properties
                const char *property_value = pa_proplist_gets(i->proplist, iter->propertyName.c_str());

                if (!property_value) continue;

                if (std::string::npos != iter->propertyValue.find(property_value))
                {
                    logInfo("PULSE - PA source:", i->index,
                            "corresponding to AMGR source:", iter->source.sourceID, " - found");

                    m_sourceToPASource[iter->source.sourceID] = i->index;
                }
            }

        }
    }
    else if (is_last)
    {
        this->m_routingReceiver->hookDomainRegistrationComplete(this->m_domain.domainID);
        logInfo("PULSE - PA sinks and source registration completed");
        //TODO: - search for existing sink inputs & sources outputs
    }
}
