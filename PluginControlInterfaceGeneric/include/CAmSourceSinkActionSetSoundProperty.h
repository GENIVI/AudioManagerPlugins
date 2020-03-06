/******************************************************************************
 * @file: CAmSourceSinkActionSetSoundProperty.h
 *
 * This file contains the declaration of router action set sound property class
 * (member functions and data members) used to implement the logic of setting
 * the sound property at router level
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

#ifndef GC_SOURCESINKACTIONSETSOUNDPROPERTY_H_
#define GC_SOURCESINKACTIONSETSOUNDPROPERTY_H_

#include "CAmLogger.h"
#include "CAmActionCommand.h"
#include "CAmHandleStore.h"


namespace am {
namespace gc {

template <typename Telement>
class CAmSourceSinkActionSetSoundProperty : public CAmActionCommand
{
public:
    CAmSourceSinkActionSetSoundProperty(std::shared_ptr<Telement > pElement)
        : CAmActionCommand(std::string("CAmActionSetSoundProperty"))
        , mpElement(pElement)
        , mHandle({H_UNKNOWN, 0})
    {
        this->_registerParam(ACTION_PARAM_PROPERTY_TYPE, &mPropertyTypeParam);
        this->_registerParam(ACTION_PARAM_PROPERTY_VALUE, &mPropertyValueParam);
    }

    virtual ~CAmSourceSinkActionSetSoundProperty()
    {
    }

protected:
    int _execute(void)
    {
        if ((nullptr == mpElement) || (false == mPropertyTypeParam.getParam(mMainSoundProperty.type))
            || (false == mPropertyValueParam.getParam(mMainSoundProperty.value)))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "Parameters not set");
            return E_NOT_POSSIBLE;
        }

        mOldMainSoundProperty = mMainSoundProperty;
        if (E_OK != mpElement->getMainSoundPropertyValue(mOldMainSoundProperty.type,
                mOldMainSoundProperty.value))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "COULD NOT GET SOUND PROPERTY VALUE");
            return E_NOT_POSSIBLE;
        }

        setUndoRequried(true);
        return _setRoutingSideProperty(mMainSoundProperty);
    }

    int _undo(void)
    {
        return _setRoutingSideProperty(mOldMainSoundProperty);
    }

    int _update(const int result)
    {
        if (AS_UNDO_COMPLETE == this->getStatus())
        {
            // Result need not be checked as, even if undo is failed nothing can be done.
            mpElement->setMainSoundPropertyValue(mOldMainSoundProperty.type,
                mOldMainSoundProperty.value);
        }
        else
        {
            if ((E_OK == result) && (this->getStatus() == AS_COMPLETED))
            {
                mpElement->setMainSoundPropertyValue(mMainSoundProperty.type,
                    mMainSoundProperty.value);
            }
        }

        // unregister the observer
        CAmHandleStore::instance().clearHandle(mHandle);
        return E_OK;
    }

    void _timeout(void)
    {
        mpElement->getControlReceive()->abortAction(mHandle);
    }

private:
    int _setRoutingSideProperty(am_MainSoundProperty_s &mainSoundProperty)
    {
        int                result(E_NOT_POSSIBLE);
        am_SoundProperty_s soundProperty;
        IAmControlReceive *pControlReceive = mpElement->getControlReceive();
        if (E_OK != mpElement->mainSoundPropertyToSoundProperty(mainSoundProperty, soundProperty))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "not able to convert main sound property to sound property");
            return mpElement->setMainSoundPropertyValue(mainSoundProperty.type,
                mainSoundProperty.value);
        }

        if (mpElement->getType() == ET_SOURCE)
        {
            result = pControlReceive->setSourceSoundProperty(mHandle, mpElement->getID(), soundProperty);
        }
        else
        {
            result = pControlReceive->setSinkSoundProperty(mHandle, mpElement->getID(), soundProperty);
        }

        if (result == E_OK)
        {
            CAmHandleStore::instance().saveHandle(mHandle, this);
            result = E_WAIT_FOR_CHILD_COMPLETION;
        }
        else
        {
            this->setError(result);
        }

        return result;
    }

    std::shared_ptr<Telement > mpElement;
    am::am_Handle_s            mHandle;
    am_MainSoundProperty_s     mMainSoundProperty;
    am_MainSoundProperty_s     mOldMainSoundProperty;
    CAmActionParam<am_CustomMainSoundPropertyType_t > mPropertyTypeParam;
    CAmActionParam<int16_t >   mPropertyValueParam;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_SOURCESINKACTIONSETSOUNDPROPERTY_H_ */
