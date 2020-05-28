/******************************************************************************
 * @file: CAmDomainElement.cpp
 *
 * This file contains the declaration of domain element used to provide
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
#include "CAmDomainElement.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

CAmDomainElement::CAmDomainElement(const gc_Domain_s &domain, IAmControlReceive *pControlReceive)
    : CAmElement(ET_DOMAIN, domain.name, pControlReceive)
    , mDomain(domain)
{
}

CAmDomainElement::~CAmDomainElement()
{
}

am_Error_e CAmDomainElement::_register(void)
{
    am_Error_e    result   = E_DATABASE_ERROR;
    am_domainID_t domainID = mDomain.domainID;
    if (E_OK == mpControlReceive->enterDomainDB(mDomain, domainID))
    {
        mID = domainID;
        if ((mDomain.domainID != 0) && (mID != mDomain.domainID))
        {
            LOG_FN_WARN(__FILENAME__, __func__, mName
                , "NOT using proposed ID", mDomain.domainID, "new ID is", mID);
        }

        result = E_OK;
    }

    return result;
}

am_Error_e CAmDomainElement::_unregister(void)
{
    am_Error_e result = E_DATABASE_ERROR;
    if (E_OK == mpControlReceive->removeDomainDB(getID()))
    {
        setID(0);
        result = E_OK;
    }

    return E_OK;

}

am_Error_e CAmDomainElement::setState(const am_DomainState_e state)
{
    return mpControlReceive->changeDomainStateDB(state, getID());
}

am_Error_e CAmDomainElement::getState(int &state) const
{
    am_Domain_s domainInfo;
    am_Error_e result = getDomainInfo(domainInfo);
    if (result == E_OK)
    {
        state = domainInfo.state;
    }
    return result;
}

am_Error_e CAmDomainElement::getDomainInfo(am_Domain_s &domainInfo) const
{
    std::vector<am_Domain_s > listDomains;
    am_Error_e                result = mpControlReceive->getListDomains(listDomains);
    if (result != E_OK)
    {
        return result;
    }

    for (auto &dom : listDomains)
    {
        if (dom.domainID == mID)
        {
            domainInfo  = dom;
            return E_OK;
        }
    }

    // not in list
    return E_UNKNOWN;
}

const gc_Domain_s &CAmDomainElement::getConfig() const
{
    return mDomain;
}

}
}
