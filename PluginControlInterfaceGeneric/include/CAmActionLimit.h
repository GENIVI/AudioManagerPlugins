/**************************************************************************//**
 * @page actionSetLimit Action Limit/UnLimit
 * 
 * __Name:__ ACTION_LIMIT<BR>
 * __Name:__ ACTION_UNLIMIT<BR>
 *
 * These actions install / remove a maximum value which the audible volume may not exceed. Such limits are
 * understood as temporary and limit the routing-side requests, but do not affect the nominal value
 * of a volume. The specified limit is memorized in given element; in case of a connection through its
 * assigned class element.
 * 
 * Multiple limitations can be applied, distinguishable through a unique pattern parameter. From the
 * list of memorized limitations the most restrictive (lowest volume) becomes effective for the active
 * connection(s). If no connection is presently active, the memorized limitation is applied once a matching
 * connection becomes active.
 * 
 * Limit can be of two types: LT_ABSOLUTE or LT_RELATIVE. Absolute limit means that the volume of the
 * connection cannot be higher than the specified limit. Relative limit means the audible volume is set
 * such that it differs from the volume of any related (mixed) connections by given value.
 * 
 * >  Example: lets say a connection of a class BASE involving Source1 and Sink1. The volume of Source1 
 * >  and Sink1 are -10 db and -20 db respectively, thus the effective volume of the connection is
 * >  -30 db. If, for example, the class is limited by absolute volume of -100 db then the connection
 * >  volume cannot be higher than -100 db. 
 * 
 * #### Operation mode parameter #
 *  - __limitState__([gc_LimitState_e](@ref am::gc::gc_LimitState_e)): LS_LIMITED for ACTION_LIMIT, 
 *                                LS_UNLIMITED for ACTION_UNLIMIT. Automatically set by policy engine.
 * 
 * #### Target object selection parameters: #
 *  - __className__ (string):      The name of the class the connection is assigned to
 *  - __connectionName__ (string): The name of the connection, given as colon-separated conjunction
 *                                 of main source and main sink name
 *  - __sourceName__ (string):     The name of the source.
 *  - __sinkName__ (string):       The name of the sink.
 *  
 *  Only one of the above names need to be specified. The object found with the first match (scanning 
 *  order is class, connection, source, sink) will be used to memorize the given limitation. The audible
 *  volume of presently active connections which match all above selection parameters are adjusted accordingly.
 * 
 *  If className or connectionName is given, automatic volume balancing is always selected. Additionally,
 *  connections can be selected by specifying any source or sink which is part of the connection. However,
 *  @note If only a single source or sink element is specified and no className or connectionName is given,
 *        this will bypass the automatic balancing and try to manipulate the specified element directly.
 *        If this element does not support volume changes, the operation will fail.
 * 
 * #### Volume limit configuration parameters #
 *  - __volume__ (int16):          Absolute limit, given as absolute value in routing-side coordinates. 
 *                                 Limit type is internally set to LT_ABSOLUTE and the audible volume
 *                                 cannot be greater than this.
 *  - __volumeStep__ (int16):      Relative limit, given as absolute value in routing-side coordinates. 
 *                                 Limit type is internally set to LT_RELATIVE and the audible volume is set such
 *                                 that it differs from the volume of related (mixed) connections by given value.
 * 
 * Either volume or volumeStep is required if used as ACTION_LIMIT (limitState specified as LS_LIMITED),
 * otherwise both are ignored.
 * 
 * #### Limitation identifier parameter #
 *  - __pattern__ (uint32):        Optional identifier to distinguish between multiple independent limitations,
 *                                 understood as a bit-mask. Defaults to a pattern with all 1's (0xFFFFFFFF).
 * 
 * @note When pattern is used in an ACTION_UNLIMIT, all limitations are removed whose identifiers are fully
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
 * 
 * Set the limit for BASE class to -200 and pattern 1. 
 * @code
 *     <action type="ACTION_LIMIT" className="N:BASE" volume="-200" pattern="01"/>
 * @endcode
 * 
 * Remove the limit for BASE class with pattern 1.
 * @code
 *     <action type="ACTION_UNLIMIT" className="N:BASE" pattern="01"/>
 * @endcode
 * 
 *//***********************************************************************//**
 * @file CAmActionLimit.h
 *
 * This class implements ACTION_LIMIT and ACTION_UNLIMIT.
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Martin Koch   <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.\n
 *     This code is developed by Advanced Driver Information Technology.\n
 *     Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 *     All rights reserved.
 *
 * @copydoc actionSetLimit
 *
 *//**************************************************************************/

#ifndef GC_ACTIONLIMIT_H_
#define GC_ACTIONLIMIT_H_


#include "CAmActionSetVolumeCore.h"


namespace am {
namespace gc {

class CAmElement;
class CAmLimitableElement;


/**************************************************************************//**
 * @class am::gc::CAmActionLimit
 * @copydoc CAmActionLimit.h
 */

class CAmActionLimit : public CAmActionSetVolumeCore
{
public:
    CAmActionLimit();
    virtual ~CAmActionLimit();

protected:
    /** @name CAmActionContainer implementation *//**@{*/
    int _execute(void) override;
    int _undo(void) override;
    /**@}*/

private:
    void _applyLimit(void);
    void _prepareLowLevelRequest(const std::shared_ptr<CAmRoutePointElement> &pElement);

    struct undoItem_s
    {
        std::shared_ptr<CAmLimitableElement> pElement;
        gc_LimitState_e                      state;
        gc_LimitVolume_s                     limit;
    };
    std::list<undoItem_s>            mUndoList;

    CAmActionParam<gc_LimitState_e > mLimitStateParam;
    CAmActionParam<uint32_t >        mPatternParam;
    CAmActionParam<am_volume_t >     mVolumeParam;
    CAmActionParam<am_volume_t >     mVolumeStepParam;
};


} /* namespace gc */
} /* namespace am */
#endif /* GC_ACTIONLIMIT_H_ */
