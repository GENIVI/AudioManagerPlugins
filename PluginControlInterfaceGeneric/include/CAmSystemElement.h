/**************************************************************************//**
 * @file  CAmSystemElement.h
 *
 * This file contains the declaration of system element class (member functions
 * and data members) used as data container to store the information related to
 * system as maintained by controller.
 *
 * @component  AudioManager Generic Controller
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *//**************************************************************************/

#ifndef GC_SYSTEMELEMENT_H_
#define GC_SYSTEMELEMENT_H_

#include "IAmControlCommon.h"
#include "CAmElement.h"
#include "CAmTypes.h"


namespace am {
namespace gc {

#define SYSTEM_ID 1


/**************************************************************************//**
 * @class   am::gc::CAmSystemElement
 * @copydoc CAmSystemElement.h
 */

class CAmSystemElement : public CAmElement
{
public:
    CAmSystemElement(const gc_System_s &systemConfiguration, IAmControlReceive *pControlReceive);
    virtual ~CAmSystemElement();
    am_Error_e getSystemProperty(const am_CustomSystemPropertyType_t type, int16_t &value) const;
    am_Error_e setSystemProperty(const am_CustomSystemPropertyType_t type, const int16_t value);
    am_Error_e setSystemProperties(const std::vector<am_SystemProperty_s> &listSystemProperties);
    int16_t getDebugLevel(void) const;
    bool isNonTopologyRouteAllowed(void) const;
    bool isUnknownElementRegistrationSupported(void) const;
    bool isSystemPropertyReadOnly(void) const;
    std::string getLastSystemPropertiesString();

protected:
    am_Error_e _register(void);
    am_Error_e _unregister(void);

private:
    am_Error_e _findSystemProperty(const std::vector<am_SystemProperty_s> &listSystemProperties,
        const uint16_t type, int16_t &value) const;
    bool _updateListLastSystemProperty(const am_CustomSystemPropertyType_t &type,
        const int16_t value);
    bool _isPersistenceSupported(const am_CustomSystemPropertyType_t &type);

    gc_System_s mSystem;
    std::vector<am_SystemProperty_s > mListSystemProperties;
    std::vector<am_SystemProperty_s > mListLastSystemProperties;
};

class CAmSystemFactory : public CAmFactory<gc_System_s, CAmSystemElement >
{
};

} /* namespace gc */
} /* namespace am */

#endif /* GC_SYSTEMELEMENT_H_ */
