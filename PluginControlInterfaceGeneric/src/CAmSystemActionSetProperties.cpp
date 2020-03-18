/******************************************************************************
 * @file: CAmSystemActionSetProperties.cpp
 *
 * This file contains the definition of set system properties class
 * (member functions and data members) used to implement the logic of setting
 * the system properties.
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Akshatha Nanjundaswamy <Nanjundaswamy.akshatha@in.bosch.com>
 *  *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/
#include "CAmSystemActionSetProperties.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmSystemActionSetProperties::CAmSystemActionSetProperties(std::shared_ptr<CAmSystemElement> pSystem)
    : CAmActionCommand(std::string("CAmSystemSetProperties"))
    , mpSystem(pSystem)
    , mpControlReceive(pSystem->getControlReceive())
    {
	   this->_registerParam(ACTION_PARAM_LIST_SYSTEM_PROPERTIES, &mListPropertyValueParam);
    }

CAmSystemActionSetProperties::~CAmSystemActionSetProperties(void)
    {
    }

int CAmSystemActionSetProperties::_execute(void)
    {
        if ((nullptr == mpSystem) || (false == mListPropertyValueParam.getParam(mListSystemProperty)))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "Parameters not set");
            return E_NOT_POSSIBLE;
        }
        mListOldSystemProperty = mListSystemProperty;

        for (auto &itListOldSystemProperty : mListOldSystemProperty)
        {
            if (E_OK != mpSystem->getSystemProperty(itListOldSystemProperty.type,
                itListOldSystemProperty.value))
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "COULD NOT GET SYSTEM Properties VALUE");
                return E_NOT_POSSIBLE;
            }
        }
        setUndoRequried(true);
        return mpSystem->setSystemProperties(mListSystemProperty);
    }

} /* namespace gc */
} /* namespace am */


