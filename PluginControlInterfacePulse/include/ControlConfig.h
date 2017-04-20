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
 *                             Porting from AM ver 1.x;
 */

#ifndef CONTROLCONFIG_H_
#define CONTROLCONFIG_H_

#include "audiomanagertypes.h"
#include <string>


/* Defines */

#define SOURCE_TYPE_INTERRUPT   (1)
#define SOURCE_TYPE_MAIN        (2)
#define SINK_TYPE_MAIN          (2)

#define ACTION_TYPE_STR(x)  (\
(x == AT_MUTE) ? "AT_MUTE" : \
(x == AT_UNMUTE) ? "AT_UNMUTE" : \
(x == AT_PAUSE) ? "AT_PAUSE" : \
(x == AT_RESUME) ? "AT_RESUME" : \
(x == AT_VOLUME_DECREASE) ? "AT_VOLUME_DECREASE" : \
(x == AT_VOLUME_INCREASE) ? "AT_VOLUME_INCREASE" : \
(x == AT_DISCONNECT) ? "AT_DISCONNECT" : \
(x == AT_CONNECT) ? "AT_CONNECT" : \
#x":Unknown")


/**
 * Action types sorted by priority: lowest - the most important
 */
enum am_ActionType_e
{
    AT_DISCONNECT=0,
    AT_PAUSE=1,
    AT_MUTE=2,
    AT_UNMUTE=3,
    AT_VOLUME_DECREASE=4,
    AT_CONNECT=5,
    AT_RESUME=6,
    AT_VOLUME_INCREASE=7,
    AT_MAX
};


enum am_ActionTarget_e
{
    ATG_SINK=0,
    ATG_SOURCE=1,
    ATG_GW_SINK=2,
    ATG_GW_SOURCE=3,
    ATG_MAX
};


/*
 * e.g. Entertainment   :   Entertainment>HifiAudio Sink:Disconnect:Pulse:Source
 */
struct am_ConnectAction_s {
    bool                  interrupting;          /* The action is triggered by an interrupting source class: e.g. TTS, Phone */
    bool                  pending;               /* The action was not applied yet. */
                                                 /*   e.g. there is an action with higher priority which is already applied to this connection */
    am::am_sourceClass_t  activeSourceClassID;   /* The name of the source class of existing connections - the one to be controlled */
    am::am_sinkID_t       activeSinkID;          /* The name of the sink of existing connections - the one to be controlled */
    am_ActionType_e       actionType;            /* What kind of action the controller decided: mute, pause, volume adjustment */
    am::am_domainID_t     targetDomain;          /* The domain of target element */
    am_ActionTarget_e     targetElement;         /* What element is controlled: sink, source or gateway */
    /**
     * Define a list of inverse actions: if the controller applied an action during connection procedure,
     * the inverse action (of any) shall be applied on disconnect.
     */
    static am_ActionType_e inverseAction(am_ActionType_e type)
    {
        switch(type)
        {
            case AT_MUTE:               return (AT_UNMUTE);
            case AT_UNMUTE:             return (AT_MUTE);

            case AT_PAUSE:              return (AT_RESUME);
            case AT_RESUME:             return (AT_PAUSE);

            case AT_VOLUME_DECREASE:    return (AT_VOLUME_INCREASE);
            case AT_VOLUME_INCREASE:    return (AT_VOLUME_DECREASE);

            case AT_DISCONNECT:         return (AT_CONNECT);//disconnection is not an invertible action; maybe it is
            case AT_CONNECT:            return (AT_MAX);//connection is not an invertible action

            default:                     return (AT_MAX); // default return value
        }
    }


    /**
     * Compare everything except action type.
     */
    bool weekCompare(struct am_ConnectAction_s &o) const
    {
        return ((this->activeSourceClassID == o.activeSourceClassID) &&
                (this->activeSinkID == o.activeSinkID) &&
                (this->targetDomain == o.targetDomain) &&
                (this->targetElement == o.targetElement));
    }

    /**
     * Compare action type
     */
    bool operator < (struct am_ConnectAction_s &o) const
    {
        return this->actionType < o.actionType;
    }

};


struct am_MixingRule_s {
    am::am_sourceClass_t            mixingSourceClassID; /* the name of the source class to be connected/disconnected */
    std::vector<am_ConnectAction_s> actions;                /* actions: e.g decrease volume for source of class Entertainment */
};

#endif


/*
USE-CASE 1
==========

Start Entertainment(connectionID = 1)

Start Phone (connectionID = 2)
    (A1)- actions caused by connectionID[2]: action on connectionID = 1 - mute source

Start Navigation (connectionID = 3)
    (A2) - actions cause by connectionID[3]: action on connectionID = 1 - decrease volume (pending): A1 - higher priority than A2
    (A3) - action caused by connectionID[2]: action on connectionID = 3 - decrease volume (applied)


End Phone
    - inverse action (A1) - unmute connectionID = 1
        - Check if there is any pending action that could be applied, but was blocked by A1
            (A2) - actions cause by connectionID[3]: action on connectionID = 1 - decrease volume
    - inverse action (A3) - increase volume connectionID = 3
        - Check if there is any pending action that could be applied, but was blocked by A3 - none



USE-CASE 2
==========


Start Entertainment(connectionID = 1)

Start Navigation (connectionID = 2)
    (A1) - actions cause by connectionID[2]: action on connectionID = 1 - decrease volume (applied)


Start Phone (connectionID = 3)
    (A2) - action caused by connectionID[3]: action on connectionID = 2 - decrease volume (applied)
    (A3) - actions caused by connectionID[1]: action on connectionID = 1 - mute source; A1 - pending


End Phone
    - inverse action (A2) - increase volume connectionID = 2
        - Check if there is any pending action that could be applied, but was blocked by A3 - none
    - inverse action (A3) - unmute connectionID = 1
        - Check if there is any pending action that could be applied, but was blocked by A3
            (A1) - actions cause by connectionID[2]: action on connectionID = 1 - decrease volume (applied)




USE-CASE 3
==========


Start Entertainment(connectionID = 1)

Start Navigation (connectionID = 2)
    (A1) - actions cause by connectionID[2]: action on connectionID = 1 - decrease volume (applied)


Start Phone (connectionID = 3)
    (A2) - action caused by connectionID[3]: action on connectionID = 2 - decrease volume (applied)
    (A3) - actions caused by connectionID[3]: action on connectionID = 1 - mute source; A1 - pending

End Navigation
    - inverse action (A1) - A1 is pending ... no action to apply

End Phone
    - inverse action (A2) - increase volume connectionID = 2
        - connection does not exits
    - inverse action (A3) - unmute connectionID = 1
*/



/*

MainAction -> list of subconnection, each subconnection, a list of actions
             each action has a triggering main connection (other)

//what actions this main connection was causing
MainAction -> a map of subconnections and corresponding action

*/
