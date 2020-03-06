/**************************************************************************//**
 * @file  CAmHandleStore.cpp
 *
 * Utility class to manage
 *  - memorizing am_Handle_s objects assigned to outgoing
 *    requests and 
 *  - notifying registered clients on incoming responses.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors   Martin Koch<mkoch@de.adit.jv.com>
 *
 * @copyright (c) 2020 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *//**************************************************************************/


#include "CAmHandleStore.h"
#include "IAmEventObserver.h"
#include "CAmLogger.h"


namespace am
{
namespace gc
{

/**************************************************************************//**
 * @class  CAmHandleStore
 *
 * Utility class to manage
 *  - memorizing am_Handle_s objects assigned to outgoing
 *    requests and 
 *  - notifying registered clients on incoming responses.
 */

CAmHandleStore::CAmHandleStore()
{
    
}

CAmHandleStore::~CAmHandleStore()
{
    for (auto it = mMapHandles.begin(); it != mMapHandles.end();  /* ++it */)
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "dropping ORPHANED handle:"
                , (am_Handle_e)(it->first >> 10), "#", it->first & 0x3ff);
        it = mMapHandles.erase(it);
    }
}

CAmHandleStore &CAmHandleStore::instance()
{
    // lazy-initialized Singleton instance
    static CAmHandleStore _theOneAndOnly;

    return _theOneAndOnly;
}


void CAmHandleStore::saveHandle(am_Handle_s handle, IAmEventObserver *pObserver)
{
    uint16_t uHandle = (((uint16_t)handle.handleType) << 10) + (handle.handle);
    if (mMapHandles.count(uHandle))
    {
        LOG_FN_ERROR(__FILENAME__, __func__, "DUPLICATE handle:", handle.handleType, "#", handle.handle);
    }

    mMapHandles[uHandle] = pObserver;
    LOG_FN_INFO(__FILENAME__, __func__, handle.handleType, "#", handle.handle);
}

void CAmHandleStore::clearHandle(am_Handle_s handle)
{
    uint16_t uHandle = (((uint16_t)handle.handleType) << 10) + (handle.handle);
    size_t removed = mMapHandles.erase(uHandle);
    if (removed)
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, handle.handleType, "#", handle.handle
                , " new map size is", mMapHandles.size());
    }
    else
    {
        LOG_FN_ERROR(__FILENAME__, __func__, handle.handleType, "#", handle.handle, "NOT FOUND");
    }
}

void CAmHandleStore::notifyAsyncResult(am_Handle_s handle, am_Error_e error)
{
    uint16_t uHandle = (((uint16_t)handle.handleType) << 10) + (handle.handle);
    if (mMapHandles.count(uHandle))
    {
        LOG_FN_INFO(__FILENAME__, __func__, error, handle.handleType, "#", handle.handle);

        IAmEventObserver *pObserver = mMapHandles[uHandle];
        if (pObserver)
        {
            pObserver->update(error);
        }
        mMapHandles.erase(uHandle);
    }
    else
    {
        // In case ACK/NACK is received for a unknown Handle means that we have received it
        // after a Timeout and it should be dropped.
        LOG_FN_ERROR(__FILENAME__, __func__, "NO matching entry found for"
                , handle.handleType, "#", handle.handle, "Error = ", error);
    }
}


}  // namespace gc
}  // namespace am

