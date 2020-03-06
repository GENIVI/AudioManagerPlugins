/**************************************************************************//**
 * @file  CAmMainConnectionElement.h
 *
 * This file contains the declaration of main connection class (member functions
 * and data members) used as data container to store the information related to
 * main connection as maintained by controller.
 *
 * @component  AudioManager Generic Controller
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2019 Advanced Driver Information Technology.
 * This code is developed by Advanced Driver Information Technology.
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.
 * All rights reserved.
 *
 */

#ifndef GC_MAINCONNECTIONELEMENT_H_
#define GC_MAINCONNECTIONELEMENT_H_

#include "CAmRouteElement.h"
#include "CAmTypes.h"

namespace am {
namespace gc {

class CAmSinkElement;
class CAmSourceElement;
class CAmClassElement;
class IAmActionCommand;

/***********************************************************************//**
 * @class am::gc::CAmMainConnectionElement
 * @copydoc  CAmMainConnectionElement.h
 */

class CAmMainConnectionElement : public CAmElement
{
public:
    /**
     * @brief It is the constructor of main connection class. Initialize the member
     * variables with default value.It will be invoked whenever new connection request is received by framework
     * @param gc_Route_s: reference to route configuration of the connection
     *        pControlReceive: pointer to control receive class instance
     * @return none
     */
    CAmMainConnectionElement(const gc_Route_s &route, IAmControlReceive *pControlReceive);
    /**
     * @brief It is the destructor of main connection class.
     * @param none
     * @return none
     */
    virtual ~CAmMainConnectionElement();
    /**
     * @brief Get the class element of the main source of the connection
     * @return class element
     */
    std::shared_ptr<CAmClassElement> getClassElement(void) const;

    /**
     * @brief Get the name of the class of the main source of the connection
     * @param none
     * @return class name
     */
    std::string getClassName(void) const;

    /**
     * @brief It is API providing the interface to get the name of main source of the connection
     * @param none
     * @return main source name
     */
    std::string getMainSourceName(void) const;

    /**
     * @brief It is API providing the interface to get the name of main sink of the connection
     * @param none
     * @return main sink name
     */
    std::string getMainSinkName(void) const;

    /**
     * @brief It is API providing the interface to get the pointer of main sink of the connection
     * @param none
     * @return pointer of main sink
     */
    std::shared_ptr<CAmSinkElement > getMainSink(void) const;

    /**
     * @brief It is API providing the interface to get the pointer of main source of the connection
     * @param none
     * @return pointer of main source
     */
    std::shared_ptr<CAmSourceElement > getMainSource(void) const;

    /**
     * @brief Keep track of incomplete Connect- / Disconnect-actions
     * @param pointer to action
     */
    void registerTransitionAction(const IAmActionCommand *pTransitionAction);
    void unregisterTransitionAction(const IAmActionCommand *pTransitionAction);

    /**
     * @brief Register for connection state change notifications
     * @param status (result) of current action
     */
    void setStateChangeTrigger(am_Error_e actionStatus);

    /**
     * @brief Indicate whether this connection is no longer used and may be removed from the data base
     */
    bool permitsDispose();

    /**
     * @brief It is API providing the interface to get the connection state of the connection
     * @param none
     * @return state of connection
     */
    am_ConnectionState_e getState() const;

    /**
     * @brief It is API providing the interface to set the connection state of the connection
     * @param connectionState: connection state to be set
     * @return none
     */
    void setState(am_ConnectionState_e connectionState);

    /**
     * @brief It is API providing the interface to get the priority of the connection
     * @param none
     * @return calculated priority of connection
     */
    int32_t getPriority(void) const override;

    /**
     * @brief Shorthand to check if all route elements are available with both source and sink
     * @return A_AVAILABLE   if all route points are A_AVAILABLE,
     *         A_UNKNOWN     if any of the route points is invalid or A_UNKNOWN
     *         A_UNAVAILABLE otherwise
     */
    am_Availability_e getRouteAvailability() const;

    /**
     * @brief It is API providing the interface to get the list of route elements (source/sink)
     * belonging to this connection
     * @param listRouteElements: list of route elements
     * @return none
     */
    void getListRouteElements(
        std::vector<std::shared_ptr<CAmRouteElement > > &listRouteElements) const;

    /**
     * @brief This API calculates the connection state from the actual route elements
     *
     * @param none
     * @return am_ConnectionState_e The connection state.
     */
    void updateState();

    /*
     * Volume Managements related methods
     */
    am_volume_t     getVolume() const;
    am_volume_t     getOffsetVolume() const;
    am_MuteState_e  getMuteState() const;
    void            setMainVolume(const am_mainVolume_t volume);
    am_mainVolume_t getMainVolume() const;

    void getLimits(std::list<gc_LimitVolume_s > &limits) const;
    bool isSinkBelongingToMainConnection(std::shared_ptr<CAmSinkElement > pSink);
    bool isSourceBelongingToMainConnection(std::shared_ptr<CAmSourceElement > pSource);
    void getVolumeChangeElements(std::list<std::shared_ptr<CAmRoutePointElement > > &listVolumeElements
            , gc_SetSourceStateDirection_e direction) const;

    virtual int update(std::shared_ptr<CAmElement > pNotifierElement,
        const am_mainVolume_t &mainVolume);
    am_Error_e storeVolumetoPersistency();
    am_Error_e updateMainVolume(void);

    std::shared_ptr<CAmElement > getElement();
    void removeRouteElements();

protected:
    am_Error_e _register(void);
    am_Error_e _unregister(void);
    am_Error_e _setLastVolume();

private:
    std::vector<std::shared_ptr<CAmRouteElement > > mListRouteElements;
    gc_Route_s                                      mRoute;
    std::shared_ptr<CAmClassElement >               mpClassElement;
    std::vector<const IAmActionCommand *>           mOngoingTransitions;
    am_mainVolume_t                                 mMainVolume;
};

class CAmConnectionListFilter
{
public:
    CAmConnectionListFilter();
    virtual ~CAmConnectionListFilter()
    {
    }

    // define filter criteria
    void setClassName(const std::string &className);
    void setSourceName(const std::string &sourceName);
    void setSinkName(const std::string &sinkName);
    void setListConnectionStates(const std::vector<am_ConnectionState_e > &listConnectionStates);
    void setListExceptSourceNames(const std::vector<std::string > &listExceptSources);
    void setListExceptSinkNames(const std::vector<std::string > &listExceptSinks);
    void setListExceptClassNames(const std::vector<std::string > &listExceptClasses);

    // compare object against criteria
    bool matches(const std::shared_ptr<CAmMainConnectionElement > &pCandidate) const;

    static bool matches(const std::string &filterName, const std::string &inputName);
    static bool matchesAny(const std::vector<std::string > &listString, const std::string &inputName);
    static bool matchesAny(const std::vector<am_ConnectionState_e > &filterStates, am_ConnectionState_e inputState);

private:
    std::string                        mClassName;
    std::string                        mSourceName;
    std::string                        mSinkName;
    std::vector<am_ConnectionState_e > mListConnectionStates;
    std::vector<std::string >          mListExceptSources;
    std::vector<std::string >          mListExceptSinks;
    std::vector<std::string >          mListExceptClasses;
};

class CAmMainConnectionFactory : public CAmFactory<gc_Route_s, CAmMainConnectionElement >
{
    friend class CAmMainConnectionElement;

private:
    // maintain a chronological list of all CAmMainConnectionElement instances.
    // It is maintained by constructor and destructor of the instances
    // and therefore cannot be based on shared pointers. Take care when using it.
    static std::list<CAmMainConnectionElement* > mOrderedList;

public:
    using CAmFactory::getListElements;

    static void getListElements(std::vector<std::shared_ptr<CAmMainConnectionElement>> &connectionList
            , const CAmConnectionListFilter &filter,  const gc_Order_e order = O_OLDEST);

    // helper function to ensure correct output of lists sorted by O_OLDEST or O_NEWEST
    // (will move given element pointer to end of chronological list)
    // New position can be specified as either of O_OLDEST or O_NEWEST.
    static void moveToEnd(const std::string &mainConnectionName, gc_Order_e position);
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_MAINCONNECTIONELEMENT_H_ */
