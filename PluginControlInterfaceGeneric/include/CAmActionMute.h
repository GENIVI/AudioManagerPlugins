/**
 *  @page actionSetMute Action Mute/Unmute
 * 
 * __Name:__ ACTION_MUTE<BR>
 * __Name:__ ACTION_UNMUTE<BR>
 * 
 * This action sets the mute state for the specified element. The mute state is a volume of
 *                   -3000. The internal implementation of the controller treats the mute state
 *                   as a special case of the limit volume. This action also allows a class mute,
 *                   the muting of a class means that the main sink of the active connection of that
 *                   class is muted. For details of the limit action please refer \ref actionSetLimit.
 * 
 * #### Operation mode parameter #
 *  - __muteState__([am_MuteState_e](@ref am::am_MuteState_e)): MS_MUTED for ACTION_MUTE or
 *                                 MS_UNMUTED for ACTION_UNMUTE. Automatically set by policy engine.
 * 
 * #### Target object selection parameters: #
 *  - __className__ (string):      The name of the class the connection is assigned to
 *  - __connectionName__ (string): The name of the connection, given as colon-separated conjunction
 *                                 of main source and main sink name
 *  - __sourceName__ (string):     The name of the source.
 *  - __sinkName__ (string):       The name of the sink.
 *  
 *  Only one of the above names need to be specified. The object found with the first match (scanning 
 *  order is class, connection, source, sink) will be used to memorize the given limitation. The audible volume
 *  of presently active connections which match above selection parameters are adjusted accordingly.
 * 
 *  If className or connectionName is given, automatic volume balancing is always selected. Additionally,
 *  connections can be selected by specifying any source or sink which is part of the connection. However,
 *  @note If only a single source or sink element is specified and no className or connectionName is given,
 *        this will bypass the automatic balancing and try to manipulate the specified element directly.
 *        If this element does not support volume changes, the operation will fail.
 * 
 * #### Limitation identifier parameter #
 *  - __pattern__ (uint32):        Optional identifier to distinguish between multiple independent limitations,
 *                                 understood as a bit-mask. Defaults to a pattern with all 1's (0xFFFFFFFF).
 * 
 * @note When pattern is used in an ACTION_UNMUTE, all limitations are removed whose identifiers are fully
 *       covered by the specified mask.
 * 
 * #### Optional execution parameter: #
 *  - __rampTime__ (uint16):       The time for the volume change, specified in milliseconds.
 *  - __rampType__:                The slope characteristic for the volume change. Predefined values are
 *                                   - RAMP_GENIVI_DIRECT,
 *                                   - RAMP_GENIVI_NO_PLOP, 
 *                                   - RAMP_GENIVI_EXP_INV, 
 *                                   - RAMP_GENIVI_LINEAR and 
 *                                   - RAMP_GENIVI_EXP
 *  - __timeOut__ (uint32):        This is the maximum duration, specified in milliseconds, the execution
 *                                 may take before an error action is started.  
 *                                 Defaults to [DEFAULT_ASYNC_ACTION_TIME](@ref am::gc::DEFAULT_ASYNC_ACTION_TIME).
 * 
 * #### Example Configurations: #
 * Mute the radioSink
 * @code
 *      <action type="ACTION_MUTE" sinkName="N:RadioSink"/>
 * @endcode
 * 
 * Mute the radioSink with pattern 0x80
 * @code
 *      <action type="ACTION_MUTE" sinkName="N:RadioSink" pattern="80"/>
 * @endcode
 * 
 * Unmute the RadioSink
 * @code
 *      <action type="ACTION_UNMUTE" sinkName="N:RadioSink"/>
 * @endcode
 * 
 *//***********************************************************************//**
 * @file CAmActionMute.h
 *
 * This class implements ACTION_MUTE and ACTION_UNMUTE.
 *
 * @component{AudioManager Generic Controller}
 *
 * @author Martin Koch   <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 * 
 * @copydoc actionSetMute
 *
 *//**************************************************************************/

#ifndef GC_ACTIONMUTE_H_
#define GC_ACTIONMUTE_H_


#include "CAmActionSetVolumeCore.h"

namespace am {
namespace gc {


class CAmLimitableElement;
class CAmClassElement;


/**************************************************************************//**
 * @class am::gc::CAmActionMute
 * @copydoc CAmActionMute.h
 */

class CAmActionMute : public CAmActionSetVolumeCore
{
public:
    CAmActionMute();
    virtual ~CAmActionMute();

protected:
    /** @name CAmActionContainer implementation *//**@{*/
    int _execute(void) override;
    int _undo(void) override;
    /**@}*/

private:
    void _prepareLowLevelRequest(std::shared_ptr<CAmRoutePointElement> pDirectElement, am_MuteState_e requestedMuteState);
    void _prepareBalancedRequest(am_MuteState_e requestedMuteState);

    void _updateLimitState(const std::shared_ptr<CAmLimitableElement> &pElement, am_MuteState_e requestedMuteState);

    struct undoItem_s
    {
        std::shared_ptr<CAmLimitableElement> pElement;
        gc_LimitState_e                      state;
        gc_LimitVolume_s                     limit;
    };
    std::list<undoItem_s>           mUndoList;

    CAmActionParam<am_MuteState_e > mMuteStateParam;
    CAmActionParam<uint32_t >       mPatternParam;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_ACTIONMUTE_H_ */
