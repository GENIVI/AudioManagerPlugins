/*
 * CAmActionCommandTest.h
 *
 *  Created on: 11-May-2016
 *      Author: dipesh
 */

#ifndef CAMACTIONCOMMANDTEST_H_
#define CAMACTIONCOMMANDTEST_H_

#include "MockCAmActionCommandTest.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am
{
namespace gc {
class CAmActionCommandTest : public ::testing::Test
{
public:
    CAmActionCommandTest ();
    ~CAmActionCommandTest ();
};

}
}

#endif /* CAMACTIONCOMMANDTEST_H_ */
