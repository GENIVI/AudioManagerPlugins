/**************************************************************************//**
 * @file CAmActionInterrupt.h
 *
 * This file contains the declaration of user action interrupt class (member functions
 * and data members) used to implement the logic of pushing (disconnect) the connection
 * at user level
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
 * @copydoc actionInterrupt
 * 
 *//***********************************************************************//**
 * @page actionInterrupt Action Push / Pop
 *
 * <b>Name:</b> ACTION_PUSH<BR>
 * <b>Name:</b> ACTION_POP<BR>
 *
 * ACTION_PUSH disconnects the connection, but unlike ACTION_DISCONNECT it 
 * does not remove the connection from the audio manager database. The connection continues to be 
 * visible from the command side in the CS_DISCONNECTED state. Analogous to stack push and pop operation,
 * the connection push and pop is performed, hence we named this action as PUSH and POP. The use cases 
 * where some connection needs to be disconnected but memorized, and at later some point of time it should 
 * be connected again, these actions should be used.
 *
 * > The ACTION_POP reverts the ACTION_PUSH.
 * > Internally ACTION_POP is realized using the class [CAmActionConnect](@ref am::gc::CAmActionConnect).
 * 
 * - - - -
 *
 * ACTION_PUSH is implemented in class [CAmActionInterrupt](@ref am::gc::CAmActionInterrupt)
 * and can be configured with below parameters:
 * 
 * #### Target object selection parameters (positive search): #
 * Selection filters, specified as strings enclosed in double quotes. Macro REQUESTING
 * (without quotes) can also be used, in which case the value will be taken from the trigger object.
 *  + __className__ (string):      The name of the class the connection is assigned to
 *  + __sourceName__ (string):     The name of an involved source.
 *  + __sinkName__ (string):       The name of an involved sink.
 *
 * > - If class name is provided as a parameter then the active main connection of that
 * >     class would be pushed if exists. 
 * > - If source and sink name pair is provided then the main
 * >     connection involving source sink pair is pushed in the queue. 
 * > - If the connection exists then the main connection is disconnected
 * > - If the connection  does not exists, the main connection is
 * >     added to the audiomanager database in the CS_DISCONNECTED state.
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
 *   <action type="ACTION_PUSH" sourceName="N:RadioTuner"/>
 * @endcode
 * This example would push the connections involving the sourceName RadioTuner.  
 *
 * @code
 *   <action type="ACTION_PUSH" className="N:BASE"/>
 * @endcode
 * This action would push the active connection of the BASE Class.
 *
 * @code
 *   <action type="ACTION_POP" className="N:BASE" order="O_NEWEST"/>
 * @endcode
 * This example would connect the latest connection of the BASE class present in the Queue. 
 *
 *//**************************************************************************/

#ifndef GC_ACTIONINTERRUPT_H_
#define GC_ACTIONINTERRUPT_H_

#include "CAmActionContainer.h"

namespace am {
namespace gc {

class CAmClassElement;
class CAmMainConnectionElement;


/**************************************************************************//**
 * @class   CAmActionInterrupt
 * @copydoc CAmActionInterrupt.h
 */
class CAmActionInterrupt : public CAmActionContainer
{
public:
    /**
     * @brief It is the constructor of push connection action at user level.
     * It sets the strings for classification of parameters as expected by this action.
     * Initialize the member variables with default value.
     */
    CAmActionInterrupt();
    /**
     * @brief It is the destructor of push action at user level.
     */
    virtual ~CAmActionInterrupt();

protected:
    /**
     * @brief This API creates the child action object if connection is found which need
     * to be pushed (disconnected). It appends the child to base class for execution.
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _execute(void);

private:
    IAmActionCommand *_createActionSetLimitState(
        std::shared_ptr<CAmMainConnectionElement > pMainConnection);

    // Variables in which policy engine will set the parameters.
    CAmActionParam<std::string >               mClassNameParam;
    CAmActionParam<std::string >               mSourceNameParam;
    CAmActionParam<std::string >               mSinkNameParam;
    CAmActionParam<std::vector<std::string > > mListClassExceptionsParam;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_ACTIONINTERRUPT_H_ */
