/**************************************************************************//**
 * @file  CAmActionSuspend.h
 *
 * This file contains the declaration of user action suspend class
 * (member functions and data members) used to implement the logic of suspending
 * the connected connection at user level
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
 * @copydoc actionSuspend
 * 
 *//***********************************************************************//**
 * @page actionSuspend Action Suspend / Resume
 *
 * <b>Name:</b> ACTION_SUSPEND<BR>
 * <b>Name:</b> ACTION_RESUME<BR>
 *
 * ACTION_SUSPEND changes the main connection state to CS_SUSPENDED state. This means the route
 * is still connected only the source state of participating source(s) in main connection is
 * changed to SS_PAUSE. This behavior is implemented in class [CAmActionSuspend](@ref am::gc::CAmActionSuspend).
 *
 * > The ACTION_RESUME reverts the ACTION_SUSPEND. Internally the 
 * > ACTION_RESUME is realized using the class [CAmActionConnect](@ref am::gc::CAmActionConnect).
 *
 * - - -
 * 
 * In ACTION_SUSPEND, the affected connections can be selected through below parameters:
 * #### Target object selection parameters (positive search): #
 * Selection filters, specified as strings enclosed in double quotes. Macro REQUESTING
 * (without quotes) can also be used, in which case the value will be taken from the trigger object.
 * If multiple connections match the criteria, the first one is selected.
 *  + __className__ (string):      The name of the class the connection is assigned to
 *  + __sourceName__ (string):     The name of an involved source.
 *  + __sinkName__ (string):       The name of an involved sink.
 *  + __order__ (@ref am::gc::gc_Order_e "gc_Order_e")
 *                                 The order in which the connection would be chosen. Default = O_NEWEST.
 *
 * #### Exclusion target object parameters (negative search): #
 * Below filter parameters are blank-separated lists of names NOT involved in any of the target connections.
 *  + __exceptClass__ (string)     Class names to be excluded from target selection.
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
 *   <action type="ACTION_SUSPEND" sourceName="N:RadioTuner"/>
 * @endcode
 * This example would suspend the newest connection involving the sourceName RadioTuner.
 *
 * @code
 *   <action type="ACTION_SUSPEND" className="N:BASE"/>
 * @endcode
 * This action would suspend the newest connection of the BASE Class.
 *
 * @code
 *   <action type="ACTION_RESUME" className="N:BASE" order="O_OLDEST"/>
 * @endcode
 * This example resumes (re-activates) the least recent connection of class BASE. 
 *
 *//**************************************************************************/

#ifndef GC_ACTIONSUSPEND_H_
#define GC_ACTIONSUSPEND_H_

#include "CAmActionContainer.h"

namespace am {
namespace gc {
class CAmClassElement;
class CAmMainConnectionElement;

#define DEFAULT_CONNECT_ORDER (O_NEWEST)

/**************************************************************************//**
 * @class   CAmActionSuspend
 * @copydoc CAmActionSuspend.h
 */
class CAmActionSuspend : public CAmActionContainer
{
public:
    /**
     * @brief It is the constructor of suspend connection action at user level.
     * It sets the strings for classification of parameters as expected by this action.
     * Initialize the member variables with default value.
     */
    CAmActionSuspend();
    /**
     * @brief It is the destructor of suspend action at user level.
     * @return none
     */
    virtual ~CAmActionSuspend(void);

protected:
    /**
     * @brief This API creates the child action object if connection is found which need
     * to be suspended. It appends the child to base class for execution.
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _execute(void);
    int _update(const int result);

private:
    void _getTargetConnections(void);
    void _selectConnection(std::vector<std::shared_ptr<CAmMainConnectionElement>> &listMainConnections
            , std::shared_ptr<CAmMainConnectionElement> &pSelected);

    IAmActionCommand *_createActionSetLimitState(
        std::shared_ptr<CAmMainConnectionElement > pMainConnection);

    // target selection parameters
    CAmActionParam<std::string >               mClassNameParam;
    CAmActionParam<std::string >               mSourceNameParam;
    CAmActionParam<std::string >               mSinkNameParam;
    CAmActionParam<gc_Order_e >                mOrderParam;
    CAmActionParam<std::vector<std::string > > mExceptClassParam;

    // affected main connections
    std::vector<std::shared_ptr<CAmMainConnectionElement > > mListMainConnections;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_ACTIONSUSPEND_H_ */
