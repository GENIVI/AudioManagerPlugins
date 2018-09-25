/******************************************************************************
 * @file: CAmClassActionSetLimitState.h
 *
 * This file contains the declaration of user action limit volume class
 * (member functions and data members) used to implement the logic of limiting
 * the volume of connection at user level
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

#ifndef GC_CLASSACTIONSETLIMITSTATE_H_
#define GC_CLASSACTIONSETLIMITSTATE_H_

#include "CAmActionContainer.h"
#include "CAmClassElement.h"
namespace am {
/**
 * @page limitvolumeaction User Action Limit volume
 *
 * Name: CAmClassActionLimitVolume<BR>
 * Responsibility: This action limits the volume of a class. This is a class level action.
 * For example if a base class is limited, then the active connections of base class are limited
 * till the ACTION_UNLIMIT_VOL is executed. When this action is executed for a class, if there is
 * an active connection for that class it's volume is limited. In future for all the new
 * connection for this class volume would be limited. Till the unlimit volume gets executed this
 * class will be limited. Additionally this action takes the hexadecimal bit pattern for limiting.
 * The meaning of the pattern is while limiting the pattern is logically ORed with the existing
 * pattern, and while unlimiting the pattern is logically ANDed with the NOT of the pattern. If
 * while unlimiting the pattern of the class is zero the unlimiting is done.
 *    For example lets say limit pattern for a class is initialized to zero, if INT class limits with
 * a pattern of 0x01 then the pattern after logically ORing becomes 0x01. If the PHONE class subsequently
 * limits with the pattern 0x02 the the pattern for base class is 0x03. If now PHONE class unlimits
 * with pattern 0x02 then the pattern is 0x03 & (~0x02) = 0x01, since the pattern is still non zero
 * the volume is not unlimited. By default if no pattern is provided then the pattern used is
 * 0xFFFFFFFF, which means the limit would mask all the bits and the unlimit would clear all the
 * bits.<BR>
 *
 * Mandatory parameters: Either or any of the below parameter are mandatory<BR>
 *  - classname: The name of the class<BR>
 *  - targetvolume: The volume of the main connection would be limited to this value.<BR>
 * Optional parameters:<BR>
 *  - ramptime: The ramp time used for volume change.<BR>
 *  - ramptype: The ramp type used for volume change.<BR>
 *  - pattern: The pattern for the volume limit.<BR>
 *  - timeout: This is the timeout for action execution. If not specified then
 *  DEFAULT_ASYNC_ACTION_TIME is used as timeout.<BR>
 */

namespace gc {

class CAmClassActionSetLimitState : public CAmActionContainer
{
public:
    /**
     * @brief It is the constructor of Limit Volume action at user level.
     * It sets the strings for classification of parameters as expected by this action.
     * Initialize the member variables with default value.
     * @param pRoutingManager: pointer to CAmRoutingManager Class object
     * @return none
     */
    CAmClassActionSetLimitState(CAmClassElement *pClassElement);
    /**
     * @brief It is the destructor of Limit Volume action at user level.
     * @param none
     * @return none
     */
    virtual ~CAmClassActionSetLimitState();
protected:
    /**
     * @brief This API creates the child action object if connection is found whose volume needs
     * to be limited. It appends the child to base class for execution.
     * @param none
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _execute(void);
private:
    // Variables in which policy engine will set the parameters.
    CAmActionParam<am_time_t > mRampTimeParam;
    CAmActionParam<am_CustomRampType_t > mRampTypeParam;
    CAmActionParam<am_volume_t > mTargetVolumeParam;
    CAmActionParam<am_volume_t > mOffsetVolumeParam;
    CAmActionParam<uint32_t > mPatternParam;
    CAmActionParam<std::string > mSinkNameParam;
    CAmActionParam<std::string > mSourceNameParam;
    CAmActionParam<gc_LimitState_e > mOpRequestedParam;
    CAmClassElement *mpClassElement;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_CLASSACTIONSETLIMITSTATE_H_ */
