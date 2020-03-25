/*
 * TestEventObserver.cpp
 *
 *  Created on: 22-Mar-2017
 *      Author: satish
 */

#include "TestEventObserver.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

TestEventObsever::TestEventObsever()
{
}

TestEventObsever::~TestEventObsever()
{
}

int TestEventObsever :: update(int result)
{
    LOG_FN_ENTRY();

    LOG_FN_INFO("** Test Observer Update Function **");
    printf(" Updated Result value is %d \n", result);

    return result;
}
