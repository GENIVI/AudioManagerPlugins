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
 *                             Porting from AM ver 1.0;
 */

/*Includes*/
#include "ControlSender.h"
#include "ControlConfig.h"
#include "IAmControl.h"
#include "CAmDltWrapper.h"

#include <cassert>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


/*Defines*/
#define LIBNAMECTL "libPluginControlInterface.so"
#define CFGNAMECTL "libPluginControlInterface.conf"

DLT_DECLARE_CONTEXT(controlSender)

using namespace am;

extern "C" IAmControlSend* PluginControlInterfaceFactory()
{
    return (new ControlSenderPlugin());
}

extern "C" void destroyControlPluginInterface(IAmControlSend* controlSendInterface)
{
    delete controlSendInterface;
}

ControlSenderPlugin::ControlSenderPlugin() :
        mControlReceiveInterface(NULL)
{
}

ControlSenderPlugin::~ControlSenderPlugin()
{
}

am_Error_e ControlSenderPlugin::startupController(IAmControlReceive *controlreceiveinterface)
{
    assert(controlreceiveinterface);
    mControlReceiveInterface = controlreceiveinterface;
    //here is a good place to insert SystemProperties into the database...
    return E_OK;
}

void ControlSenderPlugin::setControllerReady()
{
    //here is a good place to insert Source and SinkClasses into the database...
    loadConfig();
    mControlReceiveInterface->setCommandReady();
    mControlReceiveInterface->setRoutingReady();
}

void ControlSenderPlugin::setControllerRundown(const int16_t signal)
{
    logInfo("CAmControlSenderBase::setControllerRundown() was called signal=",signal);
    if (signal==2)
        mControlReceiveInterface->confirmControllerRundown(E_UNKNOWN);

    mControlReceiveInterface->confirmControllerRundown(E_OK);
}

am_Error_e ControlSenderPlugin::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    //TODO: set sink / source volume depending on mixing rules - delta - system property
    //TODO: the mixing depends on source CLASSES?
    //
    //TODO: shall we implement some kind of source auto-exclusion ? e.g. two source from te same class are not allowed simultaneously (fifo)
    //TODO: shall we implement an action = destroy? apart from disconnect? disconnect is reversible, destroy is not reversible and the main connection is deleted?
    //TODO: maybe we simply should return E_ALREADY_EXISTS - and tis should not be a real error code (if connection already exists, maybe that's fine)

    //TODO: apply actions only on active main connections!!!!

    //TODO: interrupting sources on top of main sources -> main src:temporary disconnection;
    //TODO: main sources on top of main sources -> old main src:permanent disconnection;



    std::vector<am_Route_s> audioRoute;
    if (!this->isStaticRoutingEnabled())
    {
        this->mControlReceiveInterface->getRoute(true, sourceID, sinkID, audioRoute);
    }
    else
    {
        this->getStaticRoute(true, sourceID, sinkID, audioRoute);
    }

//TODO: if there is no static route? is there any default route ?
//TODO: if there is no static route? can we use dynamic routing ?
    logInfo("ControlSenderPlugin::hookUserConnectionRequest audioRoute is empty ? ", audioRoute.empty());
    if (audioRoute.empty())
    {
        return am::E_NOT_POSSIBLE;
    }

/*
 * Find out the source class info for the new main connection.
 * Also find out if this connection is interrupting or not
 */
    am_SourceClass_s newClassInfo;
    this->mControlReceiveInterface->getSourceClassInfoDB(sourceID, newClassInfo);

    bool interrupting = false;
    for (uint16_t i = 0; i < newClassInfo.listClassProperties.size(); i++)
    {
        if (newClassInfo.listClassProperties[i].classProperty == CP_GENIVI_SOURCE_TYPE &&
                newClassInfo.listClassProperties[i].value == SOURCE_TYPE_INTERRUPT)
        {
            interrupting = true;
        }
    }

    logInfo("{BEGIN Connection request Source class:",
            newClassInfo.name,
            " [ sourceID:", sourceID, ", sinkID:", sinkID, "] - interrupting:" ,interrupting);

    std::vector<am_MainConnection_s> existingConnections;

    this->mControlReceiveInterface->getListMainConnections(existingConnections);
    std::vector<am_MainConnection_s>::iterator iter     = existingConnections.begin();
    std::vector<am_MainConnection_s>::iterator iterEnd  = existingConnections.end();

    iter = existingConnections.begin();
/*
* iterate connections to remove already existing connections for the same source/sink
*/
    bool listChanged = false;
    for (; iter < iterEnd; ++iter)
    {
         if (iter->sinkID == sinkID && iter->sourceID == sourceID)
         {
            //main connection already exists - exclude it
             logInfo("END Connection request - ALREADY EXISTS}");
             return E_ALREADY_EXISTS;
         }
    }
    if (listChanged)
    {
        this->mControlReceiveInterface->getListMainConnections(existingConnections);
        iter     = existingConnections.begin();
        iterEnd  = existingConnections.end();
    }

//TODO: first connect everything in the audio path
//TODO: if one sub-connection is not successful - disconnect everything
//TODO: only if every sub-connection was successful - apply the mixing rules

    std::map<am_mainConnectionID_t, std::vector<am_ConnectAction_s> > mainConnection2Actions;

/***************************************************************
 * START MIXING RULES EVALUATION
 * - maintain a list of actions associated to a connection;
 *   according to the mixing rules
 * - the inverse actions shall be applied during disconnection
 ***************************************************************/

/*
 * Iterate audio mixing rules, then iterate existing connections;
 * if the new source class match the mixing source class
 * find out if any action should be taken, find out the target
 * of the action -> domain/sink/source.
 */
    for (uint16_t i = 0; i < mMixingRules.size(); i++)
    {
/*
 * the source class of the new connection matches the source class of the mixing rule
 */
        if (newClassInfo.sourceClassID == mMixingRules[i].mixingSourceClassID)
        {
            iter = existingConnections.begin();
/*
 * iterate main connections
 */
            for (; iter < iterEnd; ++iter)
            {
                am_SourceClass_s    srcClassOfConn;
                am_sinkID_t         sinkIdOfConn = iter->sinkID;

                this->mControlReceiveInterface->getSourceClassInfoDB(
                        iter->sourceID, srcClassOfConn);

                am_sourceClass_t    srcClassIdOfConn = srcClassOfConn.sourceClassID;
/*
 * iterate the actions associated to the mixing rule
 */
                for (uint16_t j = 0; j < mMixingRules[i].actions.size(); j++)
                {
/*
 * if the source class of the action in the mixing rule matches the source class
   of an existing connection AND the sink of the action in the mixing rule matches
   the sink of an existing connection - then the action must be applied on
   existing connection
 */
                    if (mMixingRules[i].actions[j].activeSourceClassID == srcClassIdOfConn &&
                        mMixingRules[i].actions[j].activeSinkID == sinkIdOfConn)
                    {
                        mainConnection2Actions[iter->mainConnectionID].push_back(mMixingRules[i].actions[j]);
                        logInfo("Apply action on existing connection:", iter->mainConnectionID,
                                "caused by new connection - unknow ID yet");
                        this->applyActionOnConnection(iter->mainConnectionID, mMixingRules[i].actions[j], interrupting, false);

                    }//end if (existingClassInfo.name.compare(
                }//end for actionsCount
            }//end for existingConnections
        }//end if (newClassInfo.name.compare(mixingRules[i].sourceClassName
    }//end for mixingRules

/**
 * After actions cause by the new connections were applied on existing connections,
 * connect all elements in the route to create sub-connections;
 * for each connection, store the reference in: mConnectionID2Subconnections
 */
    //for static routing - only one route in the list of audioRoute
    std::vector<am_RoutingElement_s>::iterator iterRoute = audioRoute[0].route.begin();
    std::vector<am_RoutingElement_s>::iterator iterRouteEnd = audioRoute[0].route.end();

    std::vector<am_Connection_s> subconnections;
    am_MainConnection_s mainConnection;

    for (; iterRoute < iterRouteEnd; iterRoute++)
    {
        am_Handle_s h;

        am_mainConnectionID_t subConnectionID;

        am_Error_e ret = mControlReceiveInterface->connect(
                h, subConnectionID, iterRoute->connectionFormat,
                iterRoute->sourceID, iterRoute->sinkID);

        am_SourceClass_s    srcClassOfConn;

        this->mControlReceiveInterface->getSourceClassInfoDB(
                iterRoute->sourceID, srcClassOfConn);

        logInfo("Subconnection created: subConnectionID", subConnectionID);
        logInfo("Subconnection: sourceID:", iterRoute->sourceID ,
                ", sourceClassID:", srcClassOfConn.sourceClassID,
                ", sinkID:", iterRoute->sinkID, "]");

        if (ret == E_OK || ret == E_ALREADY_EXISTS)
        {
            am_Connection_s tempConnection;
            tempConnection.sinkID = iterRoute->sinkID;
            tempConnection.sourceID = iterRoute->sourceID;
            tempConnection.connectionID = subConnectionID;

            subconnections.push_back(tempConnection);
            mainConnection.listConnectionID.push_back(tempConnection.connectionID);
        }
        else
        {
            //TODO: disconnect already connected in case of error
            logInfo("END Connection request with error code:", ret, "}");
            return ret;
        }
    }

/*
 * Once the sub-connections were created, we are ready to store the main connections
 */

    mainConnection.mainConnectionID = 0;
    mainConnection.connectionState = CS_CONNECTED;
    mainConnection.sinkID = sinkID;
    mainConnection.sourceID = sourceID;
    am_Error_e ret = this->mControlReceiveInterface->enterMainConnectionDB(
            mainConnection, mainConnectionID);
    mainConnection.mainConnectionID = mainConnectionID;

    this->mConnectionID2Subconnections[mainConnectionID] = subconnections;
    this->mConnectionID2Actions[mainConnectionID] = mainConnection2Actions;

    if (ret != E_OK)
    {
        logInfo("END Connection request with error code:", ret, "}");
        return ret;
    }
    else
    {
        logInfo("Mainconnection: mainConnectionID", mainConnectionID);
    }
    //TODO - check the status


/*
 * Iterate audio mixing rules, then iterate actions,
 * if the source class of an action matches the new source class name
 * find out if there is an existing connection matching the mixingSourceClass
 */
    for (uint16_t i = 0; i < mMixingRules.size(); i++)
    {
        for (uint16_t j = 0; j < mMixingRules[i].actions.size(); j++)
        {
/*
 * if the source class of the action in the mixing rule matches the source class
   of an existing connection AND the sink of the action in the mixing rule matches
   the sink of an existing connection - then the action must be applied on
   existing connection
 */
            if (mMixingRules[i].actions[j].activeSourceClassID == newClassInfo.sourceClassID)
            {
                iter = existingConnections.begin();
    /*
     * iterate connections
     */
                for (; iter < iterEnd; ++iter)
                {
                    am_SourceClass_s srcClassOfConn;

                    this->mControlReceiveInterface->getSourceClassInfoDB(
                            iter->sourceID, srcClassOfConn);

                    am_sourceClass_t srcClassIdOfConn = srcClassOfConn.sourceClassID;
                    if (srcClassIdOfConn == mMixingRules[i].mixingSourceClassID)
                    {
                        if (this->mConnectionID2Actions[mainConnectionID].find(
                                    iter->mainConnectionID) !=
                            this->mConnectionID2Actions[mainConnectionID].end())
                        {
                            //there was already an action applied for iter->connectionID caused by mainConnection.connectionID
                            //connections should not exclude each other
                            logInfo("No action to apply on new connection:", mainConnection.mainConnectionID,
                                    "since there was already an action applyed to existing connection", iter->mainConnectionID);
                        }
                        else
                        {
                            logInfo("Apply action on new connection:", mainConnection.mainConnectionID,
                                    "caused by existing connection", iter->mainConnectionID);

                            interrupting = false;
                            for (uint16_t x = 0; x < srcClassOfConn.listClassProperties.size(); x++)
                            {
                                if (srcClassOfConn.listClassProperties[x].classProperty == CP_GENIVI_SOURCE_TYPE &&
                                    srcClassOfConn.listClassProperties[x].value == SOURCE_TYPE_INTERRUPT)
                                {
                                    interrupting = true;
                                }
                            }

                            this->mConnectionID2Actions[iter->mainConnectionID][mainConnectionID].push_back(mMixingRules[i].actions[j]);
                            this->applyActionOnConnection(mainConnection.mainConnectionID, mMixingRules[i].actions[j], interrupting, false);
                        }
                    }
                }
            }
        }
    }

    logInfo("END Connection request - SUCCESS}");
    return E_OK;
}


am_Error_e ControlSenderPlugin::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
    am_Handle_s h;

    if (connectionID == 0)
    {
        logError("Disconnect what :-o ? connection ID = 0");
        return E_NON_EXISTENT;
    }

    //TODO: restore other sources - resume ? or restore volume? - then disconnect
    am_SourceClass_s oldClassInfo;
    bool             interrupting = false;

    std::vector<am_MainConnection_s> existingConnections;

    this->mControlReceiveInterface->getListMainConnections(existingConnections);
    logInfo("{BEGIN Disconnection request: connectionID:", connectionID);
    std::vector<am_MainConnection_s>::iterator iter     = existingConnections.begin();
    std::vector<am_MainConnection_s>::iterator iterEnd  = existingConnections.end();

    for (; iter < iterEnd; ++iter)
    {
        if (iter->mainConnectionID == connectionID)
        {//check other existing connection , maybe volume should be restored
            this->mControlReceiveInterface->getSourceClassInfoDB(iter->sourceID, oldClassInfo);
            for (unsigned int i = 0; i < oldClassInfo.listClassProperties.size(); i++)
            {
                if (oldClassInfo.listClassProperties[i].classProperty == CP_GENIVI_SOURCE_TYPE &&
                    oldClassInfo.listClassProperties[i].value == SOURCE_TYPE_INTERRUPT)
                {
                    interrupting = true;
                }
            }
            break;
        }
    }

    //save the list of inverse actions
    std::map<am_mainConnectionID_t,
             std::vector<am_ConnectAction_s> > connectionID2Actions = this->mConnectionID2Actions[connectionID];

    this->mConnectionID2Actions[connectionID].clear();

    //apply the inverse action - of actions caused by this connection
    iter = existingConnections.begin();
    for (; iter < iterEnd; ++iter)
    {
        logInfo("Check apply inverse action on existing connection:", iter->mainConnectionID,
                "caused by disconnecion connection", connectionID);

        std::vector<am_ConnectAction_s>::iterator actionsIter
                = connectionID2Actions[iter->mainConnectionID].begin();
        std::vector<am_ConnectAction_s>::iterator actionsIterEnd
                = connectionID2Actions[iter->mainConnectionID].end();

        for (; actionsIter < actionsIterEnd; actionsIter++)
        {
            //apply the inverse action
            logInfo("Apply inverse action on existing connection:", iter->mainConnectionID,
                    "caused by disconnecion connection", connectionID);

            applyActionOnConnection(iter->mainConnectionID, *actionsIter, interrupting, true);
        }//end for actions
    }//end for existing connections


    //apply the inverse action - of actions applyed to this connection
    iter = existingConnections.begin();
    for (; iter < iterEnd; ++iter)
    {
        std::map<am_mainConnectionID_t,
                 std::vector<am_ConnectAction_s> > connectionID2Actions = this->mConnectionID2Actions[iter->mainConnectionID];

        logInfo("Check apply inverse action on existing connection:", iter->mainConnectionID,
                "caused by disconnecion connection", connectionID);

        std::vector<am_ConnectAction_s>::iterator actionsIter
                = connectionID2Actions[connectionID].begin();
        std::vector<am_ConnectAction_s>::iterator actionsIterEnd
                = connectionID2Actions[connectionID].end();

        for (; actionsIter < actionsIterEnd; actionsIter++)
        {
            //apply the inverse action
            logInfo("Apply inverse action on disconnecion connection", connectionID);

            applyActionOnConnection(connectionID, *actionsIter, interrupting, true, true);
        }//end for actions

    }

    //iterate sub-connections and disconnect
    std::vector<am_Connection_s>::iterator subConnIter =
            this->mConnectionID2Subconnections[connectionID].begin();
    std::vector<am_Connection_s>::iterator subConnIterEnd =
            this->mConnectionID2Subconnections[connectionID].end();

    for (; subConnIter < subConnIterEnd; subConnIter++)
    {
        logInfo("Subdisconnection: subConnectionID", subConnIter->connectionID);
        mControlReceiveInterface->disconnect(h, subConnIter->connectionID);
    }

    //TODO: return value depends on all disconnect() results.
    this->mConnectionID2Subconnections[connectionID].clear();

    this->mControlReceiveInterface->changeMainConnectionStateDB(
            connectionID, CS_DISCONNECTED);
    this->mControlReceiveInterface->removeMainConnectionDB(connectionID);


    logInfo("END Disconnection request}");

    return E_OK;
}



/**
 * Find out the sub-connection target of this action
 * apply the action and return the connectionID of the affected
 * sub-connection
 */
am_Error_e ControlSenderPlugin::applyActionOnConnection(
        am_mainConnectionID_t connectionID,
        am_ConnectAction_s &action,
        bool interrupt,
        bool inverse,
        bool force)
{
    am_Error_e ret;

    /**
     * Check if current action is conflicting an existing action,
     * do not apply, or cancel existing action and apply this one.
     * if action is not inverse then:
     * - current action may not be applied
     * - current action might cause other actions to become pending
     */
    std::vector<am_MainConnection_s> existingConnections;

    this->mControlReceiveInterface->getListMainConnections(existingConnections);

    std::vector<am_MainConnection_s>::iterator iter1     = existingConnections.begin();
    std::vector<am_MainConnection_s>::iterator iter1End  = existingConnections.end();

    std::vector<am_MainConnection_s>::iterator iter2     = existingConnections.begin();
    std::vector<am_MainConnection_s>::iterator iter2End  = existingConnections.end();

    if (!force)
    {

        for (; iter1 < iter1End; ++iter1)
        {
            std::map<am_mainConnectionID_t,
                 std::vector<am_ConnectAction_s> > connectionID2Actions =
                        this->mConnectionID2Actions[iter1->mainConnectionID];

            iter2 = existingConnections.begin();
            for (; iter2 < iter2End; ++iter2)
            {
                std::vector<am_ConnectAction_s>::iterator actionsIter
                        = connectionID2Actions[iter2->mainConnectionID].begin();
                std::vector<am_ConnectAction_s>::iterator actionsIterEnd
                        = connectionID2Actions[iter2->mainConnectionID].end();

                for (; actionsIter < actionsIterEnd; actionsIter++)
                {
                    if (action.weekCompare(*actionsIter))
                    {
                        if (!inverse)
                        {
                            if (actionsIter->actionType < action.actionType)
                            {
                                logInfo("Abort action", ACTION_TYPE_STR(action.actionType),
                                        "on connection:", connectionID,
                                        "since", ACTION_TYPE_STR(actionsIter->actionType),
                                        "is active; triggering mainConnection:", iter1->mainConnectionID);
                                return am::E_ABORTED;
                            }
                        }
                        else
                        {
                            if (actionsIter->actionType < am_ConnectAction_s::inverseAction(action.actionType))
                            {
                                logInfo("Abort action", ACTION_TYPE_STR(am_ConnectAction_s::inverseAction(action.actionType)),
                                        "on connection:", connectionID,
                                        "since", ACTION_TYPE_STR(actionsIter->actionType),
                                        "is active; triggering mainConnection:", iter1->mainConnectionID);
                                return am::E_ABORTED;
                            }
                        }
                    }
                }
            }
        }

    }
    //if action is inverse then:
    //  - some pending connections might become active

    //iterate sub-connections for main connection to find out the targetElement
    std::vector<am_Connection_s>::iterator iter = mConnectionID2Subconnections[connectionID].begin();
    std::vector<am_Connection_s>::iterator iterEnd = mConnectionID2Subconnections[connectionID].end();

    for (; iter < iterEnd; iter++)
    {
        am_Connection_s currentConn = *iter;
        am_Source_s     currentSrc;

        //find out the domain of currentConn.
        this->getSourceInfo(currentConn.sourceID, currentSrc);

        if (action.targetDomain == currentSrc.domainID)
        {
            ret = applyActionOnSubConnection(
                    connectionID,
                    currentConn,
                    action,
                    interrupt,
                    inverse);
         }
    }


    iter1     = existingConnections.begin();
    iter2     = existingConnections.begin();
    //apply existing action excluded by currently disappearing action
    for (; iter1 < iter1End; ++iter1)
    {
        std::map<am_mainConnectionID_t,
             std::vector<am_ConnectAction_s> > connectionID2Actions =
                    this->mConnectionID2Actions[iter1->mainConnectionID];

        iter2 = existingConnections.begin();
        for (; iter2 < iter2End; ++iter2)
        {
            std::vector<am_ConnectAction_s>::iterator actionsIter
                    = connectionID2Actions[iter2->mainConnectionID].begin();
            std::vector<am_ConnectAction_s>::iterator actionsIterEnd
                    = connectionID2Actions[iter2->mainConnectionID].end();

            for (; actionsIter < actionsIterEnd; actionsIter++)
            {
                if (action.weekCompare(*actionsIter))
                {
                    if (inverse)
                    {
                        if (actionsIter->actionType > action.actionType)
                        {
                            applyActionOnConnection(iter2->mainConnectionID, *actionsIter, false, false);
                        }
                    }
                }
            }
        }
    }

    return ret;
}

am_Error_e ControlSenderPlugin::applyActionOnSubConnection(
        am_mainConnectionID_t connectionID,
        am_Connection_s subConnection,
        am_ConnectAction_s &action,
        bool interrupt,
        bool inverse)
{
    am_Handle_s h;
    am_ActionType_e actionType = inverse ?
                    am_ConnectAction_s::inverseAction(action.actionType) :
                    action.actionType;

    logInfo("Apply action", ACTION_TYPE_STR(actionType),
            "on sub-connectionID:", subConnection.connectionID,
            "[sourceID:", subConnection.sourceID,
            ", sinkID:", subConnection.sinkID, "]");

    //TODO: is this main connection suspended or restored?
    //TODO:  - should check all sub-connections - current implementation is wrong overall connection state is decided by one sub-connection

    switch (actionType)
    {
        case AT_CONNECT:
        {
            logInfo("Apply connect: sub-connectionID:",
                    subConnection.connectionID,
                    "[sourceID:", subConnection.sourceID,
                    ", sinkID:", subConnection.sinkID, "]");

            if (subConnection.sourceID == 0 ||
                subConnection.sinkID == 0)
            {
                logError("Can not apply re-connect sink or source not defined");
                return E_NOT_POSSIBLE;
            }
            mControlReceiveInterface->connect(h,
                    subConnection.connectionID,
                    subConnection.connectionFormat,
                    subConnection.sourceID,
                    subConnection.sinkID);

            am_Connection_s tempConnection;
            tempConnection.sinkID = subConnection.sinkID;
            tempConnection.sourceID = subConnection.sourceID;
            tempConnection.connectionID = subConnection.connectionID;

            //this old connection shall be replaced with a new one.
            this->mConnectionID2Subconnections[connectionID].push_back(
                tempConnection
            );

            //TODO: this new connection id shall replace the subconnection.

            this->mControlReceiveInterface->
                    changeMainConnectionStateDB(
                            connectionID,
                            CS_CONNECTED);
            break;
        }
        case AT_DISCONNECT:
        {
            this->mControlReceiveInterface->changeMainConnectionStateDB(
                    connectionID, CS_DISCONNECTED);

            logInfo("Apply disconnect: sub-connectionID:",
                    subConnection.connectionID,
                    "[sourceID:", subConnection.sourceID,
                    ", sinkID:", subConnection.sinkID, "]");

            mControlReceiveInterface->disconnect(h, subConnection.connectionID);

            this->mControlReceiveInterface->
                    changeMainConnectionStateDB(
                            connectionID,
                            interrupt ? CS_SUSPENDED : CS_DISCONNECTED);

            if (!interrupt)
            {
                this->mControlReceiveInterface->
                        removeMainConnectionDB(connectionID);
            }
            break;
        }
        case AT_VOLUME_DECREASE:
        {
            if (action.targetElement == ATG_SOURCE)
            {
                logInfo("Apply decrease volume: sourceID:", subConnection.sourceID);
                mControlReceiveInterface->setSourceVolume(
                        h, subConnection.sourceID, 50, am::RAMP_GENIVI_NO_PLOP, 0);
            }
            if (action.targetElement == ATG_SINK)
            {
                logInfo("Apply decrease volume: sinkID:", subConnection.sinkID);
                mControlReceiveInterface->setSinkVolume(
                        h, subConnection.sinkID, 50, am::RAMP_GENIVI_NO_PLOP, 0);
            }
        }
        break;

        case AT_VOLUME_INCREASE:
        {
            if (action.targetElement == ATG_SOURCE)
            {
                logInfo("Apply increase volume: sourceID:", subConnection.sourceID);
                mControlReceiveInterface->setSourceVolume(
                        h, subConnection.sourceID, 100, am::RAMP_GENIVI_NO_PLOP, 0);
            }
            if (action.targetElement == ATG_SINK)
            {
                logInfo("Apply increase volume: sinkID:", subConnection.sinkID);
                mControlReceiveInterface->setSinkVolume(
                        h, subConnection.sinkID, 100, am::RAMP_GENIVI_NO_PLOP, 0);
            }
            break;
        }
        case AT_MUTE:
        {
            if (action.targetElement == ATG_SOURCE)
            {
                logInfo("Apply mute: sourceID:", subConnection.sourceID);
                mControlReceiveInterface->setSourceVolume(
                        h, subConnection.sourceID, 0, am::RAMP_GENIVI_NO_PLOP, 0);
            }
            if (action.targetElement == ATG_SINK)
            {
                //TODO: replace setVolume(0) with a real setMute function
                logInfo("Apply mute: sinkID:", subConnection.sinkID);
                mControlReceiveInterface->setSinkVolume(
                        h, subConnection.sinkID, 0, am::RAMP_GENIVI_NO_PLOP, 0);
            }

            this->mControlReceiveInterface->
                    changeMainConnectionStateDB(
                            connectionID,
                            CS_SUSPENDED);
            break;
        }
         case AT_UNMUTE:
        {
            if (action.targetElement == ATG_SOURCE)
            {
                logInfo("Apply unmute: sourceID:", subConnection.sourceID);
                mControlReceiveInterface->setSourceVolume(
                        h, subConnection.sourceID, 100, am::RAMP_GENIVI_NO_PLOP, 0);
            }
            if (action.targetElement == ATG_SINK)
            {
                //TODO: replace setVolume(100) with a real setMute function
                logInfo("Apply unmute: sinkID:", subConnection.sinkID);
                mControlReceiveInterface->setSinkVolume(
                        h, subConnection.sinkID, 100, am::RAMP_GENIVI_NO_PLOP, 0);
            }

            this->mControlReceiveInterface->
                    changeMainConnectionStateDB(
                            connectionID,
                            CS_CONNECTED);
        }

        //TODO: case for PAUSE/RESUME
        default:
            break;
    }//end switch (mixingRules[i].actions[j].actionType
  return E_OK;
}

am_Error_e ControlSenderPlugin::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
    (void) sinkID;
    (void) soundProperty;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
    (void) sourceID;
    (void) soundProperty;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
    (void) property;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookUserVolumeChange(const am_sinkID_t SinkID, const am_mainVolume_t newVolume)
{
    am_Handle_s h;
    return this->mControlReceiveInterface->setSinkVolume(
            h, SinkID, newVolume, am::RAMP_GENIVI_NO_PLOP, 0);
}

am_Error_e ControlSenderPlugin::hookUserVolumeStep(const am_sinkID_t SinkID, const int16_t increment)
{
    (void) SinkID;
    (void) increment;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    (void) sinkID;
    (void) muteState;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    am_Error_e retVal = mControlReceiveInterface->enterDomainDB(domainData, domainID);
    if (retVal == E_OK) {
        this->mNames2Domains[domainData.name] = domainData;
        this->mNames2Domains[domainData.name].domainID = domainID;
    }
    logInfo("Register Domain: name:" , domainData.name, ", domainId:", domainID);
    return retVal;
}

am_Error_e ControlSenderPlugin::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
    (void) domainID;
    return E_NOT_USED;
}

void ControlSenderPlugin::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
/*
 * refresh config after each domain registration.
 * elements already registered will not register again
*/
    logInfo("Domain registration completed", domainID);
    loadConfig();
}

am_Error_e ControlSenderPlugin::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    logInfo("Register Sink: name:" , sinkData.name, ", sinkId:", sinkID);
    am_Error_e  retVal =  mControlReceiveInterface->enterSinkDB(sinkData, sinkID);
    if (retVal == E_OK) {
        this->mNames2Sinks[sinkData.name] = sinkData;
        this->mNames2Sinks[sinkData.name].sinkID = sinkID;
    }
    logInfo("Register Sink: name:" , sinkData.name, ", sinkId:", sinkID);
    return retVal;
}

am_Error_e ControlSenderPlugin::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
    (void) sinkID;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    am_Error_e retVal = mControlReceiveInterface->enterSourceDB(sourceData, sourceID);
    if (retVal == E_OK)
    {
        this->mNames2Sources[sourceData.name] = sourceData;
        this->mNames2Sources[sourceData.name].sourceID = sourceID;
    }
    logInfo("Register Source: name:" , sourceData.name, ", sourceId:", sourceID);
    return retVal;
}

am_Error_e ControlSenderPlugin::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
    (void) sourceID;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    (void) gatewayData;
    (void) gatewayID;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookSystemRegisterConverter(const am_Converter_s & converterData, am_converterID_t & converterID)
{
    (void) converterData;
    (void) converterID;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
    (void) gatewayID;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookSystemDeregisterConverter(const am_converterID_t converterID)
{
    (void) converterID;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    (void) crossfaderData;
    (void) crossfaderID;
    return E_NOT_USED;
}

am_Error_e ControlSenderPlugin::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
    (void) crossfaderID;
    return E_NOT_USED;
}

void ControlSenderPlugin::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    (void) handle;
    (void) sinkID;
    (void) volume;
}

void ControlSenderPlugin::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    (void) handle;
    (void) sourceID;
    (void) volume;
}

void ControlSenderPlugin::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    (void) sourceID;
    (void) interruptState;
}

void ControlSenderPlugin::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    (void) sinkID;
    (void) availability;
}

void ControlSenderPlugin::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    (void) sourceID;
    (void) availability;
}

void ControlSenderPlugin::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
    (void) domainID;
    (void) state;
}

void ControlSenderPlugin::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
    (void) data;
}

void ControlSenderPlugin::hookSystemSpeedChange(const am_speed_t speed)
{
    (void) speed;
}

void ControlSenderPlugin::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
    (void) mainConnectionID;
    (void) time;
}

void ControlSenderPlugin::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
    (void) handle;
    (void) errorID;
}

void ControlSenderPlugin::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
    (void) handle;
    (void) errorID;
}

void ControlSenderPlugin::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
    (void) handle;
    (void) hostsink;
    (void) error;
}

void ControlSenderPlugin::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    (void) error;
    (void) volume;
    (void) handle;
}

void ControlSenderPlugin::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t voulme, const am_Error_e error)
{
    (void) error;
    (void) voulme;
    (void) handle;
}

void ControlSenderPlugin::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void ControlSenderPlugin::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void ControlSenderPlugin::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void ControlSenderPlugin::cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void ControlSenderPlugin::cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

am_Error_e ControlSenderPlugin::getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_CustomConnectionFormat_t> listPossibleConnectionFormats, std::vector<am_CustomConnectionFormat_t> & listPrioConnectionFormats)
{
    (void) sourceID;
    (void) sinkID;
    (void) listRoute;
    //ok, this is cheap. In a real product you have your preferences, right?
    listPrioConnectionFormats = listPossibleConnectionFormats;
    return (E_OK);
}

void ControlSenderPlugin::getInterfaceVersion(std::string & version) const
{
    version = "3.0";
}

void ControlSenderPlugin::confirmCommandReady(const am_Error_e error)
{
    (void) error;
    logInfo("ControlSenderPlugin got Routing Ready confirmed");
}

void ControlSenderPlugin::confirmRoutingReady(const am_Error_e error)
{
    (void) error;
    logInfo("ControlSenderPlugin got Command Ready confirmed");
}

void ControlSenderPlugin::confirmCommandRundown(const am_Error_e error)
{
    (void) error;
    logInfo("ControlSenderPlugin got Routing Rundown confirmed");
}

void ControlSenderPlugin::confirmRoutingRundown(const am_Error_e error)
{
    (void) error;
    logInfo("ControlSenderPlugin got Command Rundown confirmed");
}

am_Error_e ControlSenderPlugin::getStaticRoute(
        const bool onlyfree,
        const am_sourceID_t sourceID,
        const am_sinkID_t sinkID,
        std::vector<am_Route_s>& returnList)
{
    std::vector<am_Route_s>::iterator iter = this->mStaticRoutes.begin();
    std::vector<am_Route_s>::iterator iterEnd = this->mStaticRoutes.end();
    for (; iter < iterEnd; iter++)
    {
        if (iter->sourceID == sourceID &&
            iter->sinkID == sinkID)
        {
            returnList.push_back(*iter);
            break;
        }
    }
    return E_OK;
}

/**
 * Load audio policy and priority controller config. This SHOULD be called after
 * all the plugins were loaded and initialized.
 */
am_Error_e ControlSenderPlugin::loadConfig()
{
    //get current library path - search: /proc/< getpid() >/maps
    char proc_maps_file_name[256];
    char line[4096];
    char lib_name[256];
    char *tmp;
    pid_t pid = getpid();
    int configGroup;    /* 0-SourceClass,1-SinkClass,2-Gw,3-Route,4-Mixing */

    sprintf(proc_maps_file_name, "/proc/%d/maps", pid);
    FILE *proc_maps = fopen(proc_maps_file_name, "r");

    lib_name[0] = '\0';

    while (!feof(proc_maps))
    {
        char *cnt = fgets(line, 4095, proc_maps);
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
        if ((lib_name != NULL) && (strstr(lib_name, LIBNAMECTL) >= lib_name))
        {
            strcpy(strrchr(lib_name, '/') + 1, CFGNAMECTL);
            logInfo("CONTROLER - Load config file:", lib_name);

            break;
        }
    }

    fclose(proc_maps);

    if (lib_name[0] == 0)
    {
        logError("Error loading config file:", lib_name);
        return E_UNKNOWN;
    }

    FILE *config = fopen(lib_name, "r");

    while (!feof(config))
    {
        fgets(line, 4095, config);

        if (strlen(line) == 0 || line[0] == '#') continue;

        char *tmp = strtok(line, "|\n:>");
        if (tmp == NULL) continue;
/*
 * Set the current config group
 */
        if (strcmp("[SourceClass]", tmp) == 0)
        {
            configGroup = 0;
            continue;
        }
        if (strcmp("[SinkClass]", tmp) == 0)
        {
            configGroup = 1;
            continue;
        }
        if (strcmp("[Gateway]", tmp) == 0)
        {
            configGroup = 2;
            continue;
        }
        if (strcmp("[Route]", tmp) == 0)
        {
            configGroup = 3;
            continue;
        }
        if (strcmp("[Mixing]", tmp) == 0)
        {
            configGroup = 4;
            continue;
        }
        switch (configGroup)    /* 0-SourceClass,1-SinkClass,2-Route,3-Mixing */
        {
            case 0:
/*
 * SourceClass format: NAME|TYPE
 * TYPE=Main|Interrupt
 */
            {
                am_SourceClass_s l_newSourceClass;
                l_newSourceClass.sourceClassID = 0;
                l_newSourceClass.name = std::string(tmp);

                if (this->mNames2SourceClasses[
                        l_newSourceClass.name
                    ].sourceClassID != 0)
                {
                    //source class already added
                    break;
                }

                am_ClassProperty_s newClassProperty;
                newClassProperty.classProperty = CP_GENIVI_SOURCE_TYPE;

                char *tmp = strtok(NULL, "|\n");//type
                if (tmp != NULL && (strcmp(tmp, "Interrupt") == 0))
                {
                    newClassProperty.value = SOURCE_TYPE_INTERRUPT;
                }
                if (tmp != NULL && (strcmp(tmp, "Main") == 0))
                {
                    newClassProperty.value = SOURCE_TYPE_MAIN;
                }

                l_newSourceClass.listClassProperties.push_back(newClassProperty);

                am_Error_e e = this->mControlReceiveInterface->enterSourceClassDB(
                        l_newSourceClass.sourceClassID,
                        l_newSourceClass);
                logInfo("ControlSenderPlugin::loadConfig: Source registered?? '", l_newSourceClass.name ,
                        "', id=", l_newSourceClass.sourceClassID, ", error=", e);
                if (e != E_OK) break;

                this->mNames2SourceClasses[l_newSourceClass.name] =
                        l_newSourceClass;

                break;
            }//end case 0 - source class
            case 1:
/*
 * SinkClass: NAME|TYPE
 * TYPE=Main; currently sink class type is unused:
 *      might be - audio output; recording app; phone sink
 */
            {
                am_SinkClass_s l_newSinkClass;
                l_newSinkClass.sinkClassID = 0;
                l_newSinkClass.name = std::string(tmp);

                if (this->mNames2SinksClasses[
                        l_newSinkClass.name
                    ].sinkClassID != 0)
                {
                    //sink class already added
                    break;
                }

                am_ClassProperty_s newClassProperty;
                newClassProperty.classProperty = CP_GENIVI_SOURCE_TYPE;

                char *tmp = strtok(line, "|\n");//type
                if (tmp != NULL && strcmp(tmp, "Main"))
                {
                    newClassProperty.value = SINK_TYPE_MAIN;
                }

                l_newSinkClass.listClassProperties.push_back(newClassProperty);

                am_Error_e e = mControlReceiveInterface->enterSinkClassDB(
                        l_newSinkClass,
                        l_newSinkClass.sinkClassID);

                if (e != E_OK) break;

                this->mNames2SinksClasses[l_newSinkClass.name] =
                        l_newSinkClass;

                logInfo("New sink class name:", l_newSinkClass.name, "sinkClassId:", l_newSinkClass.sinkClassID);
                break;
            }//end case 1 - sink class
            case 2:
/*
 * Gateway format: NAME:SOURCE>SINK
 * SOURCE and SINK must be the name of an already registered Source/Sink
 */
            {
                am_Gateway_s l_newGateway;
                l_newGateway.gatewayID = 0;
                l_newGateway.name = std::string(tmp);

                if (this->mNames2Gateways[
                        l_newGateway.name
                    ].gatewayID!= 0)
                {
                    //sink class already added
                    break;
                }

                tmp = strtok(NULL, ">");
                l_newGateway.sinkID = this->mNames2Sinks[std::string(tmp)].sinkID;
                l_newGateway.domainSinkID = this->mNames2Sinks[std::string(tmp)].domainID;
                l_newGateway.listSinkFormats = this->mNames2Sinks[std::string(tmp)].listConnectionFormats;

                tmp = strtok(NULL, "\n");
                l_newGateway.sourceID = this->mNames2Sources[tmp].sourceID;
                l_newGateway.domainSourceID = this->mNames2Sources[tmp].domainID;
                l_newGateway.listSourceFormats = this->mNames2Sources[std::string(tmp)].listConnectionFormats;
                l_newGateway.controlDomainID = l_newGateway.domainSourceID;
                l_newGateway.convertionMatrix.push_back(true);

                if (l_newGateway.sinkID == 0 ||
                    l_newGateway.sourceID == 0 ||
                    l_newGateway.domainSourceID == 0 ||
                    l_newGateway.domainSinkID == 0) break;


                am_Error_e e = this->mControlReceiveInterface->enterGatewayDB(
                        l_newGateway,
                        l_newGateway.gatewayID);
                if (e != E_OK) break;

                this->mNames2Gateways[l_newGateway.name] = l_newGateway;
                logInfo("New sink class name:", l_newGateway.name, "sinkId:",
                            l_newGateway.sinkID, "sourceID:", l_newGateway.sourceID);
                break;
            }//end case 2 - gateway
            case 3:
/*
 *  Route format: SourceClass>Sink>Source>...Sink>Source>Sink
 *              : SourceClass>Sink>Source>Gateway>Sink...>Source>Sink
 */
            {
                //first element - source class
                am_SourceClass_s routeSrcClass =
                        this->mNames2SourceClasses[std::string(tmp)];
                //Do nothing if source class is not registered yet
                if (routeSrcClass.sourceClassID == 0) break;

                std::string prevElement;
                bool prevElementWasSrc = true;
                am_sourceID_t prevSrcID = 0;
                am_sourceID_t prevSinkID = 0;
                std::vector<am_RoutingElement_s> route;

                while (tmp != NULL && *tmp != 0)
                {
                    tmp = strtok(NULL, "\n>");
                    if (tmp == NULL || *tmp == 0) break;

                    std::string name(tmp);
                    if (prevElementWasSrc)
                    {
                        //next element should be a sink or a gw
                        am_Sink_s sink = this->mNames2Sinks[name];
                        if (sink.sinkID)
                        {
                            am_RoutingElement_s newRouteElement;
                            newRouteElement.sourceID = prevSrcID;
                            newRouteElement.sinkID = sink.sinkID;
                            newRouteElement.domainID= sink.domainID;
                            newRouteElement.connectionFormat = am::CF_GENIVI_STEREO;
                            //TODO: connection format should not be hardcoded
                            route.push_back(newRouteElement);
                            prevSinkID = newRouteElement.sinkID;
                            prevElementWasSrc = false;
                        }
                        else
                        {
                            am_Gateway_s gw = this->mNames2Gateways[name];
                            if (gw.gatewayID)
                            {
                                am_RoutingElement_s newRouteElement;
                                newRouteElement.sourceID = prevSrcID;
                                newRouteElement.sinkID = gw.sinkID;
                                newRouteElement.domainID= gw.domainSinkID;
                                newRouteElement.connectionFormat = am::CF_GENIVI_STEREO;
                                //TODO: connection format should not be hardcoded
                                route.push_back(newRouteElement);

                                prevSrcID = gw.sourceID;
                                prevElementWasSrc = true;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                    else
                    {//prevElementWasSink
                        am_Source_s source = this->mNames2Sources[name];
                        if (source.sourceID)
                        {
                            prevElementWasSrc = true;
                            prevSrcID = source.sourceID;
                        }
                        else
                        {
                            break;
                        }

                    }
                }

                //the route should end with a Sink
                if (!prevElementWasSrc)
                {
                    std::vector<am_Source_s> allSources;
                    this->mControlReceiveInterface->getListSources(allSources);
                    for (unsigned int i = 0; i < allSources.size(); i++)
                    {
                        if (allSources[i].sourceClassID ==
                                routeSrcClass.sourceClassID)
                        {
                            am_Route_s newRoute;
                            newRoute.sinkID = prevSinkID;
                            newRoute.sourceID = allSources[i].sourceID;
                            newRoute.route = route;

                            am_RoutingElement_s firstRouteElement = newRoute.route[0];
                            firstRouteElement.sourceID = newRoute.sourceID;
                            newRoute.route[0] = firstRouteElement;

                            //check if route already exists
                            bool routeAlreadyExists = false;
                            for (unsigned int m = 0; m < this->mStaticRoutes.size(); m++)
                            {
                                if (this->mStaticRoutes[m].sourceID == newRoute.sourceID &&
                                    this->mStaticRoutes[m].sinkID == newRoute.sinkID)
                                {
                                    routeAlreadyExists = true;
                                }
                            }
                            if (!routeAlreadyExists)
                            {
                                this->mStaticRoutes.push_back(newRoute);
                            }
                        }
                    }
                }
                else
                {
                    logError("Wrong route format:", line);
                }
                break;
            }//end case 3 - route
            case 4:
/*
 * Mixing: SourceClass>ACTIONS
 */
            {
                am_SourceClass_s mixingSrcClass =
                        this->mNames2SourceClasses[std::string(tmp)];
                if (mixingSrcClass.sourceClassID == 0) break;
                std::vector<am_ConnectAction_s> actions;
                while (tmp != NULL && *tmp != 0)
                {
                    am_ConnectAction_s newAction;
                    tmp = strtok(NULL, "\n>:|");//activeSourceClass
                    if (tmp == NULL)
                    {
                        logError("Wrong active source class");
                        break;
                    }

                    am_SourceClass_s activeSrcClass =
                            this->mNames2SourceClasses[std::string(tmp)];
                    if (activeSrcClass.sourceClassID == 0)
                    {
                        logError("Wrong active source class - no ID");
                        break;
                    }

                    tmp = strtok(NULL, "\n>:|");//activeSink
                    if (tmp == NULL)
                    {
                        logError("Wrong active sink");
                        break;
                    }

                    am_Sink_s activeSink =
                            this->mNames2Sinks[std::string(tmp)];
                    if (activeSink.sinkID == 0)
                    {
                        logError("Wrong active sink - no ID");
                        break;
                    }

                    tmp = strtok(NULL, "\n>:|");//actionType
                    if (tmp == NULL)
                    {
                        logError("Wrong action type");
                        break;
                    }

                    am_ActionType_e actionType = AT_DISCONNECT;
                    if (strcmp("Mute", tmp) == 0)
                        actionType = AT_MUTE;
                    else if (strcmp("Unmute", tmp) == 0)
                        actionType = AT_UNMUTE;
                    else if (strcmp("Pause", tmp) == 0)
                        actionType = AT_PAUSE;
                    else if (strcmp("Resume", tmp) == 0)
                        actionType = AT_RESUME;
                    else if (strcmp("DecreaseVolume", tmp) == 0)
                        actionType = AT_VOLUME_DECREASE;
                    else if (strcmp("IncreaseVolume", tmp) == 0)
                        actionType = AT_VOLUME_DECREASE;
                    else if (strcmp("Disconnect", tmp) == 0)
                        actionType = AT_DISCONNECT;
                    else
                    {
                        logError("Wrong action type - enum");
                        break;
                    }

                    tmp = strtok(NULL, "\n>:|");//targetDomain
                    if (tmp == NULL)
                    {
                        logError("Target domain is wrong");
                        break;
                    }

                    am_Domain_s targetDomain =
                            this->mNames2Domains[std::string(tmp)];

                    if (targetDomain.domainID == 0)
                    {
                        logError("Target domain is wrong - no ID");
                        break;
                    }

                    am_ActionTarget_e targetElement;
                    tmp = strtok(NULL, "\n>:|");//targetElement
                    if (strcmp("Source", tmp) == 0)
                        targetElement = ATG_SOURCE;
                    else if (strcmp("Sink", tmp) == 0)
                        targetElement = ATG_SINK;
                    else
                    {
                        logError("Target element is wrong");
                        break;
                    }

                    newAction.activeSourceClassID = activeSrcClass.sourceClassID;
                    newAction.activeSinkID = activeSink.sinkID;
                    newAction.actionType = actionType;
                    newAction.targetDomain = targetDomain.domainID;
                    newAction.targetElement = targetElement;
                    actions.push_back(newAction);
                }
                if (actions.size() > 0)
                {
                    am_MixingRule_s newMixingRule;
                    newMixingRule.mixingSourceClassID = mixingSrcClass.sourceClassID;
                    newMixingRule.actions = actions;
                    //do not add duplicates
                    bool ruleAlreadyExists = false;
                    for (unsigned int n = 0; n < this->mMixingRules.size(); n++)
                    {
                        if (this->mMixingRules[n].mixingSourceClassID == newMixingRule.mixingSourceClassID)
                        {
                        //TODO: is it necessary to compare the actions, too?
                            ruleAlreadyExists = true;
                            break;
                        }
                    }
                    if (!ruleAlreadyExists)
                    {
                        this->mMixingRules.push_back(newMixingRule);
                    }
                }
                break;
            }//end case 4: - mixing
        }
    }


    std::vector<am_Route_s>::iterator iter = mStaticRoutes.begin();
    std::vector<am_Route_s>::iterator iterEnd = mStaticRoutes.end();
    for (; iter != iterEnd; iter++)
    {
        logInfo("{BEGIN Add route: STATRT:SourceID:", iter->sourceID, "-> END:SinkID:", iter->sinkID);

        std::vector<am_RoutingElement_s>::iterator iter1 = iter->route.begin();
        std::vector<am_RoutingElement_s>::iterator iter1End = iter->route.end();
        for (; iter1 < iter1End; iter1++)
        {
            logInfo("Subroute: SourceID:", iter1->sourceID, "-> SinkID:", iter1->sinkID);
        }
        logInfo("END Add route}");
    }

    std::vector<am_MixingRule_s>::iterator iter1 = mMixingRules.begin();
    std::vector<am_MixingRule_s>::iterator iter1End = mMixingRules.end();
    for (; iter1 != iter1End; iter1++)
    {
        logInfo("{BEGIN Add mixing rule: STATRT:mixingSourceClassID:", iter1->mixingSourceClassID);

        std::vector<am_ConnectAction_s>::iterator iter2 = iter1->actions.begin();
        std::vector<am_ConnectAction_s>::iterator iter2End = iter1->actions.end();
        for (; iter2 < iter2End; iter2++)
        {
            logInfo("Action: activeSourceClassID:", iter2->activeSourceClassID,
            ";activeSinkID:", iter2->activeSinkID,
            ";actionType:", iter2->actionType,
            ";targetDomain:", iter2->targetDomain,
            ";targetElement:", iter2->targetElement);
        }
        logInfo("END Add mixing rule}");
    }

    fclose(config);
    return E_OK;
}

am_Error_e ControlSenderPlugin::getSourceInfo(
        const am_sourceID_t sourceID,
        am_Source_s &sourceData)
{
    std::vector<am_Source_s> listSources;
    this->mControlReceiveInterface->getListSources(listSources);
    std::vector<am_Source_s>::iterator iter = listSources.begin();
    std::vector<am_Source_s>::iterator iterEnd = listSources.end();
    for (; iter < iterEnd; iter++)
    {
        if (iter->sourceID == sourceID)
        {
            sourceData = *iter;
            return E_OK;
        }
    }
    return E_NON_EXISTENT;
}

am_Error_e ControlSenderPlugin::hookSystemUpdateSink(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    (void) sinkID;
    (void) sinkClassID;
    (void) listMainSoundProperties;
    (void) listConnectionFormats;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

am_Error_e ControlSenderPlugin::hookSystemUpdateSource(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{
    (void) sourceID;
    (void) sourceClassID;
    (void) listSoundProperties;
    (void) listMainSoundProperties;
    (void) listConnectionFormats;
    return (E_NOT_USED);
}

am_Error_e ControlSenderPlugin::hookSystemUpdateGateway(const am_gatewayID_t gatewayID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    (void) gatewayID;
    (void) listSourceConnectionFormats;
    (void) listSinkConnectionFormats;
    (void) convertionMatrix;
    return (E_NOT_USED);
}

am_Error_e ControlSenderPlugin::hookSystemUpdateConverter(const am_converterID_t converterID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{
    (void) converterID;
    (void) listSourceConnectionFormats;
    (void) listSinkConnectionFormats;
    (void) convertionMatrix;
    return (E_NOT_USED);
}

void ControlSenderPlugin::cbAckSetVolumes(const am_Handle_s handle, const std::vector<am_Volumes_s>& listVolumes, const am_Error_e error)
{
    (void) handle;
    (void) listVolumes;
    (void) error;
}

void ControlSenderPlugin::cbAckSetSinkNotificationConfiguration(const am_Handle_s handle, const am_Error_e error)
{
    (void) handle;
    (void) error;
}


void ControlSenderPlugin::cbAckSetSourceNotificationConfiguration(const am_Handle_s handle, const am_Error_e error)
{
    (void) handle;
    (void) error;
}


void ControlSenderPlugin::hookSinkNotificationDataChanged(const am_sinkID_t sinkID, const am_NotificationPayload_s& payload)
{
    (void) sinkID;
    (void) payload;
}

void ControlSenderPlugin::hookSourceNotificationDataChanged(const am_sourceID_t sourceID, const am_NotificationPayload_s& payload)
{
    (void) sourceID;
    (void) payload;
}

am_Error_e ControlSenderPlugin::hookUserSetMainSinkNotificationConfiguration(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) sinkID;
    (void) notificationConfiguration;
    return (E_NOT_USED);
}

am_Error_e ControlSenderPlugin::hookUserSetMainSourceNotificationConfiguration(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
    (void) sourceID;
    (void) notificationConfiguration;
    return (E_NOT_USED);
}

void ControlSenderPlugin::hookSystemNodeStateChanged(const NsmNodeState_e NodeStateId)
{
    (void) NodeStateId;
    //here you can process informations about the notestate
}

void ControlSenderPlugin::hookSystemNodeApplicationModeChanged(const NsmApplicationMode_e ApplicationModeId)
{
    (void) ApplicationModeId;
}

void ControlSenderPlugin::hookSystemSessionStateChanged(const std::string& sessionName, const NsmSeat_e seatID, const NsmSessionState_e sessionStateID)
{
    (void) sessionName;
    (void) seatID;
    (void) sessionStateID;
}

NsmErrorStatus_e ControlSenderPlugin::hookSystemLifecycleRequest(const uint32_t Request, const uint32_t RequestId)
{
    (void) Request;
    (void) RequestId;
    logInfo("CAmControlSenderBase::hookSystemLifecycleRequest request=",Request," requestID=",RequestId);
    return (NsmErrorStatus_Error);
}

void ControlSenderPlugin::hookSystemSingleTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t time)
{
    (void) connectionID;
    (void) time;
}
