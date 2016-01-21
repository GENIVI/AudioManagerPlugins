/******************************************************************************
 * @file: CAmMainConnectionActionSetLimitState.h
 *
 * This file contains the declaration of main connection action limit volume class
 * (member functions and data members) used to implement the logic of limiting
 * the volume at main connection level
 *
 * @component: AudioManager Generic Controller
 *
 * @author: Toshiaki Isogai <tisogai@jp.adit-jv.com>
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>
 *          Prashant Jain   <pjain@jp.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 *****************************************************************************/

#ifndef GC_MAINCONNECTIONACTIONSETLIMITSTATE_H_
#define GC_MAINCONNECTIONACTIONSETLIMITSTATE_H_

#include "CAmActionContainer.h"

namespace am {
namespace gc {

class CAmMainConnectionElement;
class CAmMainConnectionActionSetLimitState : public CAmActionContainer
{
public:
    /**
     * @brief It is the constructor of limit volume action at main connection level.
     * It sets the strings for classification of parameters as expected by this action.
     * Initialize the member variables with default value.
     * @param pMainConnection: pointer to MainConnection Element
     * @return none
     */
    CAmMainConnectionActionSetLimitState(CAmMainConnectionElement* PMainConnection);
    /**
     * @brief It is the destructor of limit volume action at main connection level.
     * @param none
     * @return none
     */
    virtual ~CAmMainConnectionActionSetLimitState();
protected:
    /**
     * @brief This API creates the child action object if connection is found whose volume needs to
     * be limited. It appends the child to base class for execution.
     * @param none
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _execute(void);
    /**
     * @brief This API updates the main volume of element in case of successful completion of its child.
     * @param result: status of child action execution
     * @return E_OK
     */
    int _update(const int result);
private:
    //pointer to the connection whose volume need to be changed
    CAmMainConnectionElement* mpMainConnection;
    // Variables in which parent action will set the parameters.
    CAmActionParam<am_time_t > mRampTimeParam;
    CAmActionParam<am_CustomRampType_t > mRampTypeParam;
    CAmActionParam<am_volume_t > mLimitVolumeParam;
    CAmActionParam<gc_LimitType_e > mLimitTypeParam;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_MAINCONNECTIONACTIONSETLIMITSTATE_H_ */
