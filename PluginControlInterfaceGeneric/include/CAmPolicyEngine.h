/**************************************************************************//**
 * @file CAmPolicyEngine.h
 *
 * This file contains the declaration of policy engine class (member functions
 * and data members) used to provide the interface to get the actions related
 * to the trigger/hook from framework side
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *//**************************************************************************/

#ifndef GC_POLICYENGINE_H_
#define GC_POLICYENGINE_H_

#include <string>
#include <vector>
#include <map>
#include "CAmConfigurationReader.h"
#include "CAmTypes.h"
#include "CAmRoutePointElement.h"

namespace am {
namespace gc {

#define SEARCH_STATIC_SOURCE     0x01
#define SEARCH_STATIC_SINK       0x02
#define SEARCH_STATIC_GATEWAY    0x04
#define SIZE_OF_REQUESTING_MACRO 10
struct gc_triggerParams_s
{
    gc_triggerParams_s()
    {
        triggerType = TRIGGER_UNKNOWN;
        sinkName.clear();
        sourceName.clear();
        domainName.clear();
        gatewayName.clear();
        className.clear();
        connectionName.clear();
        listMainSoundProperty.clear();
        listSystemProperties.clear();
        mainVolume                         = 0;
        isVolumeStep                       = false;
        status                             = E_OK;
        connectionState                    = CS_UNKNOWN;
        muteState                          = MS_UNKNOWN;
        interruptState                     = IS_UNKNOWN;
        mainSoundProperty.type             = MSP_UNKNOWN;
        mainSoundProperty.value            = 0;
        systemProperty.type                = SYP_UNKNOWN;
        systemProperty.value               = 0;
        availability.availability          = A_UNKNOWN;
        availability.availabilityReason    = AR_UNKNOWN;
        notificatonPayload.type            = NT_UNKNOWN;
        notificatonPayload.value           = 0;
        notificatonConfiguration.type      = NT_UNKNOWN;
        notificatonConfiguration.status    = NS_UNKNOWN;
        notificatonConfiguration.parameter = 0;
    }

    gc_Trigger_e triggerType;
    std::string sinkName;
    std::string sourceName;
    std::string domainName;
    std::string gatewayName;
    std::string className;
    std::string connectionName;
    am_ConnectionState_e connectionState;
    am_Error_e status;
    am_mainVolume_t mainVolume;
    bool  isVolumeStep;
    am_MainSoundProperty_s mainSoundProperty;
    std::vector<am_MainSoundProperty_s> listMainSoundProperty;
    am_SystemProperty_s systemProperty;
    am_Availability_s availability;
    am_MuteState_e muteState;
    am_InterruptState_e interruptState;
    am_NotificationPayload_s notificatonPayload;
    am_NotificationConfiguration_s notificatonConfiguration;
    std::vector<am_SystemProperty_s> listSystemProperties;
};

/// stream non-default members for logging
inline std::ostream &operator<<(std::ostream &out, const gc_triggerParams_s &trigger)
{
    out << trigger.triggerType << ":{";
    int cnt = 0;
    if (trigger.className.size())
    {
        out << (cnt++ ? ", " : "") << "className:" << trigger.className;
    }

    if (trigger.sourceName.size())
    {
        out << (cnt++ ? ", " : "") << "sourceName:" << trigger.sourceName;
    }

    if (trigger.sinkName.size())
    {
        out << (cnt++ ? ", " : "") << "sinkName:" << trigger.sinkName;
    }

    if (trigger.domainName.size())
    {
        out << (cnt++ ? ", " : "") << "domainName:" << trigger.domainName;
    }

    if (trigger.gatewayName.size())
    {
        out << (cnt++ ? ", " : "") << "gatewayName:" << trigger.gatewayName;
    }

    if (trigger.connectionName.size())
    {
        out << (cnt++ ? ", " : "") << "connectionName:" << trigger.connectionName;
    }

    if (trigger.connectionState != CS_UNKNOWN)
    {
        out << (cnt++ ? ", " : "") << "connectionState:" << trigger.connectionState;
    }

    if (trigger.status != E_OK)
    {
        out << (cnt++ ? ", " : "") << "status:" << trigger.status;
    }

    if (trigger.mainVolume)
    {
        out << (cnt++ ? ", " : "") << "mainVolume"
            << (trigger.isVolumeStep ? " (step):" : ":") << trigger.mainVolume;
    }

    if (trigger.mainSoundProperty.type != MSP_UNKNOWN)
    {
        out << (cnt++ ? ", " : "") << "mainSoundProperty:{type:" << trigger.mainSoundProperty.type
            << ", value:" << trigger.mainSoundProperty.value << "}";
    }

    if (trigger.listMainSoundProperty.size() != 0)
    {
        out << (cnt++ ? ", " : "") << "listMainSoundProperty: [ ";
        for (const auto& iter: trigger.listMainSoundProperty)
        {
            int loopCnt = 0;
            out << (loopCnt++ ? ", " : "") << " mainSoundProperty:{type:" << iter.type
                << ", value:" << iter.value << "}";
        }
        out << "]";
    }

    if (trigger.systemProperty.type != SYP_UNKNOWN)
    {
        out << (cnt++ ? ", " : "") << "systemProperty:{type:" << trigger.systemProperty.type
            << ", value:" << trigger.systemProperty.value << "}";
    }

    if (trigger.listSystemProperties.size() != 0)
    {
        out << (cnt++ ? ", " : "") << "listSystemProperties:[ ";
        for (const auto& iter: trigger.listSystemProperties)
        {
            int loopCnt = 0;
            out << (loopCnt++ ? ", " : "") << " SystemProperty:{type:" << iter.type
                << ", value:" << iter.value << "}";
        }
        out << "]";
    }

    if (trigger.muteState != MS_UNKNOWN)
    {
        out << (cnt++ ? ", " : "") << "muteState:" << trigger.muteState;
    }

    if (trigger.interruptState != IS_UNKNOWN)
    {
        out << (cnt++ ? ", " : "") << "interruptState:" << trigger.interruptState;
    }

    if (trigger.notificatonPayload.type != NT_UNKNOWN)
    {
        out << (cnt++ ? ", " : "") << "notificatonPayload:{type:" << trigger.notificatonPayload.type
            << ", value:" << trigger.notificatonPayload.value << "}";
    }

    if ((trigger.notificatonConfiguration.type != NT_UNKNOWN) || (trigger.notificatonConfiguration.status != NS_UNKNOWN))
    {
        out << (cnt++ ? ", " : "") << "notificatonConfiguration:{type:" << trigger.notificatonConfiguration.type
            << ", status:" << trigger.notificatonConfiguration.status
            << ", parameter:" << trigger.notificatonConfiguration.parameter << "}";
    }

    return out << "}" << std::endl;
}

class IAmPolicyReceive;
class CAmPolicyFunctions;

/**************************************************************************//**
 *  @class CAmPolicyEngine
 *  @copydoc  CAmPolicyEngine.h
 */

class CAmPolicyEngine
{
public:
    /**
     * @brief It is the constructor of policy Engine class. Initialize the member
     * variables with default value.
     * @param none
     * @return none
     */
    CAmPolicyEngine();
    /**
     * @brief It is the destructor of policy Engine class.
     * @param none
     * @return none
     */
    ~CAmPolicyEngine()
    {
    }

    /**
     * @brief It is API to start the policy engine. It gets the list of system properties from
     * configuration and give request to framework to register the properties with AM.
     * It also give request to register the classes to AM
     * @param pPolicyReceive: pointer to policy receive class instance
     * @return E_NOT_POSSIBLE on error
     *         E_OK on success
     */
    am_Error_e startPolicyEngine(IAmPolicyReceive *pPolicyReceive);

    /**
     * @brief It is API to stop the policy engine.
     * @param  none
     * @return none
     */
    void stopPolicyEngine();

    static void evaluateMacro(const gc_triggerParams_s &trigger, std::string &parameter);

    am_Error_e processTrigger(gc_triggerParams_s &triggerParams);

private:
    bool _getActionsfromPolicy(const gc_Process_s &process, std::vector<gc_Action_s > &listActions,
        const gc_triggerParams_s &parameters);

    void _convertActionParamsToValues(gc_Action_s &action, const gc_triggerParams_s &parameters);
    void _evaluateParameterFunction(const gc_triggerParams_s &triggerParams, std::string &paramValue);

    /**
     * @brief It is API providing the interface to get the actions related to trigger from configuration
     * @param trigger: trigger type for which actions are needed
     *        listActions: variable in which list of actions will be returned
     *        parameters: parameters received in hook
     * @return E_NOT_POSSIBLE on error
     *         E_OK on success
     */
    am_Error_e _getActions(std::vector<gc_Action_s > &listActions, const gc_triggerParams_s &parameters);
    am_Error_e _updateActionParameters(gc_Action_s &action, const gc_triggerParams_s &triggerParams);

    void _getImplicitActions(std::vector<gc_Action_s > &listActions, const gc_triggerParams_s &parameters);
    void _createDisconnectActions(std::set<std::string > &listActionSet, std::vector<gc_Action_s > &listActions);

    void _removeDoubleQuotes(std::string &inputString, const std::string &replaceString);
    void _getMainConnectionName(gc_Element_e elementType, std::string inElementName, std::set<std::string > &listMainConnection);

    void _getListStaticSinks(const std::string &domainName, std::vector<gc_Sink_s > &listStaticSinks);
    void _getListStaticSources(const std::string &domainName, std::vector<gc_Source_s > &listStaticSources);
    void _getListStaticGateways(std::vector<std::string > &listGateways, std::string &listSources, std::string &listSinks);

    void _gatewayDeregister(std::string inElementName, std::vector<gc_Action_s > &listActions);
    void _domainDeregister(std::string inElementName, std::vector<gc_Action_s > &listActions);

    void _scheduleSoundPropertyTriggers(std::shared_ptr<CAmRoutePointElement > pElement);
    void _restoreMainSoundProperties(std::string domainName);
    void _updateSystemProperty(am_Error_e status, std::vector<gc_Action_s > &listActions);

    // pointer to store policy receive class instance
    IAmPolicyReceive                   *mpPolicyReceive;

    CAmPolicyFunctions                 *mpPolicyFunctions;
};


} /* namespace gc */
} /* namespace am */
#endif /* GC_POLICYENGINE_H_ */
