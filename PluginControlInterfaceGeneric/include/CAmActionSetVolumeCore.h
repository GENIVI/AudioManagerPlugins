/**************************************************************************//**
 * @page volumeActions Volume Actions
 *
 * This section covers the volume related actions. They are configured in the policy rules and comprise:
 *    - ACTION_SET_VOLUME: Set the audible volume (see page @subpage actionSetVolume),
 *    - ACTION_MUTE:       Reduce amplification to inaudibleness without affecting the nominal volume value
 *    - ACTION_UNMUTE:     Revert above ACTION_MUTE (see page @subpage actionSetMute),
 *    - ACTION_LIMIT:      Specify a maximum value the audible volume may not exceed without affecting
 *                         the nominal value and
 *    - ACTION_UNLIMIT:    Revert above ACTION_LIMIT (see page @subpage actionSetLimit).
 *
 * Typically, command-side (user-side) requests like 
 *    - [setVolume()](@ref am::IAmCommandReceive::setVolume), 
 *    - [volumeStep()](@ref am::IAmCommandReceive::volumeStep) and
 *    - [setSinkMuteState()](@ref am::IAmCommandReceive::setSinkMuteState)
 *
 * trigger a volume-related action. These requests generally specify a main sink as element
 * to operate on and give the target volume in user scale (@ref am::am_mainVolume_t).
 *
 * Through policy rules, such requests can be modified to operate on any class, connection,
 * source or sink element instead. Additionally, policy rules can specify these actions to handle
 * other trigger events as well, e. g. ramping during a connect / disconnect operation
 * or as a redirected system or sound property change.
 * Page @subpage volHandling covers an example of the activities composing an overlaid navigation announcement.
 *
 * A sophisticated calculation algorithm is implemented for all three aspects of the volume handling,
 * allowing to either
 *  - automatically balance partial volume settings over a family of one or more interrelated connections or
 *  - manipulating a single dedicated element directly.
 * 
 * For implementation concepts see page @subpage volManBasics.
 */
/**************************************************************************//**
 * @page volManBasics Volume Management Fundamentals
 *
 * To achieve a solid basis for the target volume calculation in all three actions SETVOLUME, MUTE
 * and LIMIT, their base class hosts 2 tables.
 * 
 * First is the connection table with the total volume information summed over all sources and sinks
 * of all route elements, separately handling nominal value (volume), limit value (offset) and mute state.
 * 
 * |                  |            |                actual                 |||                target                              ||||
 * | :--------------- | :--------: | :--------: | :---------: | :----------: | :--------: | :---------: | :----------: | :---------: |
 * |                  | adjustable | muteState  |   volume    |    offset    |  muteState |   volume    |   offset     | relative    |
 * | MainConnection-1 |    yes     | mc1_act-ms | mc1_act-vol | mc1_act-offs | mc1_tgt-ms | mc1_tgt-vol | mc1_tgt-offs | mc1_tgt-rel |
 * | MainConnection-2 |    no      | mc2_act-ms | mc2_act-vol | mc2_act-offs | mc2_tgt-ms | mc2_tgt-vol | mc2_tgt-offs | mc2_tgt-rel |
 *
 * This table is populated in a first step from all connections matching the filter defined by parameters
 * className, sourceName, sinkName and/or connectionName. These connections are flagged as adjustable
 * because we intend to modify their audible volume. In a second step all connections are added, which are
 * linked through any shared sources and/or sinks. These shall be flagged as not adjustable, as we intend
 * to keep their audible volume unaltered.
 *
 * All top-level actions CAmActionMute, CAmActionLimit and CAmActionSetVolume shall apply their algorithms
 * on this table, balancing out volume, offset and muteState of all involved connections. In case a relative
 * offset is found in the applicable limits it shall be converted to absolute offset values in this step,
 * possibly toggling the flag from not adjustable to adjustable.
 *
 * In the end we have all constraints for the second table, which allows to calculate the 2½ dimensions
 * of the volume components for all involved source and sink elements.
 *
 * |        |           |   source 1   |    sink 1    |   source 2   |    sink 3    |    sink 4    |    MC 1     |    MC 2     |
 * | :----- | :-------- | :----------: | :----------: | :----------: | :----------: | :----------: | :---------: | :---------: |
 * | ordinal           ||       3      |      2       |      2       |      1       |      0       |     ^       |     ^       |
 * | actual | muteState | src1_act-ms  | snk1_act-ms  | src2_act-ms  | snk3_act-ms  | snk4_act-ms  |     ^       |     ^       |
 * | ^      |  volume   | src1_act-vol | snk1_act-vol | src2_act-vol | snk3_act-vol | snk4_act-vol |     ^       |     ^       |
 * | ^      |  offset   | src1_act-offs| snk1_act-offs| src2_act-offs| snk3_act-offs| snk4_act-offs|     ^       |     ^       |
 * | target | muteState | src1_tgt-ms  | snk1_tgt-ms  | src2_tgt-ms  | snk3_tgt-ms  | snk4_tgt-ms  | mc1_tgt-ms  | mc2_tgt-ms  |
 * | ^      |  volume   | src1_tgt-vol | snk1_tgt-vol | src2_tgt-vol | snk3_tgt-vol | snk4_tgt-vol | mc1_tgt-vol | mc2_tgt-vol |
 * | ^      |  offset   | src1_tgt-offs| snk1_tgt-offs| src2_tgt-offs| snk3_tgt-offs| snk4_tgt-offs| mc1_tgt-offs| mc2_tgt-offs|
 * | MainConnection-1  ||      X       |      X       |              |       X      |      X       |                          ||
 * | MainConnection-2  ||              |              |       X      |       X      |      X       |                          ||
 *
 * #### Automatic Volume Balancing #
 * Automatically balancing the volume components of involved elements is done in the common base class
 * and used always if any of the following conditions match:
 *  - A __class__ or __connection__ element is specified as operation target element __or__
 *  - In case of ACTION_SET_VOLUME,
 *     + The target volume (or volumeStep) is given in user scale or
 *     + The target volume specification is omitted. In this case, the most recent active
 *       volume will be restored.
 * 
 * #### Direct Volume Manipulation #
 * On the other hand, in special cases the automatic balancing is bypassed and a single column of
 * above second table is modified directly. This applies if all of the following conditions apply:
 *  - A __source__ or __sink__ is specified as operation target element __and__
 *  - In case of ACTION_SET_VOLUME,
 *     + The target volume is specified in internal scale or
 *     + The table of involved active connections is empty.
 * 
 * Finally, the calculated composed volume is forwarded to the involved routing side elements.
 */
/**************************************************************************//**
 * @file CAmActionSetVolumeCore.h
 *
 * @brief Common base class for CAmActionSetVolume, CAmActionLimit and CAmActionMute
 *
 * @component{AudioManager Generic Controller}
 *
 * @author Martin Koch   <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2019 Advanced Driver Information Technology.\n
 *     This code is developed by Advanced Driver Information Technology.\n
 *     Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 *     All rights reserved.
 *
 * The basic mechanism is described on page @ref volManBasics.
 */

#ifndef GC_ACTIONSETVOLUMECORE_H_
#define GC_ACTIONSETVOLUMECORE_H_


#include "CAmActionContainer.h"
#include "CAmActionCommand.h"


namespace am {
namespace gc {


class CAmRoutePointElement;
class CAmMainConnectionElement;

/***********************************************************************//**
 *@class CAmActionSetVolumeCore
 *@copydoc CAmActionSetVolumeCore.h
 */

class CAmActionSetVolumeCore : public CAmActionContainer
{
public:
    virtual ~CAmActionSetVolumeCore();

    // 2½-dimensional container to handle volume components together
    struct VolumeInfo_t
    {
        am_MuteState_e muteState;
        am_volume_t    volume;
        am_volume_t    offset;

        VolumeInfo_t(): muteState(MS_UNKNOWN), volume(AM_VOLUME_NO_LIMIT), offset(AM_VOLUME_NO_LIMIT) {}
        inline bool operator==(const VolumeInfo_t &rhs) const;
        inline bool operator!=(const VolumeInfo_t &rhs) const;
    };

    // container to handle actual / target pairs
    struct ChangeInfo_t
    {
        VolumeInfo_t actual;
        VolumeInfo_t target;
    };

    struct ConnectionInfo_t : ChangeInfo_t
    {
        am_volume_t          relative;   // add limit specifications for relative offset
        bool                 adjustable; // true if connection is directly addressed (selected) and
                                         // thus allowed to change its audible volume, false otherwise
                                         // (e. g. if linked through a shared source or sink)
    };
    typedef std::map<std::shared_ptr<CAmMainConnectionElement >, ConnectionInfo_t > ConnectionMap_t;
    typedef std::pair<const std::shared_ptr<CAmMainConnectionElement>, ConnectionInfo_t> ConnectionMapItem_t;

    // helper structure to track sharing of sinks and/or sources between multiple connections
    struct MatrixColumn_t : ChangeInfo_t
    {
        unsigned                                              ordinal; // increases with distance from main sink
        std::set<std::shared_ptr<CAmMainConnectionElement > > connections;

        MatrixColumn_t() : ordinal(0)  {}
    };
    typedef std::map<std::shared_ptr<CAmRoutePointElement>, MatrixColumn_t > Matrix_t;

protected:
    CAmActionSetVolumeCore(const std::string &name);

    /**@name First Step - balance connection volumes *//**@{*/
    void _setupConnectionMap(void);
    void _getConnectionVolumes(const std::shared_ptr<CAmMainConnectionElement > &pConnection
            , ConnectionInfo_t &settings, bool adjustable);
    void _resolveRelativeLimits(void);
    /**@}*/

    /**@name Second Step - balance element volumes *//**@{*/
    bool _isLowLevelRequest(std::shared_ptr<CAmRoutePointElement> &pElement);
    void _calculateElementVolumes(gc_SetSourceStateDirection_e direction);
    void _setupVolumeMatrix(const std::vector<am_ConnectionState_e > &listConnectionStates);
    bool _qualifiesForChanges(const std::shared_ptr<CAmRoutePointElement > &pElement);
    void _getElementVolumes(const std::shared_ptr<CAmRoutePointElement > &pElement, ChangeInfo_t &changes);
    VolumeInfo_t _getVolumeDiff(const std::shared_ptr<CAmMainConnectionElement > &pConnection);
   /**@}*/

    /// sub-action for sink or source element, interacting with the routing side
    class CAmActionSetElementVolume;
    void _createActionSetElementVolume(const std::shared_ptr<CAmRoutePointElement> &pElement, const ChangeInfo_t &settings);

    // connection table with total volume information summed over all sources and sinks
    ConnectionMap_t                      mConnectionMap;

    // element table with calculated 2½-dimensional volume components for sources and sinks
    Matrix_t                             mMatrix;

    // selection parameters
    CAmActionParam<std::string >         mClassNameParam;
    CAmActionParam<std::string >         mSourceNameParam;
    CAmActionParam<std::string >         mSinkNameParam;
    CAmActionParam<std::string >         mConnectionNameParam;

    // execution parameters
    CAmActionParam<am_time_t >           mRampTimeParam;
    CAmActionParam<am_CustomRampType_t > mRampTypeParam;
};

// comparison operators
inline bool CAmActionSetVolumeCore::VolumeInfo_t::operator==(const VolumeInfo_t &rhs) const
{
    return (muteState == rhs.muteState) && (volume == rhs.volume) && (offset == rhs.offset);
}
inline bool CAmActionSetVolumeCore::VolumeInfo_t::operator!=(const VolumeInfo_t &rhs) const
{
    return !(*this == rhs);
}

// logging operators
inline std::ostream& operator<<(std::ostream &out, const CAmActionSetVolumeCore::VolumeInfo_t &rhs)
{
    return out << "{muteState: " << rhs.muteState << ", volume: " << rhs.volume
            << ", offset: " << rhs.offset << "}";
}

inline std::ostream& operator<<(std::ostream &out, const CAmActionSetVolumeCore::ChangeInfo_t &rhs)
{
    return out << "{actual: " << rhs.actual << ", target: " << rhs.target << "}";
}

inline std::ostream& operator<<(std::ostream &out, const CAmActionSetVolumeCore::ConnectionInfo_t &rhs)
{
    return out << "{actual: " << rhs.actual << ", target: " << rhs.target << ", relative: " << rhs.relative
            << ", adjustable: " << (rhs.adjustable ? "true" : "false") << "}";
}

/**************************************************************************//**
 * @class CAmActionSetVolumeCore::CAmActionSetElementVolume
 * 
 * Sub-action of @ref CAmActionSetVolumeCore for sink or source elements,
 * directly interacting with the routing side
 */

class CAmActionSetVolumeCore::CAmActionSetElementVolume  : public CAmActionCommand
{
public:
    CAmActionSetElementVolume(const std::shared_ptr<CAmRoutePointElement> &pElement, const VolumeInfo_t &settings
            , am_time_t rampTime, am_CustomRampType_t rampType);
    virtual ~CAmActionSetElementVolume();

    /** @name IAmAction implementation *//**@{*/
    int _execute(void) override;
    int _update(int result) override;
    int _undo(void) override;
    /**@}*/

private:
    int _setRoutingSideVolume(const VolumeInfo_t &request);

    std::shared_ptr<CAmRoutePointElement> mpElement;
    am_Handle_s                           mHandle;

    // requested target volume
    VolumeInfo_t                          mRequested;

    // recent volume
    VolumeInfo_t                          mRecent;

    // ramp parameters
    am_time_t                             mRampTime;
    am_CustomRampType_t                   mRampType;
};


} /* namespace gc */
} /* namespace am */

#endif /* GC_ACTIONSETVOLUMECORE_H_ */
