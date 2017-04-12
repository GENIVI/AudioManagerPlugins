/*******************************************************************************
 *  \copyright (c) 2016 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Jens Lorenz, jlorenz@de.adit-jv.com 2013-2016
 *           Mattia Guerra, mguerra@de.adit-jv.com 2016
 *           Jayanth MC, Jayanth.mc@in.bosch.com 2013-2014
 *
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


#include <sstream>
#include <algorithm>
#include "CAmRoutingAdapterALSAMixerCtrl.h"
#include "CAmRoutingAdapterALSASender.h"
#include "CAmRoutingAdapterALSAParser.h"

#include "audiomanagerconfig.h"

#include "TAmPluginTemplate.h"

#include "CAmDLTLogging.h"

#define DEFAULT_PLUGIN_STREAMING_DIR DEFAULT_PLUGIN_DIR "/streaming"

using namespace std;
using namespace am;

extern "C" IAmRoutingSend* PluginRoutingAdapterALSAFactory()
{
    return (new CAmRoutingAdapterALSASender());
}

extern "C" void destroyPluginRoutingAdapterALSA(IAmRoutingSend* routingSendInterface)
{
    delete routingSendInterface;
}

CAmRoutingAdapterALSASender::CAmRoutingAdapterALSASender() :
        mpShadow(NULL), mpReceiveInterface(NULL), mpSocketHandler(NULL),
        mDataBase(this), mBusname(RA_ALSA_BUSNAME),
        mCommandLineArg("P", "routingAdapterProxyFolder",
                        "Routing Adapter Proxy Folder for Backend Selection. \
                         Please note that the library, configured on XML file, \
                         will be dlopened by the Routing Sender Plugin, dlopened \
                         on its behalf. It will load a Proxy which is providing \
                         the desired implementation for dealing with the target \
                         Backend.",
                         false, DEFAULT_PLUGIN_STREAMING_DIR, "string")
{
    // TODO: command line to set configuration path???
    CAmCommandLineSingleton::instance()->add(mCommandLineArg);
    CAmRoutingAdapterALSAParser parser(mDataBase, mBusname);
    parser.readConfig();
}

CAmRoutingAdapterALSASender::~CAmRoutingAdapterALSASender()
{
    /* stop all running proxies */
    vector<IAmRoutingAdapterALSAProxy*> proxies;
    mDataBase.getProxyLists(proxies);
    for (IAmRoutingAdapterALSAProxy * proxy : proxies)
    {
        if (!proxy)
        {
            continue;
        }

        void *dlhandle = proxy->getLibHandle();
        delete proxy;
        if (dlhandle)
        {
            dlclose(dlhandle);
        }
    }

    /* stop all asynchronous operating volumes */
    vector<CAmRoutingAdapterALSAVolume*> volumes;
    mDataBase.getVolumeOpLists(volumes);
    for (CAmRoutingAdapterALSAVolume * volume : volumes)
    {
        delete volume;
    }

    mDataBase.cleanup();
    delete mpShadow;
}

am_Error_e CAmRoutingAdapterALSASender::startupInterface(IAmRoutingReceive * receiveInterface)
{
    assert(receiveInterface);
    mpReceiveInterface = receiveInterface;
    mpReceiveInterface->getSocketHandler(mpSocketHandler);
    assert(mpSocketHandler);
    mpShadow = new IAmRoutingReceiverShadow(mpReceiveInterface, mpSocketHandler);
    return E_OK;
}


am_Error_e CAmRoutingAdapterALSASender::returnBusName(std::string & busname) const
{
    busname = mBusname;
    return E_OK;
}


void CAmRoutingAdapterALSASender::getInterfaceVersion(std::string & version) const
{
    version = RoutingVersion;
}

am_Error_e CAmRoutingAdapterALSASender::registerDomain(am_Domain_s & domain)
{
    if (domain.name.length() == 0)
    {
        logAmRaError("CRaALSASender::registerDomain", "Error in Domain configuration. Name is not provided!");
        return E_DATABASE_ERROR;
    }

    /* Register only in case if it is dynamic otherwise skip */
    am_Error_e error = E_OK;
    if ((domain.domainID == 0) || (domain.domainID >= DYNAMIC_ID_BOUNDARY))
    {
        error = mpShadow->registerDomain(domain, domain.domainID);
        if ((error == E_OK) && (domain.state == DS_INDEPENDENT_STARTUP))
        {
            // variable needs to be updated after successful registering of domain.
            domain.early = true;
        }
    }

    return error;
}

void CAmRoutingAdapterALSASender::registerSource(ra_sourceInfo_s & info, am_domainID_t domainID)
{
    am_Error_e error = E_OK;
    am_Source_s & source = info.amInfo;

    ra_proxyInfo_s * proxy = mDataBase.findProxyInDomain(domainID, source.name);
    if (proxy && source.sourceState == SS_UNKNNOWN)
    {
        source.sourceState = SS_OFF;
    }

    if (source.name.length() == 0)
    {
        logAmRaError("CRaALSASender::registerSource", "Error in Source configuration. Name is not provided!");
        return;
    }
    if ((info.srcClsNam.length() != 0) && (source.visible == true))
    {
        if ((error = mpReceiveInterface->peekSourceClassID(info.srcClsNam, source.sourceClassID)) != E_OK)
        {
            logAmRaInfo("CRaALSASender::registerSource", "Error on peekSourceClassID, failed with", error);
        }
    }
    /* Register only in case if it is dynamic otherwise skip */
    if ((source.sourceID == 0) || (source.sourceID >= DYNAMIC_ID_BOUNDARY))
    {
        source.domainID = domainID;
        if ((error = mpShadow->registerSource(source, source.sourceID)) != E_OK)
        {
            logAmRaError("CRaALSASender::registerSource", "Error on registering Source,", source.name, error);
        }
    }
}

void CAmRoutingAdapterALSASender::registerSink(ra_sinkInfo_s & info, am_domainID_t domainID)
{
    am_Error_e error = E_OK;
    am_Sink_s & sink = info.amInfo;

    if (sink.name.length() == 0)
    {
        logAmRaError("CRaALSASender::registerSink", "ret in Sink configuration. Name is not provided!");
        return;
    }

    if ((info.sinkClsNam.length() != 0) && (sink.visible == true))
    {
        if ((error = mpReceiveInterface->peekSinkClassID(info.sinkClsNam, sink.sinkClassID)) != E_OK)
        {
            logAmRaError("CRaALSASender::registerSink", "ret on peekSinkClassID, failed with", error);
        }
    }

    /* Register only in case if it is dynamic otherwise skip */
    if ((sink.sinkID == 0) || (sink.sinkID >= DYNAMIC_ID_BOUNDARY))
    {
        sink.domainID = domainID;
        if ((error = mpShadow->registerSink(sink, sink.sinkID)) != E_OK)
        {
            logAmRaError("CRaALSASender::registerSink", "ret on registering Sink,", sink.name, error);
        }
    }
}

void CAmRoutingAdapterALSASender::registerGateway(ra_gatewayInfo_s & info, am_domainID_t domainID)
{
    am_Error_e error = E_OK;
    am_Gateway_s & gateway = info.amInfo;

    /* validate Gateway Information*/
    if ((gateway.name.length() == 0)
            || (info.srcDomNam.length() == 0) || (info.sinkDomNam.length() == 0)
            || (info.srcNam.length() == 0) || (info.sinkNam.length() == 0))
    {
        logAmRaError("CRaALSASender::registerGateway Error in Gateway configuration",
                         "Domain name(s) or gateway or source or sink name not configured!");
        return;
    }
    /* get source information*/
    if (((error = mpReceiveInterface->peekDomain(info.srcDomNam, gateway.domainSourceID)) == E_OK)
            && ((error = mpReceiveInterface->peekSource(info.srcNam, gateway.sourceID)) == E_OK))
    {
        /*get sink information*/

        if (((error = mpReceiveInterface->peekDomain(info.sinkDomNam, gateway.domainSinkID)) == E_OK)
                && ((error = mpReceiveInterface->peekSink(info.sinkNam, gateway.sinkID)) == E_OK))
        {
            /* check if the expected matrix size is equal to the configured */
            size_t expectedMatrixSize = gateway.listSinkFormats.size() * gateway.listSourceFormats.size();
            if (expectedMatrixSize != gateway.convertionMatrix.size())
            {
                logAmRaInfo("CRaALSASender::registerGateway", gateway.name,
                        ": Matrix vs. Source and Sink formats incompatible. Expected:", expectedMatrixSize, "!=",
                        gateway.convertionMatrix.size());
            }

            /* Register only in case if it is dynamic otherwise skip */
            if ((gateway.gatewayID == 0) || (gateway.gatewayID >= DYNAMIC_ID_BOUNDARY))
            {
                gateway.controlDomainID = domainID;
                if ((error = mpShadow->registerGateway(gateway, gateway.gatewayID)) != E_OK)
                {
                    logAmRaError("CRaALSASender::registerGateway", "Error on registering gateway,", gateway.name, error);
                }
            }
        }
        else
        {
            logAmRaError("CRaALSASender::registerGateway Error on peek SinkInformation:", error);
        }
    }
    else
    {
        logAmRaError("CRaALSASender::registerGateway Error on peek SourceInformation:", error);
    }
}

void CAmRoutingAdapterALSASender::hookDomainRegistrationComplete(am_domainID_t domainID)
{
    mpShadow->hookDomainRegistrationComplete(domainID);
}

void CAmRoutingAdapterALSASender::deregisterDomain(const am_domainID_t &domainID)
{
    ra_domainInfo_s *domainInfo = mDataBase.findDomain(domainID);
    am_Error_e error = E_OK;

    if (domainInfo == NULL)
    {
        logAmRaError("Domain with ID =", domainID, " not found");
        return;
    }
    /*
     * Deregister all gateways first
     */
    for (ra_gatewayInfo_s & gateway : domainInfo->lGatewayInfo)
    {
        deregisterGateway(gateway.amInfo.gatewayID);
    }

    /*
     * Then deregister sources and sinks
     */
    for (ra_sinkInfo_s & sink : domainInfo->lSinkInfo)
    {
        deregisterSink(sink.amInfo.sinkID);
    }
    for (ra_sourceInfo_s & source : domainInfo->lSourceInfo)
    {
        deregisterSource(source.amInfo.sourceID);
    }

    /*
     * In the end, the domain itself
     */
    if (domainID >= DYNAMIC_ID_BOUNDARY)
    {
        if ((error = mpShadow->deregisterDomain(domainID)) != E_OK)
        {
            logAmRaError("CRaALSASender::deregisterDomain", "Error on deregistering domainID ", domainID, error);
            return;
        }
        logAmRaDebug("Domain ", domainID, "unregistered successfully");
    }
}

void CAmRoutingAdapterALSASender::deregisterSource(const am_sourceID_t &sourceID)
{
    am_Error_e error = E_OK;
    if (sourceID >= DYNAMIC_ID_BOUNDARY)
    {
        if ((error = mpShadow->deregisterSource(sourceID)) != E_OK)
        {
            logAmRaError("CRaALSASender::deregisterSource", "Error on deregistering sourceID ", sourceID, error);
            return;
        }
        logAmRaDebug("Source ", sourceID, "unregistered successfully");
    }
}

void CAmRoutingAdapterALSASender::deregisterSink(const am_sinkID_t &sinkID)
{
    am_Error_e error = E_OK;
    if (sinkID >= DYNAMIC_ID_BOUNDARY)
    {
        if ((error = mpShadow->deregisterSink(sinkID)) != E_OK)
        {
            logAmRaError("CRaALSASender::deregisterSink", "Error on deregistering sinkID ", sinkID, error);
            return;
        }
        logAmRaDebug("Sink ", sinkID, "unregistered successfully");
    }
}

void CAmRoutingAdapterALSASender::deregisterGateway(const am_gatewayID_t &gatewayID)
{
    am_Error_e error = E_OK;
    if (gatewayID >= DYNAMIC_ID_BOUNDARY)
    {
        if ((error = mpShadow->deregisterGateway(gatewayID)) != E_OK)
        {
            logAmRaError("CRaALSASender::deregisterGateway", "Error on deregistering gatewayID ", gatewayID, error);
            return;
        }
        logAmRaDebug("Gateway ", gatewayID, "unregistered successfully");
    }
}

void CAmRoutingAdapterALSASender::setRoutingReady(const uint16_t handle)
{
    logAmRaInfo("CRaALSASender::setRoutingReady got called");
    mDataBase.registerDomains();
    mpShadow->confirmRoutingReady(handle, E_OK);
}

void CAmRoutingAdapterALSASender::setRoutingRundown(const uint16_t handle)
{
    assert(mpReceiveInterface);
    mDataBase.deregisterDomains();
    mpShadow->confirmRoutingRundown(handle, E_OK);
}

am_Error_e CAmRoutingAdapterALSASender::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    (void) domainState;
    if (domainID == 0)
    {
        logAmRaError("CRaALSASender::setDomainState domainID should not be ZERO");
    }
    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::asyncAbort(const am_Handle_s handle)
{
    assert(mpReceiveInterface);
    assert(handle.handle);

    /* check in list if there is an asynchronous operation in progress */
    vector<CAmRoutingAdapterALSAVolume*> volumes = mDataBase.getVolumeOpList(handle);
    if (!volumes.empty())
    {
        for (CAmRoutingAdapterALSAVolume * volume : volumes)
        {
            delete volume;
        }
        volumes.clear();
        mDataBase.cleanVolumeOpList(handle);
    }

    return E_OK;
}

void CAmRoutingAdapterALSASender::asyncDeleteVolume(const am_Handle_s handle, const CAmRoutingAdapterALSAVolume* reference)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert((handle.handleType == H_SETSINKVOLUME) || (handle.handleType == H_SETSOURCEVOLUME));

    vector<CAmRoutingAdapterALSAVolume*> lVolume = mDataBase.getVolumeOpList(handle);
    vector<CAmRoutingAdapterALSAVolume*>::iterator volItr = std::find(lVolume.begin(), lVolume.end(), reference);
    if (volItr != lVolume.end())
    {
        try
        {
            delete *volItr;
            lVolume.erase(volItr);
        }
        catch (exception& exc)
        {
            logAmRaError("CRaALSASender::asyncDeleteVolume Error:", exc.what());
        }
    }
    if (lVolume.empty())
    {
        mDataBase.cleanVolumeOpList(handle);
    }
}

/* PRQA: Lint Message 593: The proxy will be freed within destructor. */
/*lint -e593 */
am_Error_e CAmRoutingAdapterALSASender::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID,
                                       const am_sourceID_t sourceID, const am_sinkID_t sinkID,
                                       const am_CustomConnectionFormat_t connectionFormat)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_CONNECT);
    assert(connectionID);
    assert(sinkID);
    assert(sourceID);

    IAmRoutingAdapterALSAProxy * proxy = NULL;

    /* Check if we can take the job and find the domain */
    ra_sourceInfo_s source(sourceID);
    ra_sinkInfo_s sink(sinkID);
    ra_domainInfo_s * pDomain = mDataBase.findDomain(source, sink);
    if (pDomain == NULL)
    {
        logAmRaError("CRaALSASender::connect No domain found with same source and sink!");
        mpShadow->ackConnect(handle, connectionID, E_NON_EXISTENT);
        return E_OK;
    }

    /* Check the formats */
    const vector<am_CustomConnectionFormat_t> & lSrcFmts = source.amInfo.listConnectionFormats;
    vector<am_CustomConnectionFormat_t>::const_iterator posSrcFmt = find(lSrcFmts.begin(), lSrcFmts.end(), connectionFormat);
    const vector<am_CustomConnectionFormat_t> & lSinkFmts = sink.amInfo.listConnectionFormats;
    vector<am_CustomConnectionFormat_t>::const_iterator posSinkFmt = find(lSinkFmts.begin(), lSinkFmts.end(), connectionFormat);
    if ((posSrcFmt == lSrcFmts.end()) && (posSinkFmt == lSinkFmts.end()))
    {
        logAmRaInfo("CRaALSASender::connect Formats of source or sink not found!");
        mpShadow->ackConnect(handle, connectionID, E_WRONG_FORMAT);
        return E_OK;
    }

    if ((source.devTyp == DPS_REAL) && (sink.devTyp == DPS_REAL))
    {
        ra_proxyInfo_s * pProxy = mDataBase.findProxyInDomain(pDomain, sourceID, sinkID);
        if (pProxy != NULL)
        {
            /* Fetch format entries of table */
            uint32_t posSrc = static_cast<uint32_t>(posSrcFmt - lSrcFmts.begin());
            uint32_t posSink = static_cast<uint32_t>(posSinkFmt - lSinkFmts.begin());

            /* Evaluate the conversion of matrix to get format and rate */
            int32_t conv = pProxy->convertionMatrix[posSrc + (posSink * lSrcFmts.size())] - 1;
            if (conv < 0)
            {
                logAmRaError("CRaALSASender::connect Check list of source and sink connection formats and respective conversionMatrix of proxy");
                mpShadow->ackConnect(handle, connectionID, E_NOT_POSSIBLE);
                return E_OK;
            }
            pProxy->alsa.format = pProxy->listPcmFormats[conv];
            pProxy->alsa.rate = pProxy->listRates[conv];
            pProxy->alsa.channels = pProxy->listChannels[conv];

            /* Now we are ready to pick up the desired Backend */

            void* tempLibHandle = NULL;

            try
            {
                if (pProxy->pxyNam.empty())
                {
                    logAmRaInfo("ProxyDefault creation");
                    proxy = new CAmRoutingAdapterALSAProxyDefault(pProxy->alsa);
                }
                else
                {
                    std::string routingAdapterProxyAbsPath = mCommandLineArg.getValue() + "/lib" + pProxy->pxyNam + ".so";
                    IAmRoutingAdapterALSAProxy *(*createFunc)(const ra_Proxy_s &);
                    createFunc = getCreateFunction<IAmRoutingAdapterALSAProxy*(const ra_Proxy_s &)>(routingAdapterProxyAbsPath.c_str(), tempLibHandle);
                    proxy = createFunc(pProxy->alsa);
                    proxy->setLibHandle(tempLibHandle);
                }
            }
            catch (int err)
            {
                logAmRaError("CRaALSASender::connect Error in creation of Proxy:", strerror(err));
                mpShadow->ackConnect(handle, connectionID, E_NOT_POSSIBLE);
                return E_OK;
            }
        }
        else
        {
            logAmRaError("CRaALSASender::connect No audio proxy found for", sourceID, "to", sinkID);
            mpShadow->ackConnect(handle, connectionID, E_NOT_POSSIBLE);
            return E_OK;
        }
    }

    am_RoutingElement_s route;
    route.sinkID = sinkID;
    route.sourceID = sourceID;
    route.domainID = pDomain->domain.domainID;
    route.connectionFormat = connectionFormat;
    mDataBase.registerConnection(connectionID, route, proxy);

    logAmRaInfo("CRaALSASender::connect Connection", connectionID, "from",
            source.amInfo.name, "to", sink.amInfo.name, "registered.");
    if (proxy != NULL)
    {
        mpShadow->hookTimingInformationChanged(connectionID, proxy->getDelay());
    }

    /* Domain with source and sink not found */
    mpShadow->ackConnect(handle, connectionID, E_OK);
    return E_OK;
}
/*lint +e593 */


am_Error_e CAmRoutingAdapterALSASender::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_DISCONNECT);
    assert(connectionID);

    ra_domainInfo_s * pDomain = mDataBase.findDomainByConnection(connectionID);
    if (pDomain == NULL)
    {
        mpShadow->ackDisconnect(handle, connectionID, E_NON_EXISTENT);
        return E_OK;
    }

    IAmRoutingAdapterALSAProxy * pProxy = mDataBase.getProxyOfConnection(connectionID);
    if (pProxy)
    {
        void *dlhandle = pProxy->getLibHandle();
        delete pProxy;
        if (dlhandle)
        {
            dlclose(dlhandle);
        }
    }
    mDataBase.deregisterConnection(connectionID);

    logAmRaInfo("CRaALSASender::asyncDisconnect Connection", connectionID, "disconnected");
    mpShadow->ackDisconnect(handle, connectionID, E_OK);
    return E_OK;
}


am_Error_e CAmRoutingAdapterALSASender::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume,
                                             const am_CustomRampType_t ramp, const am_time_t rampTime)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSINKVOLUME);
    assert(sinkID);

    if (rampTime > MAX_RAMP_TIME)
    {
        logAmRaError("CRaALSASender::asyncSetSinkVolume ramptime", rampTime, "is out of range");
        mpShadow->ackSetSinkVolumeChange(handle, 0, E_OUT_OF_RANGE);
        return E_OK;
    }

    if ((volume < AM_MUTE) || (volume > 0))
    {
        logAmRaError("CRaALSASender::asyncSetSinkVolume", volume, "is out of range");
        mpShadow->ackSetSinkVolumeChange(handle, 0, E_OUT_OF_RANGE);
        return E_OK;
    }

    ra_domainInfo_s * pDomain = mDataBase.findDomainBySink(sinkID);
    if (pDomain == NULL)
    {
        mpShadow->ackSetSinkVolumeChange(handle, volume, E_NON_EXISTENT);
        return E_OK;
    }

    /* find the sink and request the configuration settings */
    vector<ra_sinkInfo_s>::iterator itSink =
            std::find_if(pDomain->lSinkInfo.begin(), pDomain->lSinkInfo.end(), ra_sinkInfo_s(sinkID));
    if (itSink == pDomain->lSinkInfo.end())
    {
        logAmRaError("CRaALSASender::asyncSetSinkVolume no volume element configured", itSink->pcmNam, itSink->volNam);
        mpShadow->ackSetSinkVolumeChange(handle, volume, E_NON_EXISTENT);
        return E_OK;
    }

    if (volume != itSink->amInfo.volume)
    {
        try
        {
            am_Volumes_s volumes;
            volumes.volumeID.sink = sinkID;
            volumes.volumeType = VT_SINK;
            volumes.volume = volume;
            volumes.ramp = ramp;
            volumes.time = rampTime;

            CAmRoutingAdapterALSAVolume* pVolume = new CAmRoutingAdapterALSAVolume(handle, volumes,
                    itSink->pcmNam, itSink->volNam, mpReceiveInterface, mpSocketHandler, this);
            pVolume->startFading();
            mDataBase.registerVolumeOp(handle, pVolume);
            itSink->amInfo.volume = volume;
        }
        catch (exception& exc)
        {
            logAmRaError("CRaALSASender::asyncSetSinkVolume Creation failed for", itSink->pcmNam, "->", itSink->volNam, "with", exc.what());
            mpShadow->ackSetSinkVolumeChange(handle, volume, E_UNKNOWN);
        }
    }
    else
    {
        mpShadow->ackSetSinkVolumeChange(handle, volume, E_OK);
    }

    return E_OK;
}


am_Error_e CAmRoutingAdapterALSASender::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID,
        const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t rampTime)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSOURCEVOLUME);
    assert(sourceID);

    if (rampTime > MAX_RAMP_TIME)
    {
        logAmRaInfo("CRaALSASender::asyncSetSourceVolume ramptime", rampTime, "is out of range");
        mpShadow->ackSetSourceVolumeChange(handle, 0, E_OUT_OF_RANGE);
        return E_OK;
    }

    if ((volume < AM_MUTE) || (volume > 0))
    {
        logAmRaInfo("CRaALSASender::asyncSetSourceVolume", volume, "is out of range");
        mpShadow->ackSetSourceVolumeChange(handle, 0, E_OUT_OF_RANGE);
        return E_OK;
    }

    ra_domainInfo_s * pDomain = mDataBase.findDomainBySource(sourceID);
    if (pDomain == NULL)
    {
        mpShadow->ackSetSourceVolumeChange(handle, volume, E_NON_EXISTENT);
        return E_OK;
    }

    /* find the sink and request the configuration settings */
    vector<ra_sourceInfo_s>::iterator itSrc =
            std::find_if(pDomain->lSourceInfo.begin(), pDomain->lSourceInfo.end(), ra_sourceInfo_s(sourceID));
    if (itSrc == pDomain->lSourceInfo.end())
    {
        logAmRaInfo("CRaALSASender::asyncSetSourceVolume no volume element configured", itSrc->pcmNam, itSrc->volNam);
        mpShadow->ackSetSourceVolumeChange(handle, volume, E_NON_EXISTENT);
        return E_OK;
    }

    if (volume != itSrc->amInfo.volume)
    {
        try
        {
            am_Volumes_s volumes;
            volumes.volumeID.sink = sourceID;
            volumes.volumeType = VT_SOURCE;
            volumes.volume = volume;
            volumes.ramp = ramp;
            volumes.time = rampTime;

            CAmRoutingAdapterALSAVolume* pVolume = new CAmRoutingAdapterALSAVolume(handle, volumes,
                    itSrc->pcmNam, itSrc->volNam, mpReceiveInterface, mpSocketHandler, this);
            pVolume->startFading();
            mDataBase.registerVolumeOp(handle, pVolume);
            itSrc->amInfo.volume = volume;
        }
        catch (exception& exc)
        {
            logAmRaError("CRaALSASender::asyncSetSourceVolume Creation failed for", itSrc->pcmNam, "->", itSrc->volNam, "with", exc.what());
            mpShadow->ackSetSourceVolumeChange(handle, volume, E_UNKNOWN);
        }
    }
    else
    {
        mpShadow->ackSetSourceVolumeChange(handle, volume, E_OK);
    }

    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID,
                                              const am_SourceState_e state)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSOURCESTATE);
    assert(sourceID);

    /* check if we can take the job */
    ra_domainInfo_s * pDomain = mDataBase.findDomainBySource(sourceID);
    if (pDomain == NULL)
    {
        mpShadow->ackSetSourceState(handle, E_NON_EXISTENT);
        return E_OK;
    }

    vector<ra_sourceInfo_s>::iterator itSrc =
            std::find_if(pDomain->lSourceInfo.begin(), pDomain->lSourceInfo.end(), ra_sourceInfo_s(sourceID));
    if (itSrc == pDomain->lSourceInfo.end())
    {
        mpShadow->ackSetSourceState(handle, E_NON_EXISTENT);
        return E_OK;
    }

    am_Error_e error = E_OK;
    am_Source_s & source = itSrc->amInfo;

    IAmRoutingAdapterALSAProxy *proxy = mDataBase.getProxyOfConnection(mDataBase.findConnectionFromSource(pDomain->domain.domainID, sourceID));
    if (proxy)
    {
        switch (state)
        {
            case SS_ON:
                if (proxy->openStreaming() == E_OK)
                {
                    error = proxy->startStreaming();
                }
                break;
            case SS_OFF:
                error = proxy->closeStreaming();
                break;
            case SS_PAUSED:
                error = proxy->stopStreaming();
                break;
            default:
                break;
        }
    }

    if (error == E_OK)
    {
        source.sourceState = state;
    }

    mpShadow->ackSetSourceState(handle, error);
    return E_OK;
}


am_Error_e CAmRoutingAdapterALSASender::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID,
                                                    const am_SoundProperty_s& soundProperty)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSINKSOUNDPROPERTY);
    assert(sinkID);

    /* check if we can take the job */
    ra_domainInfo_s * pDomain = mDataBase.findDomainBySink(sinkID);
    if (pDomain == NULL)
    {
        mpShadow->ackSetSinkSoundProperty(handle, E_NON_EXISTENT);
        return E_OK;
    }

    vector<ra_sinkInfo_s>::iterator itSink =
            std::find_if(pDomain->lSinkInfo.begin(), pDomain->lSinkInfo.end(), ra_sinkInfo_s(sinkID));
    if (itSink == pDomain->lSinkInfo.end())
    {
        mpShadow->ackSetSinkSoundProperty(handle, E_NON_EXISTENT);
        return E_OK;
    }

    am_SoundPropertyMapping_s soundPropertyMapping;
    soundPropertyMapping.type = soundProperty.type;
    soundPropertyMapping.value = soundProperty.value;
    for (auto it = itSink->mapSoundPropertiesToMixer[soundPropertyMapping].begin(); it != itSink->mapSoundPropertiesToMixer[soundPropertyMapping].end(); it++)
    {
        CAmRoutingAdapterALSAMixerCtrl mixer;
        int err = mixer.openMixer(itSink->pcmNam, it->mixerName);
        if (err < 0)
        {
            logAmRaError(mixer.getStrError());
            mpShadow->ackSetSinkSoundProperty(handle, E_UNKNOWN);
            return E_OK;
        }

        if (!isNumber(it->value))
        {
            /* String is not a long value, then could be an Enum, let's check for that */
            err = mixer.setEnum(it->value);
            if (err < 0)
            {
                logAmRaError(mixer.getStrError());
                mpShadow->ackSetSinkSoundProperty(handle, E_UNKNOWN);
                return E_OK;
            }
        }
        else
        {
            long numValue;
            std::istringstream toLong(it->value);
            toLong >> numValue;
            err = mixer.setVolume(numValue);
            if (err < 0)
            {
                logAmRaError(mixer.getStrError());
                mpShadow->ackSetSinkSoundProperty(handle, E_UNKNOWN);
                return E_OK;
            }
        }

        for (vector<am_SoundProperty_s>::reference prop : itSink->amInfo.listSoundProperties)
        {
            if (prop.type == soundProperty.type)
            {
                prop.value = soundProperty.value;
                break;
            }
        }
    }

    mpShadow->ackSetSinkSoundProperty(handle, E_OK);
    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID,
                                                      const vector<am_SoundProperty_s>& listSoundProperties)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSINKSOUNDPROPERTIES);
    assert(sinkID);
    assert(!listSoundProperties.empty());

    for (const am_SoundProperty_s soundProperty: listSoundProperties)
    {
        asyncSetSinkSoundProperty(handle, sinkID, soundProperty);
    }
    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID,
                                                      const am_SoundProperty_s& soundProperty)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSOURCESOUNDPROPERTY);
    assert(sourceID);

    (void)soundProperty;

    /* check if we can take the job */
    ra_domainInfo_s * pDomain = mDataBase.findDomainBySource(sourceID);
    if (pDomain == NULL)
    {
        mpShadow->ackSetSourceSoundProperty(handle, E_NON_EXISTENT);
        return E_OK;
    }

    vector<ra_sourceInfo_s>::iterator itSrc =
            std::find_if(pDomain->lSourceInfo.begin(), pDomain->lSourceInfo.end(), ra_sourceInfo_s(sourceID));
    if (itSrc == pDomain->lSourceInfo.end())
    {
        mpShadow->ackSetSourceSoundProperty(handle, E_NON_EXISTENT);
        return E_OK;
    }

    am_SoundPropertyMapping_s soundPropertyMapping;
    soundPropertyMapping.type = soundProperty.type;
    soundPropertyMapping.value = soundProperty.value;
    for (auto it = itSrc->mapSoundPropertiesToMixer[soundPropertyMapping].begin(); it != itSrc->mapSoundPropertiesToMixer[soundPropertyMapping].end(); it++)
    {
        CAmRoutingAdapterALSAMixerCtrl mixer;
        int err = mixer.openMixer(itSrc->pcmNam, it->mixerName);
        if (err < 0)
        {
            logAmRaError(mixer.getStrError());
            mpShadow->ackSetSourceSoundProperty(handle, E_UNKNOWN);
            return E_OK;
        }

        if (!isNumber(it->value))
        {
            /* String is not a long value, then could be an Enum, let's check for that */
            err = mixer.setEnum(it->value);
            if (err < 0)
            {
                logAmRaError(mixer.getStrError());
                mpShadow->ackSetSourceSoundProperty(handle, E_UNKNOWN);
                return E_OK;
            }
        }
        else
        {
            long numValue;
            std::istringstream toLong(it->value);
            toLong>>numValue;
            err = mixer.setVolume(numValue);
            if (err < 0)
            {
                logAmRaError(mixer.getStrError());
                mpShadow->ackSetSourceSoundProperty(handle, E_UNKNOWN);
                return E_OK;
            }
        }

        for (vector<am_SoundProperty_s>::reference prop : itSrc->amInfo.listSoundProperties)
        {
            if (prop.type == soundProperty.type)
            {
                prop.value = soundProperty.value;
                break;
            }
        }
    }
    mpShadow->ackSetSourceSoundProperty(handle, E_OK);
    return E_OK;
}

bool CAmRoutingAdapterALSASender::isNumber(const std::string& value)
{
    char *end_ptr;
    long numericValue = strtol(value.c_str(), &end_ptr, 10);
    if(*end_ptr)
    {
        return false;
    }
    else
    {
        return true;
    }
}


am_Error_e CAmRoutingAdapterALSASender::asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID,
                                                        const vector<am_SoundProperty_s>& listSoundProperties)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSOURCESOUNDPROPERTIES);
    assert(sourceID);
    assert(!listSoundProperties.empty());

    for (const am_SoundProperty_s soundProperty: listSoundProperties)
    {
        asyncSetSourceSoundProperty(handle, sourceID, soundProperty);
    }
    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID,
                                         const am_HotSink_e hotSink, const am_CustomRampType_t rampType, const am_time_t rampTime)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_CROSSFADE);
    assert(crossfaderID);
    assert(hotSink);
    assert(rampType);
    assert(rampTime);

    mpShadow->ackCrossFading(handle, hotSink, E_NON_EXISTENT);
    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::asyncSetVolumes(const am_Handle_s handle, const vector<am_Volumes_s> & volumes)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETVOLUMES);
    assert(!volumes.empty());

    logAmRaInfo("CRaALSASender::asyncSetVolumes Running...");
    for (const am_Volumes_s & volume : volumes)
    {
        if (volume.volumeType == VT_SOURCE)
        {
            asyncSetSourceVolume(handle, volume.volumeID.source, volume.volume, volume.ramp, volume.time);
        }
        else
        {
            asyncSetSinkVolume(handle, volume.volumeID.sink, volume.volume, volume.ramp, volume.time);
        }
    }
    logAmRaInfo("CRaALSASender::asyncSetVolumes ...ends.");

    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::asyncSetSinkNotificationConfiguration(
        const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSINKNOTIFICATION);
    assert(sinkID);

    (void)notificationConfiguration;

    mpShadow->ackSinkNotificationConfiguration(handle, E_NON_EXISTENT);
    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::asyncSetSourceNotificationConfiguration(
        const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    assert(mpReceiveInterface);
    assert(handle.handle);
    assert(handle.handleType == H_SETSOURCENOTIFICATION);
    assert(sourceID);

    (void)notificationConfiguration;

    mpShadow->ackSourceNotificationConfiguration(handle, E_NON_EXISTENT);
    return E_OK;
}

am_Error_e CAmRoutingAdapterALSASender::resyncConnectionState(const am_domainID_t domainID, std::vector<am_Connection_s>& listOfExistingConnections)
{
    return E_OK;
}

