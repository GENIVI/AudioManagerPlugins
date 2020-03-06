/**
 * @page actionSetVolume Action Set Volume
 *
 * This action sets the volume according to the parameters specified with the ACTION_SET_VOLUME initializer.
 * It modifies the state of source, sink and/or class elements and normally affects the audible volume.
 * This action is triggered either directly from the command side through the `hookUserVolumeChange()` function
 * or as outcome of any matching policy rule utilizing the ACTION_SET_VOLUME. 
 * In case the target volume is given in user-scale (mainVolume or mainVolumeStep) an additional step of 
 * conversion is performed, using the mapping tables configured for the main sink. 
 *
 * #### Target object selection parameters: #
 *  - __sourceName__ (string):     The name of the source.
 *  - __sinkName__ (string):       The name of the sink.
 *  - __connectionName__ (string): the entire name of the connection, specified as colon-separated
 *                                 concatenation of sourceName and sinkName
 *  - __className__ (string):      The name of the class the connection is assigned to
 *  
 * If className or connectionName is given, automatic volume balancing is always selected. Additionally,
 * connections can be selected by specifying any source or sink which is part of the connection. However,
 * @note If only a single source or sink element is specified, no className or connectionName given
 *       and the target volume is specified in internal scale, this will bypass the automatic balancing
 *       and trying to manipulate the specified element directly. If this element does not allow volume
 *       changes, the operation will fail.
 * 
 * #### Target volume parameters #
 *  - __mainVolume__ (int16):      The main volume to be set, given as absolute value in user (command side)
 *                                 coordinates.
 *  - __mainVolumeStep__ (int16):  The main volume to be set, given in user coordinates as relative change
 *                                 to the current value.
 *  - __volume__ (int16):          The volume to be set, given as absolute value in routing-side coordinates.
 *  - __volumeStep__ (int16):      The volume to be set, given in routing-side coordinates as relative change
 *                                 to the current value.
 *  .
 * Specified parameter list is scanned for a match in above order. The first match is taken, additional
 * parameters are ignored. If none is specified, the last active volume is restored.
 * 
 * @note Special consideration is implemented in case the a routing-side volume change is requested
 *       which affects an active connection with a limit applied. In this case the partial volumes
 *       along the route are balanced so that the total audible volume remains unchanged. 
 * 
 * >     For example consider a connection between Source1 and Sink1. Let us assume that the volume of Source1
 * >     and Sink1 are -10 db and -20 db respectively. Hence the total connection volume is -30 db. Now the
 * >     connection volume was limited say to a target volume of -50 db. This can be achieved by reducing
 * >     the volume of source by -20 db. New volumes are Source1 -30 db and Sink1 -20 db.  
 * >     Assume that the set volume request comes for a Sink1 asking to change the volume of sink to -10 db.
 * >     if we change the volume of sink to -10 db the total volume of the connection becomes -40 db. This is 
 * >     higher then the limit hence we need to recompute the limit for entire connection and apply the new 
 * >     limit, we need to adjust for increase of 10 db. The controller finds out the limit and applies
 * >     it to the Source1 by  -40 db. Finally the connection volume is maintained at -50 db.
 * 
 * #### Optional execution parameters: #
 *  - __rampTime__ (uint16):       The time for the volume change, specified in milliseconds.
 *  - __rampType__:                The slope characteristic for the volume change. Predefined values are
 *                                    - RAMP_GENIVI_DIRECT,
 *                                    - RAMP_GENIVI_NO_PLOP, 
 *                                    - RAMP_GENIVI_EXP_INV, 
 *                                    - RAMP_GENIVI_LINEAR and 
 *                                    - RAMP_GENIVI_EXP
 *  - __timeOut__ (uint32):        This is the maximum duration, specified in milliseconds, the execution
 *                                 may take before an error action is started.  
 *                                 Defaults to [DEFAULT_ASYNC_ACTION_TIME](@ref am::gc::DEFAULT_ASYNC_ACTION_TIME).
 * 
 * #### Example Configurations #
 * 
 * Set the mainVolume of 45 for the RadioSink.
 * @code
 *     <action type="ACTION_SET_VOLUME" sinkName="N:RadioSink" mainVolume="45"/>
 * @endcode
 * 
 * Increase the volume in decibel of the RadioSink by 5.
 * @code
 *     <action type="ACTION_SET_VOLUME" sinkName="N:RadioSink"  volumeStep="5"/>
 * @endcode
 * 
 * Decrease the mainVolume of the RadioSink by 5.
 * @code
 *     <action type="ACTION_SET_VOLUME" sinkName="N:RadioSink" mainVolumeStep="-5"/>
 * @endcode
 * 
 * Decrease the volume in deciBel of the mediaPlayer source by 10.
 * @code
 *    <action type="ACTION_SET_VOLUME" sourceName="N:mediaPlayer" volumeStep="-10"/>
 * @endcode
 * 
 *//************************************************************************//**
 * @file CAmActionSetVolume.h
 *
 * This class implements the ACTION_SET_VOLUME.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 * 
 * @copydoc actionSetVolume
 */

#ifndef GC_ACTIONSETVOLUME_H_
#define GC_ACTIONSETVOLUME_H_

#include "CAmActionSetVolumeCore.h"


namespace am {
namespace gc {

class CAmElement;
class CAmMainConnectionElement;

/***************************************************************************//**
 * @class CAmActionSetVolume
 * @copydoc CAmActionSetVolume.h
 */
class CAmActionSetVolume : public CAmActionSetVolumeCore
{
public:
    CAmActionSetVolume();
    ~CAmActionSetVolume();

protected:
    /** @name CAmActionContainer implementation *//**@{*/
    int _execute(void) override;
    int _update(int result) override;
    /**@}*/

private:
    void        _prepareLowLevelRequest(std::shared_ptr<CAmRoutePointElement> &pDirectElement);
    void        _prepareBalancedRequest(void);
    am_volume_t _getRequestedVolume(const std::shared_ptr<CAmElement > &pTargetElement);
    am_volume_t _getCurrentVolume(const std::shared_ptr<CAmElement > &pTargetElement);
    void _compensateMainVolumeChange(ConnectionMapItem_t &connectionInfo);

    // Variables in which policy engine will set the parameters.
    CAmActionParam<am_volume_t >     mVolumeParam;
    CAmActionParam<am_volume_t >     mVolumeStepParam;
    CAmActionParam<am_mainVolume_t > mMainVolumeParam;
    CAmActionParam<am_mainVolume_t > mMainVolumeStepParam;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_CLASSACTIONASETVOLUME_H_ */
