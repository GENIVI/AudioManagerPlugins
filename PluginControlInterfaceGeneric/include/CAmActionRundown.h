/**************************************************************************//**
 * @file  CAmActionRundown.h
 *
 * This file contains the implementation of asynchronous controller rundown
 * while trying to transfer remaining active connections to routing-side
 * domain applications
 *
 * @component{AudioManager Generic Controller}
 *
 * @author   Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2020 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 * @copydoc connectionTransfer
 *
 * Finally, completion of the rundown sequence is delegated back to the appropriate
 * member function of CAmControllerPlugin
 *
 *//***********************************************************************//**
 * @page connectionTransfer  Action Transfer Connections
 *
 * <b>Name:</b> ACTION_TRANSFER<BR>
 *
 * Connections which are still active (in state CS_CONNECTED or CS_SUSPENDED) and shall be kept
 * alive beyond termination of the audio-manager process need to be handed over to the routing
 * adapter before the daemon process terminates.
 *
 * This picture shows the principle of the late rundown:
 * @image html late.png
 *
 * Selectable for handover are all domains marked with status flag DS_INDEPENDENT_RUNDOWN indicating
 * their capability of taking care of late connections autonomously. They shall be considered for the
 * hand-over offering through asynchronous, non-blocking function @ref am::IAmControlReceive::transferConnection
 * "transferConnection()". Below sequence chart shows the usage of this function and its forwarding
 * and acknowledging siblings:
 *
 * @image html early_transfer.png
 *
 * After positive acknowledgment through the routing adapter the AM is no longer responsible
 * for the connection and its elements, so the subsequent deregisterSource/Sink/Domain()
 * requests do not harm.
 *
 * @note In case the audio-manager is restarted and said connection still exists under responsibility
 *       of a routing adapter, above function registerEarlyConnection() can be invoked again.
*//**************************************************************************/

#ifndef GC_ACTIONRUNDOWN_H_
#define GC_ACTIONRUNDOWN_H_


#include "CAmActionContainer.h"

namespace am {
namespace gc {


class CAmDomainElement;
class CAmMainConnectionElement;


/**************************************************************************//**
 *  @class CAmActionRundown
 *
 *  @copydoc CAmActionRundown.h
 */
class CAmActionRundown : public CAmActionContainer
{

public:
    CAmActionRundown(std::function<void()> finalizeRundown);
    virtual ~CAmActionRundown();

protected:
    int _execute(void) override;
    int _update(const int result) override;

private:
    class CAmActionTransfer;

    std::function<void()>                                  mFinalizeRundown;
    std::vector<std::shared_ptr<CAmDomainElement>>         mListDomains;
    std::vector<std::shared_ptr<CAmMainConnectionElement>> mListMainConnections;

};

} }   // namespace am::gc

#endif /* GC_ACTIONRUNDOWN_H_ */
