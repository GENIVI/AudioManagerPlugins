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
 *//**************************************************************************/


#include "CAmActionCommand.h"
#include "CAmActionRundown.h"
#include "CAmDomainElement.h"
#include "CAmMainConnectionElement.h"
#include "CAmLogger.h"
#include "CAmHandleStore.h"

using namespace std;


namespace am {
namespace gc {

/**************************************************************************//**
 * @class CAmActionTransfer
 *
 * Helper class to offer a single main connection to domain candidates
 */
class CAmActionRundown::CAmActionTransfer  : public CAmActionCommand
{
    public:
        CAmActionTransfer(const std::vector<std::shared_ptr<CAmDomainElement>> &listDomains
                , const std::shared_ptr<CAmMainConnectionElement> &mainConnection)
            : CAmActionCommand("CAmActionTransfer")
            , mMainConnection(mainConnection)
            , mListDomains(listDomains)
            , mDomainIterator(mListDomains.begin())
            , mHandle({H_UNKNOWN, 0})
        {

        }

        virtual ~CAmActionTransfer()
        {

        }

    protected:
        int _execute(void) override
        {
            auto *pControlReceive = mMainConnection->getControlReceive();
            while (pControlReceive && mDomainIterator != mListDomains.end())
            {
                am_domainID_t domainID = (*mDomainIterator)->getID();
                LOG_FN_INFO(__FILENAME__, "CAmActionTransfer::_execute()"
                        , getStatus(), "offering main connection #", mMainConnection->getID()
                        , mMainConnection->getName()
                        , "to domain #", domainID, (*mDomainIterator)->getName());

                if(pControlReceive->transferConnection(mHandle
                        , mMainConnection->getID(), domainID) == E_OK)
                {
                    // request is out - so wait for acknowledgment
                    CAmHandleStore::instance().saveHandle(mHandle, this);
                    return E_WAIT_FOR_CHILD_COMPLETION;
                }

                mDomainIterator++;
            }

            return E_OK;
        }

        int _update(const int result) override
        {
            return result;
        }

        // override this function to allow re-entering the same action
        // for next domain in list
        int update(const int result) override
        {
            if ((getStatus() == AS_EXECUTING) && (result != E_OK))
            {
                // domain has rejected - try offering to the next domain in list
                mDomainIterator++;
                LOG_FN_DEBUG(__FILENAME__, "CAmActionTransfer::update"
                        , getStatus(), (am_Error_e)result);

                setStatus(AS_NOT_STARTED);
                return execute();
            }

            // continue in normal flow, so delegate to base class
            return CAmActionCommand::update(result);
        }

    private:
        const std::shared_ptr<CAmMainConnectionElement>       mMainConnection;
        const std::vector<std::shared_ptr<CAmDomainElement>> &mListDomains;
        std::vector<std::shared_ptr<CAmDomainElement>>
                ::const_iterator                              mDomainIterator;
        am_Handle_s                                           mHandle;
};

/**************************************************************************//**
 * @class CAmActionRundown
 *
 */
CAmActionRundown::CAmActionRundown(std::function<void()> finalizeRundown)
    : CAmActionContainer("CAmActionRundown")
    , mFinalizeRundown(finalizeRundown)
    , mListDomains()
    , mListMainConnections()
{
    // collect domains which are flagged as 'early' or DS_INDEPENDENT_RUNDOWN
    CAmDomainFactory::getListElements(mListDomains);
    for (auto it = mListDomains.begin(); it != mListDomains.end(); /* it++ */)
    {
        am_Domain_s domainInfo;
        (*it)->getDomainInfo(domainInfo);
        LOG_FN_DEBUG(__FILENAME__, __func__, "domain #", (*it)->getID(), (*it)->getName()
                , domainInfo.state, "early = ", domainInfo.early);
        if ((domainInfo.state != DS_INDEPENDENT_RUNDOWN) || domainInfo.early)
        {
            it = mListDomains.erase(it);
            continue;
        }

        it++;
    }

    // check if we have active connections which might be intended to survive our shutdown
    CAmMainConnectionFactory::getListElements(mListMainConnections);
    for (auto it = mListMainConnections.begin(); it != mListMainConnections.end();  /* it++ */)
    {
        LOG_FN_DEBUG(__FILENAME__, __func__, "mainConnection #", (*it)->getID(), (*it)->getState());
        switch ((*it)->getState())
        {
            case CS_CONNECTED:
            case CS_CONNECTING:
            case CS_SUSPENDED:
                it++;
                break;

            default:
                // not a candidate - remove from list
                it = mListMainConnections.erase(it);
                break;
        }
    }
}

CAmActionRundown::~CAmActionRundown()
{

}

int CAmActionRundown::_execute(void)
{

    for (auto &mc : mListMainConnections)
    {
        append(new CAmActionTransfer(mListDomains, mc));
    }

    return E_OK;
}

int CAmActionRundown::_update(const int result)
{
    mFinalizeRundown();

    return E_OK;
}


} }  // namespace am::gc

