/******************************************************************************
 * @file: CAmClassActionSetVolume.h
 *
 * This file contains the declaration of user action set volume class
 * (member functions and data members) used to implement the logic of setting
 * the volume of element at user level
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

#ifndef GC_CLASSACTIONSETVOLUME_H_
#define GC_CLASSACTIONSETVOLUME_H_

#include "CAmActionContainer.h"

namespace am {
/**
 * @page setvolumeaction User Action Set Volume
 *
 * Name: CAmClassActionSetVolume<BR>
 * Responsibility: This action sets the source or sink volume.<BR>
 *
 * Mandatory parameters:<BR>
 *  - sourcename: The name of the source.<BR>
 *  - sinkname: The name of the sink.<BR>
 *  - targetvolume: The main volume to be set.<BR>
 * Optional parameters:<BR>
 *  - ramptime: The ramptime for volume change.<BR>
 *  - ramptype: The ramptype for the volume change.<BR>
 *  - timeout: This is the timeout for action execution. If not specified then
 *  DEFAULT_ASYNC_ACTION_TIME is used as timeout.<BR>
 *
 */

namespace gc {

class CAmElement;
class CAmClassElement;

class CAmClassActionSetVolume : public CAmActionContainer
{
public:
    /**
     * @brief It is the constructor of set volume action at user level.
     * It sets the strings for classification of parameters as expected by this action.
     * Initialize the member variables with default value.
     * @param pRoutingManager: pointer to CAmRoutingManager Class object
     * @return none
     */
    CAmClassActionSetVolume(CAmClassElement *pClassElement);
    /**
     * @brief It is the destructor of set volume action at user level.
     * @param none
     * @return none
     */
    ~CAmClassActionSetVolume();
protected:
    /**
     * @brief This API creates the child action object if element whose volume need to be
     * set is found. It appends the child to base class for execution.
     * If sink is in mute state then only its main volume is updated and action is completed
     * @param none
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    int _execute(void);
private:
    int _deleteSetVolumeActionList(std::vector<IAmActionCommand* >& listActions);
    int _CreateSetVolumeActionList(
                    std::vector<std::pair<CAmElement*, gc_LimitVolume_s > >& listLimitElement,
                    std::vector<IAmActionCommand* >& listActions);
    /**
     * @brief This API is used to fetch the parameters as set by Policy engine for this action.
     * It checks weather element support volume change or not.
     * @param none
     * @return E_OK on success
     *         E_NOT_POSSIBLE on error
     */
    am_Error_e _getParameters();
    CAmElement* mpElement;
    CAmClassElement *mpClassElement;
    // Variables in which policy engine will set the parameters.
    CAmActionParam<am_time_t > mRampTimeParam;
    CAmActionParam<am_CustomRampType_t > mRampTypeParam;
    CAmActionParam<am_volume_t > mVolumeParam;
    CAmActionParam<am_volume_t > mVolumeStepParam;
    CAmActionParam<am_mainVolume_t > mMainVolumeParam;
    CAmActionParam<am_mainVolume_t > mMainVolumeStepParam;
    CAmActionParam<std::string > mSinkNameParam;
    CAmActionParam<std::string > mSourceNameParam;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_CLASSACTIONASETVOLUME_H_ */
