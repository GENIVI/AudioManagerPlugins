/**************************************************************************//**
 * @file  CAmSourceElement.h
 *
 * This file contains the declaration of source element class (member functions
 * and data members) used as data container to store the information related to
 * source as maintained by controller.
 *
 * @component  AudioManager Generic Controller
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 -2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 */

#ifndef GC_SOURCEELEMENT_H_
#define GC_SOURCEELEMENT_H_


#include "CAmRoutePointElement.h"


namespace am {
namespace gc {


/***********************************************************************//**
 * @class CAmSourceElement
 * @copydoc CAmSourceElement.h
 */

class CAmSourceElement : public CAmRoutePointElement
{
public:
    /**
     * @brief It is the constructor of source element class. Initialize the member
     * variables with default value.It will be invoked during gateway registration in
     * case source of gateway is not registered previously.
     * @param name: name of the source element
     *        pControlReceive: pointer to IAmControlReceive interface provided by AM daemon
     * @return none
     */
    CAmSourceElement(const gc_Source_s &sourceConfiguration, IAmControlReceive *pControlReceive);
    /**
     * @brief It is the destructor of source element class.
     * @param none
     * @return none
     */
    virtual ~CAmSourceElement();

    /**
     * @brief This API is used to get the main sound property value of source element as in database.
     * @param type: type of property whose value is required
     *        value: variable in which value will be returned
     * @return E_OK on success
     *         E_UNKNOWN on error
     */
    am_Error_e getMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
        int16_t &value) const;

    /**
     * @brief This API is used to set the main sound property value of source element in database.
     * @param type: type of property whose value is required
     *        value: value of the property
     * @return E_OK on success
     *         errors as return by AM
     */
    am_Error_e setMainSoundPropertyValue(const am_CustomMainSoundPropertyType_t type,
        const int16_t value);

    /**
     * @brief This API is used to set the main sound properties value of source element in database.
     * @param listSoundProperties: list of main sound properties
     * @return E_OK on success
     *         errors as return by AM
     */
    am_Error_e setMainSoundPropertiesValue(const std::vector<am_MainSoundProperty_s > &listSoundProperties);

    /**
     * @brief This API is used to set the main notification configuration of sink element in database.
     * @param mainNotificationConfiguraton: Notification configuration
     * @return E_OK on success
     *         errors as return by AM
     */
    am_Error_e setMainNotificationConfiguration(
        const am_NotificationConfiguration_s &mainNotificationConfiguraton);

    /**
     * @brief This API is used to update the notification data
     * @param payload: Notification Payload
     * @return E_OK on success
     *         errors as return by AM
     */
    am_Error_e notificationDataUpdate(const am_NotificationPayload_s &payload);

    /**
     * @brief This API is used to get list of Notification Configuration
     * @param listMainNotificationConfigurations: list of Main Notification Configuration
     * @return E_OK on success
     *         errors as return by AM
     */
    am_Error_e getListMainNotificationConfigurations(
        std::vector<am_NotificationConfiguration_s > &listMainNotificationConfigurations);

    /**
     * @brief This API is used to get Notification Configuration
     * @param notificationConfiguration: Notification configuration
     * @return E_OK on success
     *         errors as return by AM
     */
    am_Error_e getNotificationConfigurations(
        am_CustomNotificationType_t type,
        am_NotificationConfiguration_s &notificationConfiguration);

    /**
     * @brief This API is used to get Main Notification Configuration
     * @param mainNotificationConfiguration: Main Notification configuration
     * @return E_OK on success
     *         errors as return by AM
     */
    am_Error_e getMainNotificationConfigurations(
        am_CustomNotificationType_t type,
        am_NotificationConfiguration_s &mainNotificationConfiguration);

    /**
     * @brief This API is used to get list of Main Notification Configuration
     * @param listNotificationConfigurations: list of Notification Configuration
     * @return E_OK on success
     *         errors as return by AM
     */
    am_Error_e getListNotificationConfigurations(
        std::vector<am_NotificationConfiguration_s > &listNotificationConfigurations);

    /**
     * @brief This API is used to get the sound property value of source element as in database.
     * @param soundPropertyType: type of property whose value is required
     *        soundPropertyValue: variable in which value will be returned
     * @return E_OK on success
     *         E_UNKNOWN on error
     */
    am_Error_e getSoundPropertyValue(const am_CustomSoundPropertyType_t type, int16_t &value) const;

    /**
     * @brief This API is used to update the availability of source element.
     *        When this API is called, the datebase is udpated as well as notification
     * @param availability: variable in which availability will be returned
     */
    am_Error_e setAvailability(const am_Availability_s &availability) override;

    /**
     * @brief This API is used to get the availability of source element as in database.
     * @param availability: variable in which availability will be returned
     */
    am_Error_e getAvailability(am_Availability_s &availability) const override;

    /**
     * @brief This API is used to get the source state value of source element as in database.
     * @return source state of element
     */
    am_SourceState_e getState() const;

    am_domainID_t getDomainID(void) const override;
    am_Error_e getListConnectionFormats(
        std::vector<am_CustomConnectionFormat_t > &listConnectionFormats);
    am_Error_e upadateDB(am_sourceClass_t classId,
        std::vector<am_SoundProperty_s > listSoundProperties,
        std::vector<am_CustomConnectionFormat_t > listConnectionFormats,
        std::vector<am_MainSoundProperty_s > listMainSoundProperties);

    am_InterruptState_e getInterruptState(void) const;
    am_Error_e setInterruptState(const am_InterruptState_e interruptState);

    am_sourceClass_t getClassID(void) const;

    std::shared_ptr<CAmElement > getElement();

protected:
    am_Error_e _register(void);
    am_Error_e _unregister(void);

private:
    gc_Source_s         mSource;
};

class CAmSourceFactory : public CAmFactory<gc_Source_s, CAmSourceElement >
{
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_SOURCEELEMENT_H_ */
