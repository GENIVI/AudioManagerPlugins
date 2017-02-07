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

class CAmConnectionListFilter;
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
     * @return true if source,sink pair belongs to class and vice versa
     */
    bool isSourceSinkPairBelongtoClass(const std::string& sinkName, const std::string& sourceName);
    // factory interface
    /**
     * @brief It is API providing the interface to create the new main connection using source and sink ID
     * @param sourceName: ID of source of connection
     *        sinkName: ID of sink of connection
     *        mainConnectionID: out parameter, main connection ID as returned by AM
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
                    const gc_Order_e order = O_OLDEST);
    /**
     * @brief It is API providing the interface to get the main connection list using either source or sink
     * 		  or both name. User to provide NULL for not required parameter
     * @param   fobject : the fuction object for filtering the list connections
     *          listMainConnections: list in which main connections will be returned
     *
     * @return none
     */
    void getListMainConnections(std::vector<CAmMainConnectionElement* >& listMainConnections,
                                const CAmConnectionListFilter& fobject);
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
    am_Error_e _validateRouteFromTopology(std::vector<am_Route_s >& listRoutes,
                                          gc_Route_s& topologyRoute);

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
    using CAmFactory<gc_Class_s, CAmClassElement >::getElement;
    static CAmClassElement* getElement(const std::string sourceName, const std::string sinkName);
    static void getElementsBySource(const std::string sourceName,
                                    std::vector<CAmClassElement* >& listClasses);
    static void getElementsBySink(const std::string sinkName,
                                  std::vector<CAmClassElement* >& listClasses);
    static CAmClassElement* getElementByConnection(const std::string& connectionName);

};

class CAmConnectionListFilter
{
public:
    CAmConnectionListFilter();
    virtual ~CAmConnectionListFilter()
    {
    }
    void setSourceName(std::string sourceName);
    void setSinkName(std::string sinkName);
    void setListConnectionStates(std::vector<am_ConnectionState_e >& listConnectionStates);
    void setListExceptSourceNames(std::vector<std::string >& listExceptSources);
    void setListExceptSinkNames(std::vector<std::string >& listExceptSinks);
    std::vector<CAmMainConnectionElement* >& getListMainConnection();
    void operator()(CAmMainConnectionElement* pMainConnection);
private:
    bool _stringMatchFilter(std::string filterName, std::string inputName);
    bool _connetionStateFilter(am_ConnectionState_e connectionState);
    bool _exceptionNamesFilter(std::vector<std::string >& listString, std::string input);

    std::string mSourceName;
    std::string mSinkName;
    std::vector<am_ConnectionState_e > mListConnectionStates;
    std::vector<std::string > mListExceptSources;
    std::vector<std::string > mListExceptSinks;
    std::vector<CAmMainConnectionElement* > mListMainConnections;
};

} /* namespace gc */
} /* namespace am */
#endif /* GC_CLASSELEMENT_H_ */
