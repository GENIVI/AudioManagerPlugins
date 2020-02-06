/*******************************************************************************
 *  \copyright (c) 2016 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Jens Lorenz, jlorenz@de.adit-jv.com 2016
 *           Mattia Guerra, mguerra@de.adit-jv.com 2016
 *
 *  \copyright The MIT License (MIT)
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 ******************************************************************************/

#ifndef CAMRAALSALOGGING_H_
#define CAMRAALSALOGGING_H_

#include "CAmLogWrapper.h"


namespace am
{

/**
 * \brief Short-hand notations for a logging context dedicated to the ALSA Routing-Adapter
 *
 * This class is implemented as Singleton
 */
class CAmRaAlsaLogging
{
public:
    /**
     * Allows to retrieve the Singleton
     */
    static CAmRaAlsaLogging *Instance();
    /**
     * dtor
     */
    virtual ~CAmRaAlsaLogging();

    /**
     * provide access to logging context
     */
    inline static IAmLogContext &getContext()
    {
        return Instance()->mContext;
    }

private:
    /**
     * private ctor being a singleton
     */
    CAmRaAlsaLogging();

    IAmLogContext &mContext;
    static CAmRaAlsaLogging *mpLogging;
};

/**
 * logs given values with debuglevel with the plugin context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logAmRaDebug(T value, TArgs... args)
{
    CAmRaAlsaLogging::getContext().debug(value, args...);
}

/**
 * logs given values with infolevel with the plugin context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logAmRaInfo(T value, TArgs... args)
{
    CAmRaAlsaLogging::getContext().info(value, args...);
}

/**
 * logs given values with errorlevel with the plugin context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logAmRaError(T value, TArgs... args)
{
    CAmRaAlsaLogging::getContext().error(value, args...);
}

} /* namespace am */
#endif /* CAMRAALSALOGGING_H_ */
