/**************************************************************************//**
 *  @page actionRegister Action Register
 * 
 * __Name:__ ACTION_REGISTER<BR>
 * 
 * This action can be used to conditionally register sources, sinks and / or
 * gateways which are not registered otherwise (e.g. from their domain).
 * 
 * #### Target object selection parameters: #
 *  - __sourceName__ (string):   List of sources
 *  - __sinkName__ (string):     List of sinks
 *  - __gatewayName__ (string):  List of gateways
 *  
 * > One or more of the above parameters may be specified, each given as string
 * > containing one or more names, separated by a blank character ' '.
 * > Given names must match elements specified in configuration file.
 * 
 * #### Optional execution parameter: #
 *  - __timeOut__ (uint32):      This is the maximum duration, specified in milliseconds, the execution
 *                               may take before an error action is started.  
 *                               Defaults to [DEFAULT_ASYNC_ACTION_TIME](@ref am::gc::DEFAULT_ASYNC_ACTION_TIME).
 * 
 * #### Example Configurations: #
 * Register 2 sinks
 * @code
 *      <action type="ACTION_REGISTER" sinkName="RadioSink Amplifier"/>
 * @endcode
 * 
 *//***********************************************************************//**
 * @file CAmSystemActionRegister.h
 *
 * This file contains the declaration of system action register class (member
 * functions and data members) used to implement the logic of registering the
 * static source/sink/gateway as defined in configuration.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 * For configuration details of this action see page @ref actionRegister.
 *
 *//**************************************************************************/

#ifndef GC_SYSTEMACTIONREGISTER_H_
#define GC_SYSTEMACTIONREGISTER_H_

#include "CAmActionCommand.h"
#include "CAmSystemElement.h"

namespace am {
namespace gc {


/**************************************************************************//**
 * @class am::gc::CAmSystemActionRegister
 * @copydoc CAmSystemActionRegister.h
 */
class CAmSystemActionRegister : public CAmActionCommand
{
public:
    /**
     * @brief It is the constructor of register action at system level.
     * It sets the strings for classification of parameters as expected by this action.
     * Initialize the member variables with default value.
     * @param pRoutingManager: pointer to CAmRoutingManager Class object
     * @return none
     */
    CAmSystemActionRegister(std::shared_ptr<CAmSystemElement > pSystem);
    /**
     * @brief It is the destructor of register action at system level.
     * @param none
     * @return none
     */
    virtual ~CAmSystemActionRegister(void);

protected:
    /**
     * @brief This API based on parameter set invokes the routing manager API to register the element
     * @param none
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _execute(void);

private:
    /**
     * @brief This API is used to fill the gateway information in structure which will be used
     * during registration of gateway.
     * @param none
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _populateGatewayStruct(gc_Gateway_s &gateway);

    /**
     * @brief This API is used to get the domain id of element
     * @param none
     * @return E_OK on success
     *         E_UNKNOWN on error
     */
    am_Error_e _getDomainID(const std::string &domainName, am_domainID_t &domainID);

    // Set in constructor. It is used to invoke the function to register the element.
    IAmControlReceive                          *mpControlReceive;
    // Variables in which parent action will set the parameters.
    CAmActionParam<std::vector<gc_Source_s > >  mListSources;
    CAmActionParam<std::vector<gc_Sink_s > >    mListSinks;
    CAmActionParam<std::vector<gc_Gateway_s > > mListGateways;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_SYSTEMACTIONREGISTER_H_ */
