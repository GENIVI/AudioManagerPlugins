/******************************************************************************
 * @file: CAmTriggerQueue.cpp
 *
 * This file contains the declaration of abstract class used to provide
 * the interface for policy engine side to provide API to framework to pass the
 * trigger and get actions related to trigger from policy engine
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

#include "CAmTriggerQueue.h"

namespace am {

namespace gc {

CAmTriggerQueue *CAmTriggerQueue::mpTriggerQueue = NULL;
CAmTriggerQueue::CAmTriggerQueue()
    : mlistTrigger()
    , mlistPriority()
{
}

am_Error_e CAmTriggerQueue::queue(gc_Trigger_e triggerType, gc_TriggerElement_s *triggerData)
{
    mlistTrigger.push_back(std::make_pair(triggerType, triggerData));
    return E_OK;
}

am_Error_e CAmTriggerQueue::queueWithPriority(gc_Trigger_e triggerType, gc_TriggerElement_s *triggerData)
{
    mlistPriority.push_back(std::make_pair(triggerType, triggerData));
    return E_OK;
}

gc_TriggerElement_s *CAmTriggerQueue::dequeue(gc_Trigger_e &triggerType)
{
    gc_TriggerElement_s                           *pTriggerElement = NULL;
    std::pair<gc_Trigger_e, gc_TriggerElement_s *> triggerpair;

    if (!mlistPriority.empty())  // check priority queue first
    {
        triggerpair     = mlistPriority.front();
        pTriggerElement = triggerpair.second;
        triggerType     = triggerpair.first;
        mlistPriority.pop_front();
    }
    else if (!mlistTrigger.empty())
    {
        triggerpair     = mlistTrigger.front();
        pTriggerElement = triggerpair.second;
        triggerType     = triggerpair.first;
        mlistTrigger.pop_front();
    }

    return pTriggerElement;
}

void CAmTriggerQueue::getSnapShot(std::vector<std::pair<gc_Trigger_e, const gc_TriggerElement_s *> > &queueSnapShot)
{
    queueSnapShot.clear();
    queueSnapShot.reserve(mlistPriority.size() + mlistTrigger.size());
    for (const auto &trigger : mlistPriority)
    {
        // give away trigger data pointer, but prohibit modification outside of this class
        queueSnapShot.push_back(std::pair<gc_Trigger_e, const gc_TriggerElement_s *>(trigger.first,
                const_cast<const gc_TriggerElement_s *>(trigger.second)));
    }
    for (const auto &trigger : mlistTrigger)
    {
        // give away trigger data pointer, but prohibit modification outside of this class
        queueSnapShot.push_back(std::pair<gc_Trigger_e, const gc_TriggerElement_s *>(trigger.first,
                const_cast<const gc_TriggerElement_s *>(trigger.second)));
    }
}

CAmTriggerQueue *CAmTriggerQueue::getInstance()
{
    if (mpTriggerQueue == NULL)
    {
        mpTriggerQueue = new CAmTriggerQueue();
    }

    return mpTriggerQueue;
}

void CAmTriggerQueue::freeInstance()
{
    if (NULL != mpTriggerQueue )
    {
        delete mpTriggerQueue;
        mpTriggerQueue = NULL;
    }
}

}   /* namespace gc */

} /* namespace am */
