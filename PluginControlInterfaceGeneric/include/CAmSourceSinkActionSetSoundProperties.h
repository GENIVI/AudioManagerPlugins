/******************************************************************************
 * @file: CAmSourceSinkActionSetSoundProperties.h
 *
 * This file contains the declaration of router action set sound properties class
 * (member functions and data members) used to implement the logic of setting
 * the sound properties at router level
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com>
 *          Nilkanth Ahirrao and Kapildev Patel  <kpatel@jp.adit-jv.com>
 *  *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#ifndef GC_SOURCESINKACTIONSETSOUNDPROPERTIES_H_
#define GC_SOURCESINKACTIONSETSOUNDPROPERTIES_H_

#include "CAmLogger.h"
#include "CAmActionCommand.h"
#include "CAmHandleStore.h"

namespace am {
namespace gc {

template <typename Telement>
class CAmSourceSinkActionSetSoundProperties : public CAmActionCommand
{
public:
    CAmSourceSinkActionSetSoundProperties(std::shared_ptr<Telement > pElement)
        : CAmActionCommand(std::string("CAmActionSetSoundProperties"))
        , mpElement(pElement)
        , mHandle({H_UNKNOWN, 0})
    {
        this->_registerParam(ACTION_PARAM_LIST_PROPERTY, &mListPropertyValueParam);
    }

    virtual ~CAmSourceSinkActionSetSoundProperties()
    {
    }

protected:
    int _execute(void)
    {
        if ((nullptr == mpElement) || (false == mListPropertyValueParam.getParam(mListMainSoundProperty)))
        {
            LOG_FN_ERROR(__FILENAME__, __func__, "Parameters not set");
            return E_NOT_POSSIBLE;
        }

        mListOldMainSoundProperty = mListMainSoundProperty;

        for (auto &itListOldMainSoundProperty : mListOldMainSoundProperty)
        {
            if (E_OK != mpElement->getMainSoundPropertyValue(itListOldMainSoundProperty.type,
                                                             itListOldMainSoundProperty.value))
            {
                LOG_FN_ERROR(__FILENAME__, __func__, "COULD NOT GET SOUND Properties VALUE");
                return E_NOT_POSSIBLE;
            }
        }

        setUndoRequried(true);
        return _setRoutingSideProperties(mListMainSoundProperty);
    }

    int _undo(void)
    {
       return _setRoutingSideProperties(mListOldMainSoundProperty);
    }

    int _update(const int result)
    {
        if (AS_UNDO_COMPLETE == this->getStatus())
        {
            mpElement->setMainSoundPropertiesValue(mListOldMainSoundProperty);
        }
        else
        {
            if ((E_OK == result) && (this->getStatus() == AS_COMPLETED))
            {
                mpElement->setMainSoundPropertiesValue(mListMainSoundProperty);
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
    int _setRoutingSideProperties(std::vector<am_MainSoundProperty_s> &listMainSoundProperty)
    {
        int result(E_NOT_POSSIBLE);

        std::vector<am_SoundProperty_s > listSoundProperties;

        IAmControlReceive *pControlReceive = mpElement->getControlReceive();

        for (auto &itlistMainSoundProperty : listMainSoundProperty)
        {
            am_SoundProperty_s soundProperty;
            if (E_OK != mpElement->mainSoundPropertyToSoundProperty(itlistMainSoundProperty,
                                                                    soundProperty))
            {
                LOG_FN_ERROR(__FILENAME__, __func__,
                             "not able to convert main sound property to sound property");
                return mpElement->setMainSoundPropertyValue(itlistMainSoundProperty.type,
                                                            itlistMainSoundProperty.value);
            }
            listSoundProperties.push_back(soundProperty);
        }

        if (mpElement->getType() == ET_SOURCE)
        {
            result = pControlReceive->setSourceSoundProperties(mHandle, mpElement->getID(),
                                                               listSoundProperties);
        }
        else
        {
            result = pControlReceive->setSinkSoundProperties(mHandle, mpElement->getID(), listSoundProperties);
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

    std::shared_ptr<Telement >           mpElement;
    am::am_Handle_s                      mHandle;
    CAmActionParam<std::vector<am_MainSoundProperty_s > > mListPropertyValueParam;
    std::vector<am_MainSoundProperty_s > mListMainSoundProperty;
    std::vector<am_MainSoundProperty_s > mListOldMainSoundProperty;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_SOURCESINKACTIONSETSOUNDProperties_H_ */
