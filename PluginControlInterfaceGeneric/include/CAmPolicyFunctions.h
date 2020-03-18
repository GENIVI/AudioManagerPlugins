/**************************************************************************//**
 * @file CAmPolicyFunctions.h
 *
 * This class encapsulates the evaluation of conditions as defined in policy configuration
 * (see page @ref cond). It is used by the policy engine alone.
 * 
 * Internally it maintains a map of member functions implementing the available
 * @ref functionTable, which are selected by function name string as given in the
 * policy rule.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *//**************************************************************************/

#ifndef GC_POLICYFUNCTIONS_H_
#define GC_POLICYFUNCTIONS_H_

#include "CAmTypes.h"
#include <map>


namespace am {
namespace gc {


class CAmConfigurationReader;
class IAmPolicyReceive;
struct gc_triggerParams_s;
struct gc_ConditionStruct_s;
struct gc_FunctionElement_s;


/**************************************************************************//**
 *  @class CAmPolicyFunctions
 *  @copydoc  CAmPolicyFunctions.h
 */

class CAmPolicyFunctions
{
public:
    CAmPolicyFunctions(IAmPolicyReceive *pPolicyReceive);

    /**
     * @brief Check if all conditions of a specific policy process are met
     * 
     * @param listConditionSets: Collection of conditions defined for the process
     * @param parameters: Unified trigger object containing the request each condition
     *        is checked against
     * 
     * @return true if all conditions are met
     */
    bool evaluateConditionSet(const std::vector<gc_ConditionStruct_s > &listConditionSets,
        const gc_triggerParams_s &parameters);

    void evaluateParameter(const gc_triggerParams_s &trigger, std::string &parameter);

private:
    /**
     * @brief It is API providing the interface to execute the function as defined in conditions as
     *        given in policy set of configuration file
     * @param function: structure defining the function to be executed
     *        trigger: parameters received in hook from framework side during which this function need to be evaluated
     *        listOutputs: list in which output of function execution will be stored
     * @return true on success
     */
    bool _executeFunction(gc_FunctionElement_s function, const gc_triggerParams_s &trigger
        , std::vector<std::string > &listOutputs);

    bool _isConditionTrue(const gc_ConditionStruct_s &Condition, const gc_triggerParams_s &parameters);

    // shorthand for parameters of mapped functions
    #define FUNCTION_PARAMS const gc_FunctionElement_s &function, const gc_triggerParams_s &parameters \
        , std::vector<std::string > &listOutputs 

    /**
     * @brief Prototype for mapped condition functions
     * @param conditionInstance: condition which need to be evaluated
     *        listOutputs: list in which output will be returned
     *        parameters: configuration obtained from trigger
     *        isLHS: to indicate whether RHS or LHS side function of condition need to be evaluated
     * @return E_OK if successful, E_UNKNOWN otherwise
     */
    typedef am_Error_e (CAmPolicyFunctions::*functionPtr)(FUNCTION_PARAMS);

    // implementation of the peek() function
    //   - elementTypeOfType = SOURCEOFCLASS, SINKOFCLASS
    am_Error_e _findPeek(FUNCTION_PARAMS);

    // implementation of the scale() function
    //   - elementType = any (will be ignored
    am_Error_e _findScale(FUNCTION_PARAMS);

    // implementation of the count() function
    //   - elementType = any (will be ignored
    am_Error_e _findCount(FUNCTION_PARAMS);

    // implementation of the name() function
    //   - elementType = SOURCE, SINK, DOMAIN, CONNECTION, CLASS, CLASSOFSOURCE, CLASSOFSINK, 
    //                   DOMAINOFSOURCE, DOMAINOFSINK, SOURCEOFCLASS, SINKOFCLASS
    am_Error_e _findName(FUNCTION_PARAMS);
    am_Error_e _findName(const gc_FunctionElement_s &function,
        std::vector<std::string > &listOutputs, const std::string &name);

    // implementation of the elements() function
    //   - elementType = CLASS, DOMAIN
    am_Error_e _findSubItems(FUNCTION_PARAMS);

    // implementation of the priority() function
    //   - elementType = SOURCE, SINK, CONNECTION, CLASS, CONNECTIONOFCLASS
    am_Error_e _findPriority(FUNCTION_PARAMS);
    template <typename T>
    am_Error_e _findPriority(const gc_FunctionElement_s &function,
        std::string mandatoryParameter,
        std::vector<std::string > &listOutputs);

    // implementation of the connectionState() function
    //   - elementType = CONNECTIONOFSOURCE, CONNECTIONOFSINK, CONNECTIONOFCLASS, returning list
    //   - elementType = CONNECTION, USER, returning single item
    am_Error_e _findConnectionState(FUNCTION_PARAMS);
    am_Error_e _findConnectionState(const gc_FunctionElement_s &function,
        std::vector<std::string > &listOutputs,
        std::string mandatoryParameter, const gc_Element_e elementType);

    // implementation of the volume() function
    //   - elementType = SOURCE, SINK, CONNECTION, CLASS
    am_Error_e _findDeviceVolume(FUNCTION_PARAMS);
    am_Error_e _findDeviceVolume(const gc_Element_e elementType,
        const gc_FunctionElement_s &function,
        std::string elementName, std::vector<std::string > &listOutputs);

    // implementation of the mainVolume() function
    //   - elementType = SOURCE, SINK, USER
    am_Error_e _findMainVolume(FUNCTION_PARAMS);
    am_Error_e _findMainVolume(const gc_Element_e elementType,
        const gc_FunctionElement_s &function,
        std::string elementName, std::vector<std::string > &listOutputs);

    // implementation of the isMainVolumeStep() function
    //   - elementType = USER
    am_Error_e _findIsMainVolumeStep(FUNCTION_PARAMS);

    // implementation of the volumeLimit() function
    //   - elementType = SOURCE, SINK, CONNECTION 
    am_Error_e _findLimit(FUNCTION_PARAMS);

    am_Error_e _findUserErrorValue(FUNCTION_PARAMS);

    // implementation of the soundProperty() function
    //   - elementType = SOURCE, SINK 
    am_Error_e _findDevicePropertyValue(FUNCTION_PARAMS);

    // implementation of the mainSoundPropertyValue() function
    //   - elementType = SOURCE, SINK, USER
    am_Error_e _findMainSoundPropertyValue(FUNCTION_PARAMS);

    // implementation of the systemPropertyValue() function
    //   - elementType = SYSTEM, USER
    am_Error_e _findSystemPropertyValue(FUNCTION_PARAMS);

    // implementation of the systemPropertyType() function
    //   - elementType = USER
    am_Error_e _findSystemPropertyType(FUNCTION_PARAMS);

    // implementation of the mainSoundPropertyType() function
    //   - elementType = USER
    am_Error_e _findMainSoundPropertyType(FUNCTION_PARAMS);

    // implementation of the muteState() function
    //   - elementType = SINK, CONNECTION, CLASS, USER
    am_Error_e _findMuteState(FUNCTION_PARAMS);

    // implementation of the availability() and availabilityReason() functions
    //   - elementType = SOURCE, SINK, USER
    am_Error_e _findAvailability(FUNCTION_PARAMS);
    am_Error_e _findAvailabilityReason(FUNCTION_PARAMS);
    am_Error_e _findAvailability(const gc_FunctionElement_s &function
            , const gc_triggerParams_s &parameters, am_Availability_s &availability);

    // implementation of the connectionFormat() function
    //   - elementType = CONNECTION, CLASS
    am_Error_e _findConnectionFormat(FUNCTION_PARAMS);

    // implementation of the interruptState() function
    //   - elementType = SOURCE, CONNECTION, USER
    am_Error_e _findInterruptState(FUNCTION_PARAMS);

    // implementation of the isRegistered() function
    //   - elementType = SOURCE, SINK, GATEWAY, DOMAIN
    am_Error_e _findIsRegistered(FUNCTION_PARAMS);

    // implementation of the isRegistrationComplete() function
    //   - elementType = DOMAIN
    am_Error_e _findIsDomainRegistrationComplete(FUNCTION_PARAMS);

    // implementation of the state() function
    //   - elementType = SOURCE, DOMAIN 
    am_Error_e _findState(FUNCTION_PARAMS);

    // Internal function used to find the list of connections as per optional
    // parameter set in function of condition in policy section of configuration file
    am_Error_e _getConnectionList(const gc_Element_e elementType, const std::string &elementName,
        const std::string &optionalParameter,
        std::vector<gc_ConnectionInfo_s > &listConnectionInfo,
        const gc_triggerParams_s &parameters) const;

    // implementation of the notificationConfigurationStatus()
    // and notificationConfigurationParam() functions
    //   - elementType = SOURCE, SINK, USER
    am_Error_e _findNTStatus(FUNCTION_PARAMS);
    am_Error_e _findNTParam(FUNCTION_PARAMS);
    am_Error_e _findNTStatusParam(const gc_FunctionElement_s &function,
        std::vector<std::string > &listOutputs,
        std::string mandatoryParameter, gc_Element_e elementType,
        am_CustomNotificationType_t ntType, const bool isStatusReq);

    // implementation of the notificationDataValue() and notificationDataType() functions
    //   - elementType = USER
    am_Error_e _findUserNotificationValue(FUNCTION_PARAMS);
    am_Error_e _findNotificationType(FUNCTION_PARAMS);

    // implementation of the mainNotificationConfigurationType() functions
    //   - elementType = USER
    am_Error_e _findUserNotificationType(FUNCTION_PARAMS);

    // implementation of the mainNotificationConfigurationStatus()
    // and mainNotificationConfigurationParam() functions
    //   - elementType = SOURCE, SINK
    am_Error_e _findMainNTStatus(FUNCTION_PARAMS);
    am_Error_e _findMainNTParam(FUNCTION_PARAMS);
    am_Error_e _findMainNTStatusParam(const gc_FunctionElement_s &function,
        std::vector<std::string > &listOutputs,
        std::string mandatoryParameter, gc_Element_e elementType,
        am_CustomNotificationType_t ntType, const bool isStatusReq);

    // Supporting internal function used to find the value of mandatory parameter of condition
    void _getValueOfParameter(const gc_FunctionElement_s &function,
        std::string &mandatoryParam) const;

    // Supporting internal function used to find whether the condition is for evaluating single domain or for multiple domain evaluation
    bool _isSingleDomainRequest(const gc_FunctionElement_s &function,
        std::string &mandatoryParameter) const;

    // Supporting internal function used to find the name of element from connection list
    am_Error_e _getNameList(const std::string &mandatoryParameter,
        const std::string &optionalParameter,
        std::vector<std::string > &listOutputs,
        const gc_triggerParams_s &parameters, const bool isSinkRequired);

    // Supporting internal template function used to find the domain or class name of element
    template <typename Telement>
    am_Error_e _findDomainOrClassOfElementName(Telement &elementInstance,
        const gc_FunctionElement_s &function, std::vector<std::string > &listOutputs,
        std::string mandatoryParameter, const bool isClassRequired);

    // map to store map of function pointers
    std::map<std::string, functionPtr > mMapFunctionNameToFunctionMaps;

    // map to store function return value type. This are the functions supported in condition of policy
    std::map<std::string, bool >        mMapFunctionReturnValue;

    // pointer to store policy receive class instance
    IAmPolicyReceive *mpPolicyReceive;

};


} /* namespace gc */
} /* namespace am */

#endif /* GC_POLICYFUNCTIONS_H_ */
