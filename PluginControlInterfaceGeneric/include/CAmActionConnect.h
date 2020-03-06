/**************************************************************************//**
 * @file CAmActionConnect.h
 *
 * This file contains the declaration of class connection action connect class
 * (member functions and data members) used to implement the logic of connect.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2020 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 * @copydoc actionConnect
 * 
 *//***********************************************************************//**
 * @page actionConnect  Action Connect
 *
 * <b>Name:</b> ACTION_CONNECT<BR>
 * <b>Name:</b> ACTION_RESUME<BR>
 * <b>Name:</b> ACTION_POP<BR>
 *
 * Implementation of the ACTION_CONNECT, ACTION_RESUME and ACTION_POP @ref am::gc::gc_Action_e "action".
 * Establishes a main connection between a main source and a main sink. Can also be used to
 * re-activate inactive (suspended or pushed) connections.
 * Multiple non-conflicting main connections can be (re-)established with the same command.
 * 
 * A main connection may consist of multiple connection segments (or route elements) depending 
 * on the domain topology. A main connection is said to be in established (or active) when the 
 * connection state is CS_CONNECTED. The connection state is CS_CONNECETD only if all the route elements
 * are connected and the state of all the sources is SS_ON. After the connection is established the 
 * controller would set the change state of all the sources to SS_ON except the sources which has state
 * SS_UNKNOWN. The direction of the source state change could be from main source towards the source 
 * closest to main Sink or vice versa, this depends on the class type of the connection. For the 
 * C_PLAYBACK the direction is from main sink to main source and vice versa. For disconnect/suspend\
 * push action the state of all the sources would be changed and the direction would be reverse that
 * of connect.  
 *   Lets first understand, how class manages the connections queue. Class maintains the list of all 
 * the connections including the ones not in CS_CONNECTED state. only ACTION_DISCONNECT removes the 
 * connection from the class queue as well as the audio manager daemon database.Other actions like 
 * ACTION_SUSPEND and ACTION_PUSH just change the state of the main connection to CS_SUSPENDED and 
 * CS_DISONNECTED respectively but the connection continues to exist in the class Queue. 
 *  This action accepts sourceName, sinkName and className as a parameter to identify the 
 * connection(s) to connect, all the parameter are optional, which means this action would function 
 * even if no parameter are passed. This means the behavior of this action changes heavily depending 
 * on the parameters, it is really important to understand this.
 
 * - - - -
 *
 * ACTION_CONNECT is implemented in class [CAmActionConnect](@ref am::gc::CAmActionConnect)
 * and can be configured with below parameters:
 * 
 * #### Target object selection parameters (positive search): #
 * Selection filters, specified as strings enclosed in double quotes. Macro REQUESTING
 * (without quotes) can also be used, in which case the value will be taken from the trigger object.
 *  + __className__ (string):      The name of the class the connection is assigned to
 *  + __sourceName__ (string):     The name of an involved source.
 *  + __sinkName__ (string):       The name of an involved sink.
 *  + __order__ (@ref am::gc::gc_Order_e "gc_Order_e")
 *                                 The order in which the connection would be chosen. Relevant only in
 *                                 case an existing, inactive connection is to be re-connected.
 *
 * > - if only className is specified then this actions connects the top-most connection depending on the 
 * >     order parameter (default = O_NEWEST).
 * > - if only sourceName is specified then, all the connections possible for that
 * >     source get connected.
 * > - if only sinkName is specified then, all the connection in all the classes belonging to that 
 * >     sink get connected.
 * > - if nothing is specified then the most recent connections of all the classes get re-connected.
 * > - if both sourceName and sinkName are specified, then only one connection (possible between 
 * >     the source and the sink) is established. If such a connection doesn't exists in the class queue, 
 * >     then first such a connection is established and then connected.
 *
 * #### Optional execution parameters: #
 *  + __connectionFormat__ (@ref am::am_CustomConnectionFormat_t "am_CustomConnectionFormat_t")
 *                                 The connection format for the connection. Predefined values are:
 *                                 CF_GENIVI_MONO, CF_GENIVI_STEREO, CF_GENIVI_ANALOG and CF_GENIVI_AUTO.
 *                                 Optionally projects can define more formats.
 *  + __timeOut__ (uint32):        This is the maximum duration, specified in milliseconds, the execution
 *                                 may take before an error action is started.  
 *                                 Defaults to [DEFAULT_ASYNC_ACTION_TIME](@ref am::gc::DEFAULT_ASYNC_ACTION_TIME).
 *                                 It is also possible to specify the infinite timeout i.e. -1.
 *
 * @note This action implicitly launches an @ref actionSetVolume in order to unmute the established connection.
 *       By default, this operation restores its most recent active volume. In case this is not desired,
 *       any of the four optional parameters mainVolume, mainVolumeStep, volume and volumeStep are
 *       forwarded to this operation and can be used to alter its behavior, e.g. specifying  mainVolumeStep="0"
 *       completely neutralizes this action.
 *
 * #### Example Configuration: #
 * Below are some examples of the action configuration
 * @code
 *    <action type="ACTION_CONNECT" sourceName="N:RadioTuner" sinkName="N:RearSpeaker"/>
 * @endcode
 * The above action connects the RadioTuner to RearSpeaker.  
 *
 * @code
 *    <action type="ACTION_CONNECT" sourceName="REQUESTING" sinkName="REQUESTING"/>
 * @endcode
 * The above action connects the source and sink which were part of the trigger. The source and  
 * sink, can only be part of the connect/disconnect trigger from the command side
 *
 * @code
 *    <action type="ACTION_CONNECT" className="REQUESTING" order="O_HIGHPRIORITY"/>
 * @endcode
 * The above action connects the highest priority connection of the requesting class. 
 *
 * @code
 *    <action type="ACTION_CONNECT" sinkName="REQUESTING" mainVolumeStep="0"/>
 * @endcode
 * The above action (re-)connects the most recently established connection for the sink specified 
 * in the trigger. The volume is not altered (e. g. stays muted in most cases). Volume handling 
 * remains up to the  user / system designer.
 *
 *//**************************************************************************/

#ifndef GC_ACTIONCONNECT_H_
#define GC_ACTIONCONNECT_H_

#include "CAmActionContainer.h"
#include "CAmTypes.h"

namespace am {
namespace gc {

#define DEFAULT_CONNECT_ORDER (O_NEWEST)

class CAmClassElement;
class CAmMainConnectionElement;


/**************************************************************************//**
 *  @class CAmActionConnect
 *  @copydoc CAmActionConnect.h
 */
class CAmActionConnect : public CAmActionContainer
{

public:
    CAmActionConnect();
    virtual ~CAmActionConnect();

protected:
    int _execute(void) override;
    int _update(const int result) override;

private:
    am_Error_e _getTargetConnections();
    am_Error_e _selectConnection(std::vector<std::shared_ptr<CAmMainConnectionElement>> &listCandidates
            , std::shared_ptr<CAmMainConnectionElement> &pSelected);
    void _updateConnectionElement(int result, std::shared_ptr<CAmMainConnectionElement> &pConnection);

    IAmActionCommand *_createActionMainConnectionSetVolume(
        std::shared_ptr<CAmMainConnectionElement > pMainConnection);

    // affected main connections
    std::vector<std::shared_ptr<CAmMainConnectionElement > > mListMainConnections;

    // target selection parameters
    std::string                                  mSourceName;
    std::string                                  mSinkName;
    std::string                                  mClassName;
    CAmActionParam<std::string >                 mSourceNameParam;
    CAmActionParam<std::string >                 mSinkNameParam;
    CAmActionParam<std::string >                 mClassNameParam;
    CAmActionParam<std::vector<std::string > >   mExceptClassParam;
    CAmActionParam<gc_Order_e >                  mOrderParam;
    CAmActionParam<am_CustomConnectionFormat_t > mConnectionFormatParam;
    bool mDisposeMainConnection;

    // optional parameters to pass-on to the implicit CAmMainConnectionActionConnect action
    CAmActionParam<am_volume_t >               mVolumeParam;
    CAmActionParam<am_volume_t >               mVolumeStepParam;
    CAmActionParam<am_mainVolume_t >           mMainVolumeParam;
    CAmActionParam<am_mainVolume_t >           mMainVolumeStepParam;
};

}
}
#endif // GC_ACTIONCONNECT_H_
