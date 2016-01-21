/******************************************************************************
 * @file: CAmMainConnectionElement.h
 *
 * This file contains the declaration of main connection class (member functions
 * and data members) used as data container to store the information related to
 * main connection as maintained by controller.
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

#ifndef GC_MAINCONNECTIONELEMENT_H_
#define GC_MAINCONNECTIONELEMENT_H_

#include "CAmRouteElement.h"
#include "CAmTypes.h"

namespace am {
namespace gc {
class CAmControlReceive;
class CAmSinkElement;
class CAmSourceElement;
class CAmMainConnectionElement : public CAmElement
{
public:
    /**
     * @brief It is the constructor of main connection class. Initialize the member
     * variables with default value.It will be invoked whenever new connection request is received by framework
     * @param pMainSource: pointer to main source of the connection
     *        pMainSink: pointer to main sink of the connection
     *        pControlReceive: pointer to control receive class instance
     * @return none
     */
    CAmMainConnectionElement(const gc_Route_s& route, CAmControlReceive* pControlReceive);
    /**
     * @brief It is the destructor of main connection class.
     * @param none
     * @return none
     */
    virtual ~CAmMainConnectionElement();
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
    CAmSinkElement* getMainSink(void) const;
    /**
     * @brief It is API providing the interface to get the pointer of main source of the connection
     * @param none
     * @return pointer of main source
     */
    CAmSourceElement* getMainSource(void) const;
    /**
     * @brief It is API providing the interface to get the connection state of the connection
     * @param none
     * @return state of connection
     */
    am_Error_e getState(int& state) const;
    /**
     * @brief It is API providing the interface to set the connection state of the connection
     * @param connectionState: connection state to be set
     * @return none
     */
    am_Error_e setState(const int connectionState);
    /**
     * @brief It is API providing the interface to get the priority of the connection
     * @param none
     * @return priority of connection
     */
    am_Error_e getPriority(int32_t& priority) const;
    /**
     * @brief It is API providing the interface to get the volume of connection. It is the sum of
     * all the elements decibel volume value of this connection
     * @param none
     * @return connection volume in decibel
     */
    am_Error_e getVolume(am_volume_t& volume) const;
    /**
     * @brief It is API providing the interface to get the element of connection whose volume is limited
     * @param none
     * @return pointer to source element
     */
    am_Error_e getLimitElement(
                    CAmElement* pChangedVolumeElelment, am_volume_t newVolume,
                    std::vector<std::pair<CAmElement*, gc_LimitVolume_s > >& listRouteElements);
    /**
     * @brief It is API providing the interface to get the list of route elements (source/sink)
     * belonging to this connection
     * @param listRouteElements: list of route elements
     * @return none
     */
    void getListRouteElements(std::vector<CAmRouteElement* >& listRouteElements) const;
    /**
     * @brief This API calculates the connection state from the actual route elements
     *
     * @param none
     * @return am_ConnectionState_e The connection state.
     */
    void updateState();
    am_MuteState_e getMuteState() const;

protected:
    am_Error_e _register(void);
    am_Error_e _unregister(void);

private:
    // list of object of route element class instance storing information about particular route
    am_volume_t _getNewOffsetVolume(CAmElement* pElement, am_volume_t newVolume);
    std::vector<CAmRouteElement* > mListRouteElements;
    CAmControlReceive* mpControlReceive;
    gc_Route_s mRoute;
};

class CAmMainConnectionFactory : public CAmFactory<gc_Route_s, CAmMainConnectionElement >
{
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_MAINCONNECTIONELEMENT_H_ */
