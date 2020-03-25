/*
 * TestEventObserver.h
 *
 *  Created on: 22-Mar-2017
 *      Author: satish
 */

#ifndef PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCONTROLSENDTEST_TESTEVENTOBSERVER_H_
#define PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCONTROLSENDTEST_TESTEVENTOBSERVER_H_

// This file implements update function of IAmEventObserver and purely will be used for Testing purpose only.

#include <vector>
#include "IAmEventObserver.h"

namespace am {
namespace gc {
class TestEventObsever : public IAmEventObserver
{
public:
    TestEventObsever();
    ~TestEventObsever();

    int update(const int result);

};

}
}

#endif /* PLUGINCONTROLINTERFACEGENERIC_TEST_CAMCONTROLSENDTEST_TESTEVENTOBSERVER_H_ */
