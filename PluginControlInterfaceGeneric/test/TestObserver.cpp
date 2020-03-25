/*
 * TestObserver.cpp
 *
 *  Created on: 22-Mar-2017
 *      Author: satish
 */

#include "TestObserver.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

TestObserver::TestObserver()
{
}

TestObserver::~TestObserver()
{
}

int TestObserver :: update(int result)
{
    LOG_FN_ENTRY();

    LOG_FN_INFO("** Test Observer Update Function **");
    printf(" Updated Result value is %d \n", result);

    return result;
}

}
}
