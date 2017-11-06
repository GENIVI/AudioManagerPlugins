/*
 * CAmConnectionManager.h
 *
 *  Created on: Jun 15, 2017
 *      Author: al1
 */

#ifndef CAMCONNECTIONMANAGER_H_
#define CAMCONNECTIONMANAGER_H_

#include <memory>
#include <mutex>
#include <unordered_map>

#include <audiomanagertypes.h>

#include "CAmMainloopPulse.h"

class CAmConnectionElement
{
public:

    static CAmConnectionElement* createConnectionElement(CAmMainloopPulse& pulse,
                                                         uint32_t sourceIdx, bool isSourceVirtual,
                                                         uint32_t sinkIdx, bool isSinkVirtual);

    CAmConnectionElement(CAmMainloopPulse& pulse, uint32_t sourceIdx, uint32_t sinkIdx);
    virtual ~CAmConnectionElement() {}

    virtual void connect() = 0;
    virtual void disconnect() = 0;

protected:
    CAmMainloopPulse&   mPulse;
    uint32_t            mSourceIdx;
    uint32_t            mSinkIdx;
};

class CAmConnectionSinkInput2Sink : public CAmConnectionElement
{
public:
    CAmConnectionSinkInput2Sink(CAmMainloopPulse& pulse, uint32_t sourceIdx, uint32_t sinkIdx);
    ~CAmConnectionSinkInput2Sink();

    void connect() override;
    void disconnect() override;
};

class CAmConnectionSource2SourceOutput : public CAmConnectionElement
{
public:
    CAmConnectionSource2SourceOutput(CAmMainloopPulse& pulse, uint32_t sourceIdx, uint32_t sinkIdx);
    ~CAmConnectionSource2SourceOutput();

    void connect() override;
    void disconnect() override;
};

class CAmConnectionManager
{
public:
    CAmConnectionManager(CAmMainloopPulse& pulse);

    void newSource(am::am_sourceID_t sourceID, uint32_t idx);
    void removeSource(uint32_t idx);
    void newSinkInput(am::am_sourceID_t sourceID, uint32_t idx);
    void removeSinkInput(uint32_t idx);
    void newSink(am::am_sinkID_t sinkID, uint32_t idx);
    void removeSink(uint32_t idx);
    void newSourceOutput(am::am_sinkID_t sinkID, uint32_t idx);
    void removeSourceOutput(uint32_t idx);

    am::am_Error_e registerSource(am::am_sourceID_t sourceID);
    am::am_Error_e deregisterSource(am::am_sourceID_t sourceID);
    am::am_Error_e registerSink(am::am_sinkID_t sinkID);
    am::am_Error_e deregisterSink(am::am_sinkID_t sinkID);

    am::am_Error_e connect(am::am_sourceID_t sourceID, am::am_sinkID_t sinkID, am::am_connectionID_t connectionID);
    am::am_Error_e disconnect(am::am_connectionID_t connectionID);

private:
    bool isSourceRegistered(am::am_sourceID_t sourceID) { return mSources.find(sourceID) != mSources.end(); }
    bool isSinkRegistered(am::am_sinkID_t sinkID) { return mSinks.find(sinkID) != mSinks.end(); }
    void handleNewSource(am::am_sourceID_t sourceID, uint32_t idx, bool isVirtual);
    void handleNewSink(am::am_sinkID_t sinkID, uint32_t idx, bool isVirtual);
    void handleRemoveSource(uint32_t idx, bool isVirtual);
    void handleRemoveSink(uint32_t idx, bool isVirtual);
    am::am_connectionID_t findConnectionBySourceID(am::am_sourceID_t sourceID);
    am::am_connectionID_t findConnectionBySinkID(am::am_sinkID_t sinkID);
    void updateConnection(am::am_connectionID_t connectionID);

    struct rp_SourceConnection_s
    {
        uint32_t idx;
        bool isVirtual;
    };

    struct rp_SinkConnection_s
    {
        uint32_t idx;
        bool isVirtual;
    };

    struct rp_Connection_s
    {
        am::am_sourceID_t                       sourceID;
        am::am_sinkID_t                         sinkID;
        std::shared_ptr<CAmConnectionElement>   connection;
    };

    CAmMainloopPulse&   mPulse;
    std::mutex          mMutex;

    std::unordered_map<am::am_sourceID_t, rp_SourceConnection_s> mSources;
    std::unordered_map<am::am_sinkID_t, rp_SinkConnection_s> mSinks;
    std::unordered_map<am::am_connectionID_t, rp_Connection_s> mConnections;
};

#endif /* CAMCONNECTIONMANAGER_H_ */
