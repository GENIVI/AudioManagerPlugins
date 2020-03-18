/**************************************************************************//**
 * @file CAmRoutePointElement.h
 * 
 * Common base class for sink and source elements
 *
 * This file contains the declaration of route-point element class (member functions and
 * data members) used as base class for source and sink element classes. Volume and 
 * sound-property handling is moved here from former location in class CAmElement.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 */

#ifndef GC_ROUTEPOINTELEMENT_H_
#define GC_ROUTEPOINTELEMENT_H_


#include "CAmElement.h"


namespace am {
namespace gc {


/***************************************************************************//**
 * @class am::gc::CAmRoutePointElement
 * 
 * @copydoc CAmRoutePointElement.h
 */
class CAmRoutePointElement  : public CAmLimitableElement
{
public:
    /** @name General features *//**@{*/
    std::string getElementClassName() const;
    int32_t getPriority(void) const override;
    bool isPersistencySupported() const;
    virtual am_domainID_t getDomainID(void) const = 0;
    virtual am_Error_e setAvailability(const am_Availability_s &availability) = 0;
    virtual am_Error_e getAvailability(am_Availability_s &availability) const = 0;
    /**@}*/

    /** @name Volume managements related methods *//**@{*/
    bool getVolumeSupport() const;
    bool isVolumePersistencySupported() const;
    void setVolume(const am_volume_t volume);
    am_volume_t getVolume() const;
    am_volume_t getMinVolume() const;
    am_volume_t getMaxVolume() const;
    void setOffsetVolume(const am_volume_t limitVolume);
    am_volume_t getOffsetVolume(void) const;
    void setMuteState(const am_MuteState_e muteState);
    am_MuteState_e getMuteState() const override;

    am_volume_t getRoutingSideVolume(am_volume_t internalVolume);
    /**@}*/

    /** @name Sound property related methods *//**@{*/
    bool isMSPPersistenceSupported(const am_CustomMainSoundPropertyType_t &type) const;
    am_Error_e mainSoundPropertyToSoundProperty(const am_MainSoundProperty_s &mainSoundProperty,
        am_SoundProperty_s &soundProperty);
    am_Error_e soundPropertyToMainSoundProperty(const am_SoundProperty_s &soundProperty,
        am_MainSoundProperty_s &mainSoundProperty);
    am_Error_e saturateMainSoundPropertyRange(
        const am_CustomMainSoundPropertyType_t mainSoundPropertyType,
        int16_t &soundPropertyValue);
    am_Error_e saturateSoundPropertyRange(
        const am_CustomSoundPropertyType_t soundPropertyType, int16_t &soundPropertyValue);
    /**@}*/

protected:
    CAmRoutePointElement(gc_Element_e type, const std::string &name, gc_NodePoint_s &config, IAmControlReceive *pControlReceive);

    // Access to common configuration parameters
    // Object hosted in derived class.
    gc_NodePoint_s                       &mConfig;

    // Volume management related members
    am_volume_t                           mVolume;
    am_volume_t                           mMinVolume;
    am_volume_t                           mMaxVolume;
    am_volume_t                           mOffsetVolume;
    am_MuteState_e                        mMuteState;

    // Convert given value from left to right according to given map
    static float _lookupForward(float lhs, const std::map<float, float> &mapValues);
    // Convert given value from right to left according to given map
    static float _lookupReverse(float rhs, const std::map<float, float> &mapValues);

    template <typename TPropertyType, typename Tlisttype>
    am_Error_e _saturateSoundProperty(const TPropertyType soundPropertyType,
        const std::vector<Tlisttype > &listGCSoundProperties,
        int16_t &soundPropertyValue);

    template <typename TPropertyType, typename Tlisttype>
    bool _isSoundPropertyConfigured(const TPropertyType soundPropertyType,
        const std::vector<Tlisttype > &listGCSoundProperties);
};

template <typename TPropertyType, typename Tlisttype>
bool CAmRoutePointElement::_isSoundPropertyConfigured(
    const TPropertyType soundPropertyType,
    const std::vector<Tlisttype > &listGCSoundProperties)
{
    bool                                             soundPropertyConfigured = false;
    typename std::vector<Tlisttype >::const_iterator itListSoundProperties;
    for (itListSoundProperties = listGCSoundProperties.begin();
         itListSoundProperties != listGCSoundProperties.end(); ++itListSoundProperties)
    {
        if ((*itListSoundProperties).type == soundPropertyType)
        {
            soundPropertyConfigured = true;
            break;
        }
    }

    return soundPropertyConfigured;
}

template <typename TPropertyType, typename Tlisttype>
am_Error_e CAmRoutePointElement::_saturateSoundProperty(
    const TPropertyType soundPropertyType,
    const std::vector<Tlisttype > &listGCSoundProperties, int16_t &soundPropertyValue)
{
    am_Error_e                                       result = E_UNKNOWN;
    typename std::vector<Tlisttype >::const_iterator itListSoundProperties;
    // if property list is empty return error
    if (true == listGCSoundProperties.empty())
    {
        LOG_FN_ERROR(__FILENAME__, __func__, " List of sound property is empty");
    }
    else
    {
        for (itListSoundProperties = listGCSoundProperties.begin();
             itListSoundProperties != listGCSoundProperties.end();
             ++itListSoundProperties)
        {
            if ((*itListSoundProperties).type == soundPropertyType)
            {
                soundPropertyValue = std::min(std::max(soundPropertyValue,
                            (*itListSoundProperties).minValue), (*itListSoundProperties).maxValue);
                LOG_FN_EXIT(__FILENAME__, __func__, " Value:", soundPropertyValue);
                result = E_OK;
                break;
            }
        }
    }

    return result;
}


} /* namespace gc */
} /* namespace am */

#endif /* GC_ROUTEPOINTELEMENT_H_ */
