/**************************************************************************//**
 * @file  CAmActionDisconnect.h
 *
 * This file contains the declaration of user connection action disconnect class
 * (member functions and data members) used to implement the logic of disconnect.
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
 * @copydoc actionDisconnect
 *
 *//***********************************************************************//**
 * @page actionDisconnect Action Disconnect
 *
 * <b>Name:</b> ACTION_DISCONNECT<BR>
 *
 * Implementation of the ACTION_DISCONNECT @ref am::gc::gc_Action_e "action". It disconnects 
 * main connections and removes them from  the Audio manager database. This behavior is implemented
 * in class [CAmActionDisconnect](@ref am::gc::CAmActionDisconnect)
 * 
 * > All parameters of this action are optional. Without any parameter it would disconnect all. 
 *
 * #### Target object selection parameters (positive search): #
 * Positive selection filters, specified as strings enclosed in double quotes. Macro REQUESTING
 * (without quotes) can also be used, in which case the value will be taken from the trigger object.
 *  + __className__ (string):      The name of the class the connection is assigned to
 *  + __sourceName__ (string):     The name of an involved source.
 *  + __sinkName__ (string):       The name of an involved sink.
 *  + __connectionName__ (string): the entire name of the connection, specified as colon-separated
 *                                 concatenation of sourceName and sinkName
 *  + __connectionState__ (@ref am::am_ConnectionState_e "am_ConnectionState_e")
 *                                 All connections matching given state will be disconnected.
 *
 * > If only class name is given, then the active connection of the class is disconnected if it exists. 
 * > If only source or sink name is given, then all the main connections involving passed source or sink
 * > are disconnected and removed.\n
 *
 * #### Exclusion target object parameters (negative search): #
 * Below filter parameters are blank-separated lists of names NOT involved in any of the target connections.
 *  + __exceptClass__ (string)     Class names to be excluded from target selection.
 *  + __exceptSource__ (string)    Source names to be excluded from target selection.
 *  + __exceptSink__ (string)      Sink names to be excluded from target selection.
 *
 * #### Optional execution parameters: #
 *  + __timeOut__ (uint32):        This is the maximum duration, specified in milliseconds, the execution
 *                                 may take before an error action is started.  
 *                                 Defaults to [DEFAULT_ASYNC_ACTION_TIME](@ref am::gc::DEFAULT_ASYNC_ACTION_TIME).
 *                                 It is also possible to specify the infinite timeout i.e. -1.
 *
 * #### Example Configuration: #
 * Below are some examples of the action configuration
 * @code
 *   <action type="ACTION_DISCONNECT" sourceName="N:RadioTuner"/>
 * @endcode
 *  This example would disconnect all the connection involving RadioTuner.  
 *
 * @code
 *   <action type="ACTION_DISCONNECT" className="N:ENTERTAINMENT" sourceName="N:RadioTuner"/>
 * @endcode
 * This example would disconnect all the connection in class ENTERTAINMENT with source name RadioTuner.  
 *
 * @code
 *   <action type="ACTION_DISCONNECT" exceptClasses="N:PHONE"/>
 * @endcode
 * This example would disconnect all the connection except the connections belonging to class  PHONE.
 *
 * @code
 *   <action type="ACTION_DISCONNECT" exceptSinks="N:FrontSpeaker"/>
 * @endcode
 * This example would disconnect all the connection except the connections involving sink FrontSpeaker.
 *
 * @code
 *   <action type="ACTION_DISCONNECT"/>
 * @endcode
 * This example would disconnect all the connections.
 *
 *//**************************************************************************/

#ifndef GC_ACTIONDISCONNECT_H_
#define GC_ACTIONDISCONNECT_H_

#include "CAmActionContainer.h"

namespace am {

namespace gc {

class CAmClassElement;
class CAmMainConnectionElement;


/***************************************************************************//**
 *   @class CAmActionDisconnect
 *   @copydoc CAmActionDisconnect.h
 */
class CAmActionDisconnect : public CAmActionContainer
{
public:
    /**
     * @brief It is the constructor of disconnect action at user level.
     * It sets the strings for classification of parameters as expected by this action.
     * Initialize the member variables with default value.
     */
    CAmActionDisconnect();
    /**
     * @brief It is the destructor of disconnect action at user level.
     * @param none
     * @return none
     */
    virtual ~CAmActionDisconnect();

protected:
    /**
     * @brief This API creates the child action object if class is found whose connection needs to be
     * disconnected. It appends the child to base class for execution.
     * @param none
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _execute(void);

    /**
     * @brief This API dispose the connection if its child action completes successfully.
     * @param None
     * @return E_OK
     */
    int _update(const int result);

private:
    /**
     * @brief This API is used to set the parameters for its child action.
     * @param pMainConnection: pointer to connection which need to be disconnected
     * @return am_Error_e E_OK on success
     */
    am_Error_e _createDisconnectAction(CAmMainConnectionElement *pMainConnection);

    std::vector<std::shared_ptr<CAmMainConnectionElement > > mpListMainConnections;
    // Variables in which policy engine will set the parameters.
    CAmActionParam<std::string >                        mClassNameParam;
    CAmActionParam<std::string >                        mSourceNameParam;
    CAmActionParam<std::string >                        mSinkNameParam;
    CAmActionParam<std::string >                        mConnectionNameParam;
    CAmActionParam<std::vector<am_ConnectionState_e > > mListConnectionStatesParam;
    CAmActionParam<std::vector<std::string > >          mListClassExceptionsParam;
    CAmActionParam<std::vector<std::string > >          mListSinkExceptionsParam;
    CAmActionParam<std::vector<std::string > >          mlistSourceExceptionsParam;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_ACTIONDISCONNECT_H_ */
