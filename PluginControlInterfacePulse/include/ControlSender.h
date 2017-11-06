/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2011-2014, Wind River Systems
 * Copyright (C) 2014, GENIVI Alliance
 *
 * This file is part of Pulse Audio Interface Control Plugin.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License (MPL), v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information see http://www.genivi.org/.
 *
 * List of changes:
 *
 * 08.09.2014, Adrian Scarlat, First version of the code;
 *                             porting from AM ver 1.x;
 */

#ifndef CONTROLSENDER_H_
#define CONTROLSENDER_H_

#include "ControlConfig.h"
#include "IAmControl.h"
#include <map>

using namespace am;

class ControlSenderPlugin: public IAmControlSend
{
public:
    ControlSenderPlugin();

    virtual ~ControlSenderPlugin();

    am_Error_e startupController(IAmControlReceive* controlreceiveinterface);

    void setControllerReady();

    void setControllerRundown(const int16_t signal);

    am_Error_e hookUserConnectionRequest(
        const am_sourceID_t sourceID,
        const am_sinkID_t sinkID,
        am_mainConnectionID_t& mainConnectionID);

    am_Error_e hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID);

    am_Error_e hookUserSetMainSinkSoundProperty(
        const am_sinkID_t sinkID,
        const am_MainSoundProperty_s& soundProperty);

    am_Error_e hookUserSetMainSourceSoundProperty(
        const am_sourceID_t sourceID,
        const am_MainSoundProperty_s& soundProperty);

    am_Error_e hookUserSetSystemProperty(const am_SystemProperty_s& property);

    am_Error_e hookUserVolumeChange(
        const am_sinkID_t SinkID,
        const am_mainVolume_t newVolume);

    am_Error_e hookUserVolumeStep(
        const am_sinkID_t SinkID,
        const int16_t increment);

    am_Error_e hookUserSetSinkMuteState(
        const am_sinkID_t sinkID,
        const am_MuteState_e muteState);

    am_Error_e hookSystemRegisterDomain(
        const am_Domain_s& domainData,
        am_domainID_t& domainID);

    am_Error_e hookSystemDeregisterDomain(const am_domainID_t domainID);

    void hookSystemDomainRegistrationComplete(const am_domainID_t domainID);

    am_Error_e hookSystemRegisterSink(
        const am_Sink_s& sinkData,
        am_sinkID_t& sinkID);

    am_Error_e hookSystemDeregisterSink(const am_sinkID_t sinkID);

    am_Error_e hookSystemRegisterSource(
        const am_Source_s& sourceData,
        am_sourceID_t& sourceID);

    am_Error_e hookSystemDeregisterSource(const am_sourceID_t sourceID);

    am_Error_e hookSystemRegisterGateway(
        const am_Gateway_s& gatewayData,
        am_gatewayID_t& gatewayID);

    am_Error_e hookSystemRegisterConverter(
        const am_Converter_s& converterData,
        am_converterID_t& converterID);

    am_Error_e hookSystemDeregisterGateway(const am_gatewayID_t gatewayID);

    am_Error_e hookSystemDeregisterConverter(const am_converterID_t converterID);

    am_Error_e hookSystemRegisterCrossfader(
        const am_Crossfader_s& crossfaderData,
        am_crossfaderID_t& crossfaderID);

    am_Error_e hookSystemDeregisterCrossfader(
        const am_crossfaderID_t crossfaderID);

    void hookSystemSinkVolumeTick(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const am_volume_t volume);

    void hookSystemSourceVolumeTick(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const am_volume_t volume);

    void hookSystemInterruptStateChange(
        const am_sourceID_t sourceID,
        const am_InterruptState_e interruptState);

    void hookSystemSinkAvailablityStateChange(
        const am_sinkID_t sinkID,
        const am_Availability_s& availability);

    void hookSystemSourceAvailablityStateChange(
        const am_sourceID_t sourceID,
        const am_Availability_s& availability);

    void hookSystemDomainStateChange(
        const am_domainID_t domainID,
        const am_DomainState_e state);

    void hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s>& data);

    void hookSystemSpeedChange(const am_speed_t speed);

    void hookSystemTimingInformationChanged(
        const am_mainConnectionID_t mainConnectionID,
        const am_timeSync_t time);

    void cbAckConnect(
        const am_Handle_s handle,
        const am_Error_e errorID);

    void cbAckDisconnect(
        const am_Handle_s handle,
         const am_Error_e errorID);

    void cbAckCrossFade(
        const am_Handle_s handle,
        const am_HotSink_e hostsink,
        const am_Error_e error);

    void cbAckSetSinkVolumeChange(
        const am_Handle_s handle,
        const am_volume_t volume,
        const am_Error_e error);

    void cbAckSetSourceVolumeChange(
        const am_Handle_s handle,
        const am_volume_t voulme,
        const am_Error_e error);

    void cbAckSetSourceState(
        const am_Handle_s handle,
        const am_Error_e error);

    void cbAckSetSourceSoundProperty(
        const am_Handle_s handle,
        const am_Error_e error);

    void cbAckSetSourceSoundProperties(
        const am_Handle_s handle,
        const am_Error_e error);

    void cbAckSetSinkSoundProperty(
        const am_Handle_s handle,
        const am_Error_e error);

    void cbAckSetSinkSoundProperties(
        const am_Handle_s handle,
        const am_Error_e error);

    am_Error_e getConnectionFormatChoice(
        const am_sourceID_t sourceID,
        const am_sinkID_t sinkID,
        const am_Route_s listRoute,
        const std::vector<am_CustomConnectionFormat_t> listPossibleConnectionFormats,
        std::vector<am_CustomConnectionFormat_t>& listPrioConnectionFormats);

    void getInterfaceVersion(std::string& version) const;
    void confirmCommandReady(const am_Error_e error);
    void confirmRoutingReady(const am_Error_e error);
    void confirmCommandRundown(const am_Error_e error);
    void confirmRoutingRundown(const am_Error_e error);

    am_Error_e hookSystemUpdateSink(
        const am_sinkID_t sinkID,
        const am_sinkClass_t sinkClassID,
        const std::vector<am_SoundProperty_s>& listSoundProperties,
        const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats,
        const std::vector<am_MainSoundProperty_s>& listMainSoundProperties);

    am_Error_e hookSystemUpdateSource(
        const am_sourceID_t sourceID,
        const am_sourceClass_t sourceClassID,
        const std::vector<am_SoundProperty_s>& listSoundProperties,
        const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats,
        const std::vector<am_MainSoundProperty_s>& listMainSoundProperties);

    am_Error_e hookSystemUpdateGateway(
        const am_gatewayID_t gatewayID,
        const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats,
        const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats,
        const std::vector<bool>& convertionMatrix);

    am_Error_e hookSystemUpdateConverter(
        const am_converterID_t converterID,
        const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats,
        const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats,
        const std::vector<bool>& convertionMatrix);

    void cbAckSetVolumes(
        const am_Handle_s handle,
        const std::vector<am_Volumes_s>& listVolumes,
        const am_Error_e error);

    void cbAckSetSinkNotificationConfiguration(
        const am_Handle_s handle,
        const am_Error_e error);

    void cbAckSetSourceNotificationConfiguration(
        const am_Handle_s handle,
        const am_Error_e error);

    void hookSinkNotificationDataChanged(
        const am_sinkID_t sinkID,
        const am_NotificationPayload_s& payload);

    void hookSourceNotificationDataChanged(
        const am_sourceID_t sourceID,
        const am_NotificationPayload_s& payload);

    am_Error_e hookUserSetMainSinkNotificationConfiguration(
        const am_sinkID_t sinkID,
        const am_NotificationConfiguration_s& notificationConfiguration);

    am_Error_e hookUserSetMainSourceNotificationConfiguration(
        const am_sourceID_t sourceID,
        const am_NotificationConfiguration_s& notificationConfiguration);

    void hookSystemNodeStateChanged(const NsmNodeState_e NodeStateId);

    void hookSystemNodeApplicationModeChanged(
        const NsmApplicationMode_e ApplicationModeId);

    void hookSystemSessionStateChanged(
        const std::string& sessionName,
        const NsmSeat_e seatID,
        const NsmSessionState_e sessionStateID);

    NsmErrorStatus_e hookSystemLifecycleRequest(
        const uint32_t Request,
        const uint32_t RequestId);

    void hookSystemSingleTimingInformationChanged(
        const am_connectionID_t connectionID,
        const am_timeSync_t time);
private:
    bool isStaticRoutingEnabled() {return true;}

    /**
     * Load controller configuration file. The name and path of the configuration file
     * should be identical to library (except the extensions which is .conf instead of .so)
     */
    am_Error_e loadConfig();

    /**
     * Apply some actions on top of main connection specified by (connectionID)
     * If interrupt=true, the action like "disconnect" is reversible (e.g. when the main connection
     * triggering this action, is destroyed, connectionID will be re-established, otherwise actions like "disconnect" are permanent)
     * If inverse = true, action is inversely applied: (e.g. action = MUTE and inverse = true, we apply UNMUTE)
     */
    am_Error_e applyActionOnConnection(
        am_mainConnectionID_t connectionID,
        am_ConnectAction_s &action,
        bool interrupt,
        bool inverse,
        bool force = false);

    am_Error_e applyActionOnSubConnection(
        am_mainConnectionID_t connectionID,
        am_Connection_s subConnection,
        am_ConnectAction_s &action,
        bool interrupt,
        bool inverse);

    am_Error_e getStaticRoute(
        const bool onlyfree,
        const am_sourceID_t sourceID,
        const am_sinkID_t sinkID,
        std::vector<am_Route_s>& returnList);

    am_Error_e getSourceInfo(
        const am_sourceID_t sourceID,
        am_Source_s &sourceData);

/*
 * TODO: REVIEW DESCRIPTION
 * A mapping between connection and controller actions taken during connection.
 * To be used to apply inverse actions during disconnection:
 * am_mainConnectionID_t      - the main connection request triggering the action
 * am_connectionID_t          - the sub-connection target of the action
 * list of am_ConnectAction_s - actions to be applied on this connection
 *                              in this list, only one action is not pending
 *                              and the non pending action has the higher priority
 *                             (e.g. disconnect > mute > decrease volume)
 */
    std::map<am_mainConnectionID_t,
             std::map<am_mainConnectionID_t,
                     std::vector<am_ConnectAction_s> > > mConnectionID2Actions;
/*
 * A mapping between main connection and and the list of associated sub-connections.
 * First element in the list has the same connectionID as the map key.
 */
    std::map<am_mainConnectionID_t,
             std::vector<am_Connection_s> >     mConnectionID2Subconnections;

    IAmControlReceive                           *mControlReceiveInterface;
    std::vector<am_Route_s>                     mStaticRoutes;
    std::vector<am_MixingRule_s>                mMixingRules;

/*
 * map of structures registered during initialization/configuration phase;
 * DO NOT USE DIRECTLY
 */
    std::map<std::string, am_Source_s>          mNames2Sources;
    std::map<std::string, am_Sink_s>            mNames2Sinks;
    std::map<std::string, am_SinkClass_s>       mNames2SinksClasses;
    std::map<std::string, am_SourceClass_s>     mNames2SourceClasses;
    std::map<std::string, am_Domain_s>          mNames2Domains;
    std::map<std::string, am_Gateway_s>         mNames2Gateways;
};

#endif /* CONTROLSENDER_H_ */
