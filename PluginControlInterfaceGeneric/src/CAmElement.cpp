/******************************************************************************
 * @file: CAmElement.cpp
 *
 * This file contains the definition of element class (member functions and
 * data members) used as base class for source, sink & gateway element classes
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

#include <algorithm>
#include <map>
#include "CAmElement.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmElement::CAmElement(gc_Element_e type, const std::string &name, IAmControlReceive *pControReceive)
    : mpControlReceive(pControReceive)
    , mName(name)
    , mID(0)
    , mType(type)
{
    LOG_FN_DEBUG(__FILENAME__, mType, mName, "established");
}

CAmElement::~CAmElement()
{
    LOG_FN_DEBUG(__FILENAME__, mType, mName, "removed");
}

IAmControlReceive *CAmElement::getControlReceive(void)
{
    return mpControlReceive;
}

std::string CAmElement::getName(void) const
{
    return mName;
}

gc_Element_e CAmElement::getType(void) const
{
    return mType;
}

void CAmElement::setID(const uint16_t ID)
{
    mID = ID;
}

uint16_t CAmElement::getID(void) const
{
    return mID;
}

int32_t CAmElement::getPriority() const
{
    // default implementation for elements which do not support dedicated priority

    return 0;
}

am_Error_e CAmElement::_register(std::shared_ptr<CAmElement > pObserver)
{
    am_Error_e                                           result = E_UNKNOWN;
    std::vector<std::shared_ptr<CAmElement > >::iterator itListObservers;

    if (pObserver != nullptr)
    {
        itListObservers = std::find(mListObservers.begin(), mListObservers.end(), pObserver);
        if (itListObservers == mListObservers.end())
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, mType, mName, "observer attached:"
                    , pObserver->getType(), pObserver->getName());

            /*element not found so push it to the list*/
            mListObservers.push_back(pObserver);
            result = E_OK;
        }
        else
        {
            LOG_FN_WARN(__FILENAME__, __func__, mType, mName, "observer is already registered:"
                    , (*itListObservers)->getType(), (*itListObservers)->getName());
            result = E_ALREADY_EXISTS;
        }
    }
    else
    {
        // Null pointer
        LOG_FN_ERROR(__FILENAME__, __func__, getName(), "observer is nullptr");
        result = E_NOT_POSSIBLE;
    }

    return result;
}

am_Error_e CAmElement::_deregister(std::shared_ptr<CAmElement > pObserver)
{
    am_Error_e                                           result = E_UNKNOWN;
    std::vector<std::shared_ptr<CAmElement > >::iterator itListObservers;

    /* handle the NULL pointer check */
    if (pObserver != nullptr)
    {
        itListObservers = std::find(mListObservers.begin(), mListObservers.end(), pObserver);
        if (itListObservers != mListObservers.end())
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, mType, mName, "observer detached:"
                    , (*itListObservers)->getType(), (*itListObservers)->getName());

            mListObservers.erase(itListObservers);
            result = E_OK;
        }
        else
        {
            result = E_OK;
        }
    }
    else
    {
        result = E_OK;
    }

    return result;
}

am_Error_e CAmElement::attach(std::shared_ptr<CAmElement > pSubject)
{
    am_Error_e result = E_UNKNOWN;

    if (pSubject != nullptr)
    {
        result = pSubject->_register(this->getElement());
        if (result == E_OK)
        {
            mListSubjects.push_back(pSubject);
        }
        else if (result == E_ALREADY_EXISTS)
        {
            /*if observer is already registered then check if subject also registered*/
            std::vector<std::shared_ptr<CAmElement > >::iterator itListSubjects;
            itListSubjects = std::find(mListSubjects.begin(), mListSubjects.end(), pSubject);
            if (itListSubjects != mListSubjects.end())
            {
                LOG_FN_INFO(__FILENAME__, __func__, "subject is already registered");
                result = E_ALREADY_EXISTS;
            }
            else
            {
                /*element not found so push it to the list*/
                mListSubjects.push_back(pSubject);
                result = E_OK;
            }
        }
        else
        {
            // error E_NOT_POSSIBLE
        }
    }
    else
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "subject is nullptr");
        result = E_NOT_POSSIBLE;
    }

    return result;
}

am_Error_e CAmElement::detach(std::shared_ptr<CAmElement > pSubject)
{
    am_Error_e                                           result = E_UNKNOWN;
    std::vector<std::shared_ptr<CAmElement > >::iterator itListSubjects;

    if (pSubject != nullptr)
    {
        itListSubjects = std::find(mListSubjects.begin(), mListSubjects.end(), pSubject);
        if (itListSubjects != mListSubjects.end())
        {
            mListSubjects.erase(itListSubjects);                /* remove subject from the observer list */
            result = pSubject->_deregister(this->getElement()); /* remove observer from the subject list */
        }
        else
        {
            // element not found
            result = E_OK;
        }
    }
    else
    {
        // Null pointer
        result = E_NOT_POSSIBLE;
    }

    return result;
}

void CAmElement::_detachAll()
{
    std::vector<std::shared_ptr<CAmElement > >::iterator itListSubjects;

    itListSubjects = mListSubjects.begin();
    while (itListSubjects != mListSubjects.end())
    {
        (*itListSubjects)->_deregister(this->getElement());
        itListSubjects = mListSubjects.erase(itListSubjects);
    }
}

int CAmElement::getObserverCount(void) const
{
    return mListObservers.size();
}

int CAmElement::getObserverCount(gc_Element_e type, std::vector<std::shared_ptr<CAmElement > > *pListElements) const
{
    int numObservers = 0;
    for (auto it : mListObservers)
    {
        if (it->getType() == type)
        {
            numObservers++;
            if (pListElements != nullptr)
            {
                pListElements->push_back(it);
            }
        }
    }

    return numObservers;
}

int CAmElement::getSubjectCount(void) const
{
    return mListSubjects.size();
}

int CAmElement::update(std::shared_ptr<CAmElement > pNotifierElement,
    const am_mainVolume_t &mainVolume)
{
    return 0;
}

void CAmElement::removeObservers()
{
    if (mListObservers.empty())
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, mType, mName, "No observers to remove");
    }

    std::vector<std::shared_ptr<CAmElement > >::iterator itListObservers;
    itListObservers = mListObservers.begin();
    while (!mListObservers.empty())
    {
        itListObservers = mListObservers.begin();
        _deregister((*itListObservers));
    }
}

bool CAmElement::_isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
    const gc_Element_e &elementType) const
{
    if (pAmElement->getType() == elementType)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CAmElement::_isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
    const std::string &elementName) const
{
    if (pAmElement->getName() == elementName)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CAmElement::_isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
    const int &elementPriority) const
{
    int priority = pAmElement->getPriority();
    if (priority == elementPriority)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CAmElement::_isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
    const struct gc_ElementTypeName_s &elementTypeName) const
{
    if ((pAmElement->getType() == elementTypeName.elementType) &&
        (pAmElement->getName() == elementTypeName.elementName))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CAmElement::_isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
    const struct gc_ElementTypeID_s &elementTypeID) const
{
    if ((pAmElement->getType() == elementTypeID.elementType) && (pAmElement->getID()
                                                                 == elementTypeID.elementID))
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::shared_ptr<CAmElement > CAmElement::getElement()
{
    return nullptr;
}

// *****************************************************************************
//                  class   L i m i t - M a p - F e a t u r e
//
// Add-On to support multiple independent volume limits
//

CAmLimitableElement::CAmLimitableElement(gc_Element_e type, const std::string &name, IAmControlReceive *pControlReceive)
    : CAmElement(type, name, pControlReceive)
{
    
}

void CAmLimitableElement::setLimitState(gc_LimitState_e limitState
        , const gc_LimitVolume_s &limitVolume, uint32_t pattern)
{
    std::ostringstream hexPattern;
    hexPattern << "0x" << std::hex << pattern << std::dec;

    if (LS_LIMITED == limitState)
    {
        // Add the volume limit in the map. In case if the same pattern is already present
        // overwrite the limit.
        mMapLimitVolumes[pattern] = limitVolume;

        LOG_FN_DEBUG(__FILENAME__, mType, mName, __func__
                , "ADDED", hexPattern.str(), limitVolume.limitType, limitVolume.limitVolume);
    }
    else if (LS_UNLIMITED == limitState)
    {
        // remove all limits whose pattern have only the bits defined in given pattern
        for (auto it = mMapLimitVolumes.begin(); it != mMapLimitVolumes.end(); )
        {
            if (0 == (it->first & (~pattern)))
            {
                LOG_FN_DEBUG(__FILENAME__, mType, mName, __func__
                        , "ERASED", hexPattern.str(), it->second.limitType, it->second.limitVolume);

                it = mMapLimitVolumes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

gc_LimitState_e CAmLimitableElement::getLimit(uint32_t pattern, gc_LimitVolume_s &limit) const
{
    const auto &it = mMapLimitVolumes.find(pattern);
    if (it == mMapLimitVolumes.end())
    {
        limit.limitType = LT_ABSOLUTE;
        limit.limitVolume = AM_VOLUME_NO_LIMIT;
        return LS_UNLIMITED;
    }

    limit = it->second;
    return LS_LIMITED;
}

void CAmLimitableElement::getLimits(std::list<gc_LimitVolume_s > &limits) const
{
    // copy direct limits
    for (const auto &limit : mMapLimitVolumes)
    {
        limits.push_back(limit.second);
    }
}

am_MuteState_e CAmLimitableElement::getMuteState() const
{
    for (const auto &limit : mMapLimitVolumes)
    {
        if ((LT_ABSOLUTE == limit.second.limitType) && (AM_MUTE == limit.second.limitVolume))
        {
            return MS_MUTED;
        }
    }

    return MS_UNMUTED;
}


} /* namespace gc */
} /* namespace am */
