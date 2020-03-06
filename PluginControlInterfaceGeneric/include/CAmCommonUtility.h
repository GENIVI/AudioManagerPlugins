/******************************************************************************
 * @file: CAmCommonUtility.h
 *
 * This file contains the declaration of CAmCommonUtility class, it is used to
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
#ifndef GC_COMMON_UTILITY_H_
#define GC_COMMON_UTILITY_H_

#include "CAmTypes.h"

namespace am {
namespace gc {

class CAmCommonUtility
{
public:
    static am_Error_e parseString(const std::string &delimiter, std::string &inputString,
        std::vector<std::string > &listOutput);

    virtual ~CAmCommonUtility();

private:
    CAmCommonUtility(void);   // is private because instance should not be created
};

}
}
#endif /* GC_COMMON_UTILITY_H_ */
