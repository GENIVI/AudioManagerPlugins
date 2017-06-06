#include <algorithm>
#include <cstring>

#include <CAmDltWrapper.h>

#include "CAmRoutingSenderPulse.h"

using namespace std;

namespace am
{

DLT_DECLARE_CONTEXT(routingPulse)

extern "C" IAmRoutingSend* PluginRoutingInterfacePULSEFactory()
{
    return (new CAmRoutingSenderPulse());
}

extern "C" void destroyPluginRoutingInterfacePULSE(IAmRoutingSend* routingSendInterface)
{
    delete routingSendInterface;
}

CAmRoutingSenderPulse::CAmRoutingSenderPulse() :
    mPulse(*this),
    mVolumeManager(mPulse),
    mConnectionManager(mPulse),
    mDomainID(0),
    mShadow(nullptr)
{
}

CAmRoutingSenderPulse::~CAmRoutingSenderPulse()
{
    mPulse.stop();
}

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
    CAmXmlConfigParser xmlConfigParser;
    rp_Configuration_s config;

    xmlConfigParser.parse(config);

    logInfo("PULSE - routingInterfacesReady");

    mShadow->confirmRoutingReady(handle, E_OK);

    registerDomain(config.domain);

    registerSources(config.listSources);

    registerSinks(config.listSinks);

    mShadow->hookDomainRegistrationComplete(mDomainID);

    mPulse.start();
}

void CAmRoutingSenderPulse::setRoutingRundown(uint16_t handle)
{
    mPulse.stop();

    mShadow->confirmRoutingRundown(handle, am::E_OK);
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

    logInfo("PULSE - asyncConnec source: ", sourceID, ", sink: ", sinkID, ",connection: ", connectionID);

    auto ret = mConnectionManager.connect(sourceID, sinkID, connectionID);

    mShadow->ackConnect(handle, connectionID, ret);

    return ret;
}

am_Error_e CAmRoutingSenderPulse::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    logInfo("PULSE - asyncDisconnect - connection ID:", connectionID);

    auto ret = mConnectionManager.disconnect(connectionID);

    mShadow->ackDisconnect(handle, connectionID, ret);

    return ret;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSinkVolume(
        const am_Handle_s handle,
        const am_sinkID_t sinkID,
        const am_volume_t volume,
        const am_CustomRampType_t ramp,
        const am_time_t time)
{
    am_Error_e ret = mVolumeManager.setSinkVolume(sinkID, volume, ramp, time);

    mShadow->ackSetSinkVolumeChange(handle, volume, ret);

    return ret;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSourceVolume(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const am_volume_t volume,
        const am_CustomRampType_t ramp,
        const am_time_t time)
{
    am_Error_e ret = mVolumeManager.setSourceVolume(sourceID, volume, ramp, time);

    mShadow->ackSetSourceVolumeChange(handle, volume, ret);

    return ret;
}

am_Error_e CAmRoutingSenderPulse::asyncSetSourceState(
        const am_Handle_s handle,
        const am_sourceID_t sourceID,
        const am_SourceState_e state)
{
    logInfo("PULSE - asyncSetSourceState, id: ", sourceID, ", state: ", state);

    am_Error_e ret = mVolumeManager.setSourceState(sourceID, state);

    mShadow->ackSetSourceState(handle, ret);

    return ret;
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

    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderPulse::asyncSetSinkNotificationConfiguration(const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) handle;
    (void) sinkID;
    (void) notificationConfiguration;

    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderPulse::asyncSetSourceNotificationConfiguration(const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) handle;
    (void) sourceID;
    (void) notificationConfiguration;

    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderPulse::resyncConnectionState(const am_domainID_t domainID, vector<am_Connection_s>& listOfExistingConnections)
{
    return E_OK;
}

void CAmRoutingSenderPulse::onNewSource(const pa_source_info *info)
{
    am_sourceID_t sourceID = getSourceIdByConfig(info->name, info->proplist);

    if (sourceID)
    {
        mSourceIdxToSourceId[info->index] = sourceID;

        mVolumeManager.newSource(sourceID, info);
        mConnectionManager.newSource(sourceID, info->index);

        setSourceAvailable(sourceID, true);
    }
    else
    {
        logWarning("PULSE - Can't find config for source: ", info->name);
    }
}

void CAmRoutingSenderPulse::onRemoveSource(uint32_t idx)
{
    am_sourceID_t sourceID = getSourceIdByIdx(idx, false);

    mSourceIdxToSourceId.erase(idx);

    if (sourceID)
    {
        mVolumeManager.removeSource(idx);
        mConnectionManager.removeSource(idx);

        setSourceAvailable(sourceID, false);
    }
}

void CAmRoutingSenderPulse::onNewSinkInput(const pa_sink_input_info *info)
{
    am_sourceID_t sourceID = getSourceIdByConfig(info->name, info->proplist);

    if (sourceID)
    {
        mSinkInputIdxToSourceId[info->index] = sourceID;

        mVolumeManager.newSinkInput(sourceID, info);
        mConnectionManager.newSinkInput(sourceID, info->index);

        setSourceAvailable(sourceID, true);
    }
    else
    {
        logWarning("PULSE - Can't find config for sink input: ", info->name);
    }
}

void CAmRoutingSenderPulse::onRemoveSinkInput(uint32_t idx)
{
    am_sourceID_t sourceID = getSourceIdByIdx(idx, true);

    mSinkInputIdxToSourceId.erase(idx);

    if (sourceID)
    {
        mVolumeManager.removeSinkInput(idx);
        mConnectionManager.removeSinkInput(idx);

        setSourceAvailable(sourceID, false);
    }
}

void CAmRoutingSenderPulse::onNewSink(const pa_sink_info *info)
{
    am_sinkID_t sinkID = getSinkIdByConfig(info->name, info->proplist);

    if (sinkID)
    {
        mSinkIdxToSinkId[info->index] = sinkID;

        mVolumeManager.newSink(sinkID, info);
        mConnectionManager.newSink(sinkID, info->index);

        setSinkAvailable(sinkID, true);
    }
    else
    {
        logWarning("PULSE - Can't find config for sink: ", info->name);
    }
}

void CAmRoutingSenderPulse::onRemoveSink(uint32_t idx)
{
    am_sinkID_t sinkID = getSinkIdByIdx(idx, false);

    mSinkIdxToSinkId.erase(idx);

    if (sinkID)
    {
        mVolumeManager.removeSink(idx);
        mConnectionManager.removeSink(idx);

        setSinkAvailable(sinkID, false);
    }
}

void CAmRoutingSenderPulse::onNewSourceOutput(const pa_source_output_info *info)
{
    am_sinkID_t sinkID = getSinkIdByConfig(info->name, info->proplist);

    if (sinkID)
    {
        mSourceOutputIdxToSinkId[info->index] = sinkID;

        setSinkAvailable(sinkID, true);

        mVolumeManager.newSourceOutput(sinkID, info);
        mConnectionManager.newSourceOutput(sinkID, info->index);
    }
    else
    {
        logWarning("PULSE - Can't find config for source output: ", info->name);
    }

}

void CAmRoutingSenderPulse::onRemoveSourceOutput(uint32_t idx)
{
    am_sinkID_t sinkID = getSinkIdByIdx(idx, true);

    mSourceOutputIdxToSinkId.erase(idx);

    if (sinkID)
    {
        setSinkAvailable(sinkID, false);

        mVolumeManager.removeSourceOutput(idx);
        mConnectionManager.removeSourceOutput(idx);
    }
}

bool CAmRoutingSenderPulse::cmpProperty(pa_proplist *proplist, const string& name, const string& value)
{
    const char *propertyValue = pa_proplist_gets(proplist, name.c_str());

    if (propertyValue &&
        (string::npos != string(propertyValue).find(value) ||
         string::npos != value.find(propertyValue)))
    {
        logInfo("PULSE - matches property name:", name, " propValue: ", propertyValue, " value: ", value);

        return true;
    }

    return false;
}

am_sourceID_t CAmRoutingSenderPulse::getSourceIdByIdx(uint32_t idx, bool isVirtual)
{
    am_sourceID_t sourceID = 0;

    if (!isVirtual)
    {
        auto it = mSourceIdxToSourceId.find(idx);

        if (it == mSourceIdxToSourceId.end())
        {
            logWarning("PULSE - Can't find source ID for source idx: ", idx);

            return 0;
        }

        sourceID = it->second;
    }
    else
    {
        auto it = mSinkInputIdxToSourceId.find(idx);

        if (it == mSinkInputIdxToSourceId.end())
        {
            logWarning("PULSE - Can't find source ID for sink input idx: ", idx);

            return 0;
        }

        sourceID = it->second;
    }

    auto it = mRegisteredSources.find(sourceID);

    if (it == mRegisteredSources.end())
    {
        logWarning("PULSE - source id: ", sourceID, " is not registered");

        return 0;
    }

    return sourceID;
}

am_sinkID_t CAmRoutingSenderPulse::getSinkIdByIdx(uint32_t idx, bool isVirtual)
{
    am_sinkID_t sinkID = 0;

    if (!isVirtual)
    {
        auto it = mSinkIdxToSinkId.find(idx);

        if (it == mSinkIdxToSinkId.end())
        {
            logWarning("PULSE - Can't find sink ID for sink idx: ", idx);

            return 0;
        }

        sinkID = it->second;
    }
    else
    {
        auto it = mSourceOutputIdxToSinkId.find(idx);

        if (it == mSourceOutputIdxToSinkId.end())
        {
            logWarning("PULSE - Can't find sink ID for source output idx: ", idx);

            return 0;
        }

        sinkID = it->second;
    }

    auto it = mRegisteredSinks.find(sinkID);

    if (it == mRegisteredSinks.end())
    {
        logWarning("PULSE - sink id: ", sinkID, " is not registered");

        return 0;
    }

    return sinkID;
}

am_sourceID_t CAmRoutingSenderPulse::getSourceIdByConfig(const char* name, pa_proplist *proplist)
{
    // check name
    auto it = find_if(mRegisteredSources.begin(), mRegisteredSources.end(),
                      [name](const pair<am_sourceID_t, rp_ConfigSource_s>& value)
                      { return value.second.deviceName == string(name); });

    // check property
    if (it == mRegisteredSources.end())
    {
        it = find_if(mRegisteredSources.begin(), mRegisteredSources.end(),
                     [proplist, this](const pair<am_sourceID_t, rp_ConfigSource_s>& value)
                     { return cmpProperty(proplist, value.second.propertyName, value.second.propertyValue); } );
    }

    if (it == mRegisteredSources.end())
    {
        return 0;
    }

    return it->first;
}

am_sinkID_t CAmRoutingSenderPulse::getSinkIdByConfig(const char* name, pa_proplist *proplist)
{
    // check name
    auto it = find_if(mRegisteredSinks.begin(), mRegisteredSinks.end(),
                      [name](const pair<am_sinkID_t, rp_ConfigSink_s>& value)
                      { return value.second.deviceName == string(name); });

    // check property
    if (it == mRegisteredSinks.end())
    {
        it = find_if(mRegisteredSinks.begin(), mRegisteredSinks.end(),
                     [proplist, this](const pair<am_sinkID_t, rp_ConfigSink_s>& value)
                     { return cmpProperty(proplist, value.second.propertyName, value.second.propertyValue); } );
    }

    if (it == mRegisteredSinks.end())
    {
        return 0;
    }

    return it->first;
}

void CAmRoutingSenderPulse::setSourceAvailable(am_sourceID_t sourceID, bool isAvailable)
{
    am_Availability_s availability;

    if (isAvailable)
    {
        availability.availability = A_AVAILABLE;
        availability.availabilityReason = AR_GENIVI_NEWMEDIA;
    }
    else
    {
        availability.availability = A_UNAVAILABLE;
        availability.availabilityReason = AR_GENIVI_NOMEDIA;
    }

    mShadow->hookSourceAvailablityStatusChange(sourceID, availability);
}

void CAmRoutingSenderPulse::setSinkAvailable(am_sinkID_t sinkID, bool isAvailable)
{
    am_Availability_s availability;

    if (isAvailable)
    {
        availability.availability = A_AVAILABLE;
        availability.availabilityReason = AR_GENIVI_NEWMEDIA;
    }
    else
    {
        availability.availability = A_UNAVAILABLE;
        availability.availabilityReason = AR_GENIVI_NOMEDIA;
    }

    mShadow->hookSinkAvailablityStatusChange(sinkID, availability);
}

void CAmRoutingSenderPulse::registerDomain(const rp_ConfigDomain_s& config)
{
    am_Domain_s amDomain;

    amDomain.name     = config.name;
    returnBusName(amDomain.busname);//set domain bus name = current interface bus name
    amDomain.nodename = config.nodeName;
    amDomain.early    = false;
    amDomain.complete = true;
    amDomain.state    = am::DS_CONTROLLED;
    amDomain.domainID = config.id;

    mShadow->registerDomain(amDomain, mDomainID);
}

void CAmRoutingSenderPulse::registerSources(const vector<rp_ConfigSource_s>& listSources)
{
    for(auto config : listSources)
    {
        registerSource(config);
    }
}

void CAmRoutingSenderPulse::registerSinks(const vector<rp_ConfigSink_s>& listSinks)
{
    for(auto config : listSinks)
    {
        registerSink(config);
    }
}


am_Error_e CAmRoutingSenderPulse::registerSource(const rp_ConfigSource_s& rpSource)
{
    am_Source_s amSource {};
    am_sourceID_t sourceID = 0;

    amSource.sourceID = rpSource.id;
    amSource.name = rpSource.name;
    // Do we need it ?
    amSource.sourceState = SS_OFF;
    amSource.domainID = mDomainID;
    amSource.visible = true;
    amSource.volume = 0;
    amSource.listConnectionFormats.push_back(am::CF_GENIVI_STEREO);

    mShadow->peekSourceClassID(rpSource.className, amSource.sourceClassID);

    auto ret = mShadow->registerSource(amSource, sourceID);

    if (ret == E_OK)
    {
        mRegisteredSources[sourceID] = rpSource;

        mVolumeManager.registerSource(sourceID, amSource.volume, amSource.sourceState);
        mConnectionManager.registerSource(sourceID);

        logInfo("PULSE - register source:", rpSource.name,
                "(", rpSource.propertyName , ", ", rpSource.propertyValue, ")");
    }
    else
    {
        logError("PULSE - failed to register source:", rpSource.name,
                 "(", rpSource.propertyName , ", ", rpSource.propertyValue, ")");
    }

    return ret;
}

am_Error_e CAmRoutingSenderPulse::deregisterSource(am_sourceID_t sourceID)
{
    auto ret = mShadow->deregisterSource(sourceID);

    if (ret == E_OK)
    {
        mRegisteredSources.erase(sourceID);

        mVolumeManager.deregisterSource(sourceID);
        mConnectionManager.deregisterSource(sourceID);
    }

    return ret;
}

am_Error_e CAmRoutingSenderPulse::registerSink(const rp_ConfigSink_s& rpSink)
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

    amSink.sinkID = rpSink.id;
    amSink.name = rpSink.name;

    amSink.volume = -300;

    amSink.domainID = mDomainID;
    amSink.visible = true;

    amSink.listSoundProperties.push_back(l_spTreble);
    amSink.listSoundProperties.push_back(l_spMid);
    amSink.listSoundProperties.push_back(l_spBass);
    amSink.listConnectionFormats.push_back(am::CF_GENIVI_STEREO);

    mShadow->peekSinkClassID(rpSink.className, amSink.sinkClassID);

    auto ret = mShadow->registerSink(amSink, sinkID);

    if (ret == E_OK)
    {
        mRegisteredSinks[sinkID] = rpSink;

        mVolumeManager.registerSink(sinkID, amSink.volume, amSink.muteState);
        mConnectionManager.registerSink(sinkID);

        logInfo("PULSE - register sink:", rpSink.name,
                "(", rpSink.propertyName , ", ", rpSink.propertyValue, ")");
    }
    else
    {
        logError("PULSE - failed to register sink:", rpSink.name,
                "(", rpSink.propertyName , ", ", rpSink.propertyValue, ")");
    }

    return ret;
}

am_Error_e CAmRoutingSenderPulse::deregisterSink(am_sinkID_t sinkID)
{
    auto ret = mShadow->deregisterSink(sinkID);

    if (ret == E_OK)
    {
        mRegisteredSinks.erase(sinkID);

        mVolumeManager.deregisterSink(sinkID);
        mConnectionManager.deregisterSink(sinkID);
    }

    return ret;
}

}
