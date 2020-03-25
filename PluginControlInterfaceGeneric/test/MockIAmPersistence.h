/*******************************************************************************
 *  \copyright (c) 2018 Advanced Driver Information Technology.
 *                   ADIT is a joint venture company of
 *   Robert Bosch GmbH/Robert Bosch Car Multimedia GmbH and DENSO Corporation
 *
 *  \author: Yuki Tsunashima <ytsunashima@jp.adit-jv.com> 2018
 *           Kapildev Patel, Satish Dwivedi <kpatel@jp.adit-jv.com> 2018
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

#ifndef MOCKIAMPERSISTENCE_H_
#define MOCKIAMPERSISTENCE_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define READ_SIZE  1024
#define WRITE_SIZE 1024

namespace am {
namespace gc {
class MockIAmPersistence : public IAmPersistence
{

public:
    MockIAmPersistence()
    {
    }

    virtual ~MockIAmPersistence()
    {
    }

    MOCK_METHOD1(open,
        am_Error_e(const std::string &appName));

    MOCK_METHOD3(read,
        am_Error_e(const std::string &keyName, std::string &readData, int dataSize = READ_SIZE));

    MOCK_METHOD3(write,
        am_Error_e(const std::string &keyName, const std::string &writeData, int dataSize = WRITE_SIZE));

    MOCK_METHOD0(close,
        am_Error_e(void));
};

}
}

#endif /* MOCKIAMPERSISTENCE_H_ */
