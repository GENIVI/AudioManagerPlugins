/******************************************************************************
 * @file: CAmSystemActionSetProperties.h
 *
 * This file contains the declaration of set system properties class
 * (member functions and data members) used to implement the logic of setting
 * the system properties at router level
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Akshatha Nanjundaswamy <Nanjundaswamy.akshatha@in.bosch.com>
 *  *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#ifndef GC_ACTIONSETSYSTEMPROPERTIES_H_
#define GC_ACTIONSETSYSTEMPROPERTIES_H_

#include "CAmLogger.h"
#include "CAmActionCommand.h"
#include "CAmSystemElement.h"

namespace am {
namespace gc {

class CAmSystemActionSetProperties : public CAmActionCommand
{
public:
    /**
     * @brief It is the constructor of register action at system level.
     * It sets the strings for classification of parameters as expected by this action.
     * Initialize the member variables with default value.
     * @param pRoutingManager: pointer to CAmRoutingManager Class object
     * @return none
     */
    CAmSystemActionSetProperties(std::shared_ptr<CAmSystemElement > pSystem);
    /**
     * @brief It is the destructor of register action at system level.
     * @param none
     * @return none
     */
    virtual ~CAmSystemActionSetProperties(void);

protected:
    /**
     * @brief This API based on parameter set invokes the routing manager API to register the element
     * @param none
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _execute(void);


private:
    IAmControlReceive                 *mpControlReceive;
    std::shared_ptr<CAmSystemElement > mpSystem;
    // Variables in which parent action will set the parameters.
    CAmActionParam<std::vector<am_SystemProperty_s > > mListPropertyValueParam;
    std::vector<am_SystemProperty_s > mListSystemProperty;
    std::vector<am_SystemProperty_s > mListOldSystemProperty;

};

} /* namespace gc */
} /* namespace am */
#endif /* GC_ACTIONSETSYSTEMPROPERTIES_H_ */
