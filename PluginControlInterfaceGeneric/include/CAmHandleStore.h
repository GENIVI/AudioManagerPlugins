/**************************************************************************//**
 * @file  CAmHandleStore.h
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

#ifndef GC_HANDLESTORE_H_
#define GC_HANDLESTORE_H_

#include "CAmTypes.h"


namespace am
{
namespace gc
{

class IAmEventObserver;

/**************************************************************************//**
 * @class  CAmHandleStore
 *
 * @copydoc CAmHandleStore.h
 */
class CAmHandleStore
{
public:
    // lazy-initialized Singleton instance
    static CAmHandleStore &instance();

    void saveHandle(am_Handle_s handle, IAmEventObserver *pObserver);
    void clearHandle(am_Handle_s handle);

    // call the notify() function of all subscribers registered for given handle
    void notifyAsyncResult(am_Handle_s handle, am_Error_e Error);

private:
    CAmHandleStore();
    ~CAmHandleStore();

    std::map<uint16_t, IAmEventObserver * > mMapHandles;
};


}  // namespace gc
}  // namespace am


#endif /* GC_HANDLESTORE_H_ */
