/*
 * TestObserver.h
 *
 *  Created on: 22-Mar-2017
 *      Author: satish
 */

#ifndef PLUGINCONTROLINTERFACEGENERIC_TEST_TESTOBSERVER_H_
#define PLUGINCONTROLINTERFACEGENERIC_TEST_TESTOBSERVER_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <vector>
#include "IAmEventObserver.h"

namespace am {
namespace gc {
class TestObserver : public IAmEventObserver
{
public:
    TestObserver();
    ~TestObserver();

    int update(const int result);

};

}
}

#endif /* PLUGINCONTROLINTERFACEGENERIC_TEST_TESTOBSERVER_H_ */
