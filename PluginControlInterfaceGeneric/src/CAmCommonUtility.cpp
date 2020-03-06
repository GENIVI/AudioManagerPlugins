/******************************************************************************
 * @file: CAmCommonUtility.cpp
 *
 * This file contains the definition of CAmCommonUtility class, it is used to
 * provide interface for common utility/helper functions
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com>
 *          Nilkanth Ahirrao and Kapildev Patel  <kpatel@jp.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/
#include "CAmLogger.h"
#include "CAmCommonUtility.h"

namespace am {
namespace gc {



am::IAmLogContext &GenericControllerLogger(am_LogLevel_e loglevel)
{
    // Lazy initialization. Will be assigned on first use.
    static am_LogLevel_e current = static_cast<am_LogLevel_e>(-1);
    static IAmLogContext &context = CAmLogWrapper::instance()->registerContext(GC_CONTEXT_STRING
            , GC_CONTEXT_DESCRIPTION);

    // if log-level does not match, re-register same context with new level
    if ((loglevel >= 0) && (current != loglevel))
    {
        context = CAmLogWrapper::instance()->registerContext(GC_CONTEXT_STRING
                , GC_CONTEXT_DESCRIPTION, loglevel, LS_ON);
        current = loglevel;
    }

    // return instance
    return context;
}


am_Error_e CAmCommonUtility::parseString(const std::string &delimiter, std::string &inputString,
    std::vector<std::string > &listOutput)
{
    if (!inputString.empty())
    {
        size_t      pos = 0;
        std::string token;
        bool        delimiterFound = false;
        while ((pos = inputString.find(delimiter)) != std::string::npos)
        {
            delimiterFound = true;
            token          = inputString.substr(0, pos);
            inputString.erase(0, pos + delimiter.length());
            if (!token.empty())
            {
                listOutput.push_back(token);
            }
        }

        if (!inputString.empty() && delimiterFound)
        {
            listOutput.push_back(inputString);
            return E_OK;
        }
        else
        {
            return E_NOT_POSSIBLE;
        }
    }
    else
    {
        LOG_FN_INFO(__FILENAME__, __func__, "input string is empty");
        return E_NOT_POSSIBLE;
    }
}

CAmCommonUtility::~CAmCommonUtility()
{

}

} /* namespace gc */
} /* namespace am */
