/******************************************************************************
 * @file: CAmSystemActionDebug.cpp
 *
 * This file contains the definition of system action register class (member
 * functions and data members) used to implement the logic of registering the
 * static source/sink/gateway as defined in configuration.
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Toshiaki Isogai <tisogai@jp.adit-jv.com>
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>
 *          Prashant Jain   <pjain@jp.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#include "CAmSystemActionDebug.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmSystemActionDebug::CAmSystemActionDebug(CAmControlReceive* pControlReceive) :
                                CAmActionCommand(std::string("CAmSystemActionDebug")),
                                mpControlReceive(pControlReceive)

{
    LOG_FN_DEBUG("  object created");
    _registerParam(ACTION_PARAM_DEBUG_TYPE, &mTypeParam);
    _registerParam(ACTION_PARAM_DEBUG_VALUE, &mValueParam);
}
CAmSystemActionDebug::~CAmSystemActionDebug(void)
{
}

int CAmSystemActionDebug::_execute(void)
{
    uint16_t debugType;
    int16_t debugValue;
    if ((false == mTypeParam.getParam(debugType)) || (false == mValueParam.getParam(debugValue)))
    {
        return E_NOT_POSSIBLE;
    }
    else
    {
        if (SYP_DEBUGLEVEL == debugType)
        {
            LOG_FN_CHANGE_LEVEL(debugValue);
        }
        am_SystemProperty_s property;
        property.type = debugType;
        property.value = debugValue;
        mpControlReceive->changeSystemPropertyDB(property);
    }
    return E_OK;
}

} /* namespace gc */
} /* namespace am */
