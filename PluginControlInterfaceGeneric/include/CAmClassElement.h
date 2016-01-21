/******************************************************************************
 * @file: CAmClassElement.h
 *
 * This file contains the declaration of class Element class (member functions
 * and data members) used as data container to store the information related to
 * class as maintained by controller.
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

#ifndef GC_CLASSELEMENT_H_
#define GC_CLASSELEMENT_H_

#include "CAmElement.h"
#include "CAmTypes.h"

namespace am {
namespace gc {
class CAmMainConnectionElement;
class CAmControlReceive;
class CAmClassElement : public CAmElement
{

public:
    /**
     * @brief It is the constructor of class controller class. Initialize the member
     * variables with default value. It is invoked whenever new class is created in controller.
     * @param name: name of the class element
     *        type: type of the class element
     *        pControlReceive: pointer to control receive class instance
     * @return none
     */
    CAmClassElement(const gc_Class_s& pclass, CAmControlReceive* pControlReceive);
    /**
     * @brief It is the destructor of class element class.
     * @param none
     * @return none
     */
    virtual ~CAmClassElement();
    bool isSourceBelongtoClass(const std::string& sourceName);
    bool isSinkBelongtoClass(const std::string& sinkName);
    /**
     * @brief It is API providing the interface to verify whether source and sink both belong to this class
     * @param sinkName: name of sink
     *        sourceName: name of source
     * @return true if yes
     *         false if no
     */
    bool isSourceSinkPairBelongtoClass(const std::string& sinkName, const std::string& sourceName);
    // factory interface
    /**
     * @brief It is API providing the interface to create the new main connection using source and sink ID
     * @param sourceID: ID of source of connection
     *        sinkID: ID of sink of connection
     *        mainConnectionID: main connection ID as returned by AM
     * @return E_NOT_POSSIBLE on internal error
     *         E_OK on success
     */
    am_Error_e createMainConnection(const std::string& sourceName, const std::string& sinkname,
                                    am_mainConnectionID_t& mainConnectionID);
    /**
     * @brief It is API providing the interface to destroy the main connection object
     * @param mainConnectionID: ID of main connection to be destroyed
     * @return none
     */
    void disposeConnection(const am_mainConnectionID_t mainConnectionID);

    CAmMainConnectionElement* getMainConnection(
                    const std::string& sourceName, const std::string& sinkName,
                    std::vector<am_ConnectionState_e >& listConnectionStates,
                    const gc_Order_e order);
    /**
     * @brief It is API providing the interface to get the main connection list using either source or sink
     * 		  or both name. User to provide NULL for not required parameter
     * @param listMainConnections: list in which main connections will be returned
     * 		  sourceName: source name of connection
     *        sinkName: sink name of connection
     *
     * @return none
     */
    void getListMainConnections(const std::string& sourceName, const std::string& sinkName,
                                std::vector<am_ConnectionState_e >& listConnectionStates,
                                std::vector<CAmMainConnectionElement* >& listMainConnections);
    /**
     * @brief It is API providing the interface to get the main connection instance of given connection state with given order.
     * In case order not provided by default oldest connection is returned
     * @param connectionState: state of connection
     * 		  order: connection order
     * @return pointer to main connection
     */
    CAmMainConnectionElement* getMainConnection(const am_ConnectionState_e connectionState,
                                                const gc_Order_e order = O_OLDEST);
    /**
     * @brief It is API providing the interface to get the list of all the connection belonging to this class
     * @param listConnections: variable in which list will be returned
     * @return none
     */
    void getListMainConnections(std::vector<CAmMainConnectionElement* >& listConnections);

    /**
     * @brief It is API providing the interface to store the volume limit and pattern information,
     * in the list maintained by the class, by which it is limited. It is used to verify whether
     * volume change is needed or not
     * @param volume: volume by which it should be limited
     *        pattern: pattern of limit
     * @return E_NOT_POSSIBLE: if volume need not to be limited
     *         E_OK: if volume need to be limited
     */
    am_Error_e setLimitState(const gc_LimitState_e type, const gc_LimitVolume_s &limitVolume,
                             const uint32_t pattern = DEFAULT_LIMIT_PATTERN);

    /**
     * @brief It is API providing the interface to verify whether this class is limited or not.
     * It is invoked during connect and disconnect action
     * @param none
     * @return true if class is limited
     *         false if class is not limited
     */
    gc_LimitState_e getLimitState(void);
    /**
     * @brief It is API providing the interface to get the volume by which this class is currently limited
     * @param none
     * @return limit volume
     */
    am_Error_e getClassLimitVolume(const am_volume_t volume, gc_LimitVolume_s& limitVolume);
    /**
     * @brief This function updates the connection Queue. Always the connected conenction should
     * be the last element in the Queue.
     *
     * @param none
     * @return none
     */
    void updateMainConnectionQueue(void);
    void pushMainConnectionInQueue(CAmMainConnectionElement* pConnection);

    am_MuteState_e getMuteState() const;
protected:
    am_Error_e _register(void);
    am_Error_e _unregister(void);
private:
    /**
     * @brief It is internal function used to find the route of the requested connection
     * @param mainSourceName: Name of main source of connection
     *        mainSinkName: Name of main sink of connection
     *        listRoutes: list of route elements ID involved in connection
     *        listVolumeElements: list of element pointer involved in connection
     * @return E_UNKNOWN if route not found
     *         E_OK on success
     */
    am_Error_e _getRoute(const std::string& mainSourceName, const std::string& mainSinkName,
                         gc_Route_s& route);
    am_Error_e _validateRouteFromTopology(std::vector<am_Route_s>& listRoutes, gc_Route_s& topologyRoute);
    // list of main connections of this class
    std::vector<CAmMainConnectionElement* > mListMainConnections;
    // map to store source element pointer belonging to this class
    std::vector<std::string > mListOwnedSourceElements;
    // map to store sink element pointer belonging to this class
    std::vector<std::string > mListOwnedSinkElements;
    // map to store gateway element pointer belonging to this class
    std::vector<std::string > mListOwnedGatewayElements;
    // list to store the limit volume information
    std::map<uint32_t, gc_LimitVolume_s > mMapLimitVolumes;

    CAmControlReceive* mpControlReceive;
    gc_Class_s mClass;
};

class CAmClassFactory : public CAmFactory<gc_Class_s, CAmClassElement >
{
public:
    static CAmClassElement* getClassElement(const std::string sourceName,
                                            const std::string sinkName);
    static void getElementsBySource(const std::string sourceName,
                                    std::vector<CAmClassElement* >& listClasses);
    static void getElementsBySink(const std::string sinkName,
                                  std::vector<CAmClassElement* >& listClasses);
    static CAmClassElement* getClassElement(const std::string& connectionName);

};
} /* namespace gc */
} /* namespace am */
#endif /* GC_CLASSELEMENT_H_ */
