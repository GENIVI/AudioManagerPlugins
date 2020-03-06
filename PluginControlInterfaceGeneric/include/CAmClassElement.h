/**************************************************************************//**
 * @file CAmClassElement.h
 *
 * This file contains the declaration of class Element class (member functions
 * and data members) used as data container to store the information related to
 * class as maintained by controller.
 *
 * @component AudioManager Generic Controller
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *//**************************************************************************/

#ifndef GC_CLASSELEMENT_H_
#define GC_CLASSELEMENT_H_

#include "CAmElement.h"
#include "CAmTypes.h"

namespace am {
namespace gc {
class CAmMainConnectionElement;


const std::vector<gc_SetSourceStateDirection_e> classTypeToConnectDirectionLUT =
{
    SD_MAINSINK_TO_MAINSOURCE,
    SD_MAINSINK_TO_MAINSOURCE,
    SD_MAINSOURCE_TO_MAINSINK
};

const std::vector<gc_SetSourceStateDirection_e> classTypeToDisconnectDirectionLUT =
{
    SD_MAINSOURCE_TO_MAINSINK,
    SD_MAINSOURCE_TO_MAINSINK,
    SD_MAINSINK_TO_MAINSOURCE,

};

class CAmConnectionListFilter;

/**************************************************************************//**
 * @class CAmClassElement
 * @copydoc CAmClassElement.h
 */

class CAmClassElement : public CAmLimitableElement
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
    CAmClassElement(const gc_Class_s &pclass, IAmControlReceive *pControlReceive);
    /**
     * @brief It is the destructor of class element class.
     * @param none
     * @return none
     */
    virtual ~CAmClassElement();

    /**
     * @brief It is API providing the interface to verify whether source and sink both belong to this class
     * @param sinkName: name of sink
     *        sourceName: name of source
     * @return true if source,sink pair belongs to class and vice versa
     */
    bool isSourceSinkPairBelongtoClass(const std::string &sinkName, const std::string &sourceName) const;

    // factory interface
    /**
     * @brief It is API providing the interface to create the new main connection using source and sink ID
     * @param sourceName: ID of source of connection
     *        sinkName: ID of sink of connection
     *        mainConnectionID: out parameter, main connection ID as returned by AM
     * @return E_NOT_POSSIBLE on internal error
     *         E_OK on success
     */
    am_Error_e createMainConnection(const std::string &sourceName, const std::string &sinkname,
        am_mainConnectionID_t &mainConnectionID);

    /**
     * @brief It is API providing the interface to destroy the main connection object
     * @param mainConnectionID: ID of main connection to be destroyed
     * @return none
     */
    void disposeConnection(const std::shared_ptr<CAmMainConnectionElement > pMainConnection);

    std::shared_ptr<CAmMainConnectionElement > getMainConnection(
        const std::string &sourceName, const std::string &sinkName,
        const std::vector<am_ConnectionState_e > &listConnectionStates,
        const gc_Order_e order = O_OLDEST);

    /**
     * @brief This function updates the connection Queue. Always the connected conenction should
     * be the last element in the Queue.
     *
     * @param none
     * @return none
     */
    void updateMainConnectionQueue(void);
    bool isElementBelongtoClass(const std::string &elementName, gc_Element_e elementType) const;
    am_Error_e restoreVolume(gc_LastClassVolume_s &lastClassVolume);
    std::string getVolumeString();
    std::string getMainConnectionString();
    std::string getLastMainConnectionsVolumeString();
    std::string getLastMainSoundPropertiesString();
    void restoreMainSoundProperties(gc_LastMainSoundProperties_s &lastMainSoundProperties);

    bool isVolumePersistencySupported() const;
    am_sourceID_t getSourceClassID(void) const;
    am_sinkID_t getSinkClassID(void) const;
    gc_Class_e getClassType() const;
    bool isPerSinkClassVolumeEnabled(void) const;
    am_Error_e getLastVolume(const std::string &sinkName, am_mainVolume_t &mainVolume);
    am_Error_e getLastVolume(std::shared_ptr<CAmMainConnectionElement > pMainConnection,
        am_mainVolume_t &mainVolume);
    void restoreLastMainConnectionsVolume(gc_LastMainConnectionsVolume_s &lastMainConnectionsVolume);
    am_Error_e setLastVolume(std::shared_ptr<CAmMainConnectionElement > pMainConnection,
        const std::string &sinkName = "", am_mainVolume_t mainVolume = 0);
    bool _checkElementInSubjectList(const std::string &elementName,
        const gc_Element_e type) const;

    std::shared_ptr<CAmElement > getElement();
    void setLastSoundProperty(const gc_ElementTypeName_s &elementInfo, const am_MainSoundProperty_s &mainSoundProperty);
    am_Error_e getLastSoundProperty(const gc_ElementTypeName_s &elementInfo,
        std::vector<am_MainSoundProperty_s> &listLastMainSoundProperty);

protected:
    am_Error_e _register(void);
    am_Error_e _unregister(void);
    am_Error_e _setLastClassVolume(const std::string &sinkName, am_mainVolume_t mainVolume);
    am_Error_e _setLastMainConnectionVolume(
        std::shared_ptr<CAmMainConnectionElement > pMainConnection);

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
    am_Error_e _getPreferredRoute(const std::string &mainSourceName, const std::string &mainSinkName,
        gc_Route_s &route) const;
    am_Error_e _getRouteFromTopology(const std::vector<gc_TopologyElement_s> &topology,
        const std::string &mainSourceName,
        const std::string &mainSinkName,
        gc_Route_s &route) const;
    am_Error_e _validateRouteFromTopology(std::vector<am_Route_s > &listRoutes,
        gc_Route_s &topologyRoute) const;
    am_Error_e _checkElementPresentInTopology(const std::vector<gc_TopologyElement_s> &topology,
        const std::string &elementName,
        const gc_ClassTopologyCodeID_e type = MC_SINK_ELEMENT) const;
    bool _updateListLastMainSoundProperty(const gc_ElementTypeName_s &elementInfo,
        const am_MainSoundProperty_s &mainSoundProperty);

    gc_Class_s                     mClass;
    am_sourceClass_t               mSourceClassID;
    am_sinkClass_t                 mSinkClassID;
    gc_LastClassVolume_s           mClassLastVolume;
    gc_LastMainConnectionsVolume_s mLastMainConnectionsVolume;
    gc_LastMainSoundProperties_s   mLastMainSoundProperties;
    gc_LastMainSoundProperties_s   mLastMainSoundPropertiesFromPersistence;

};

class CAmClassFactory : public CAmFactory<gc_Class_s, CAmClassElement >
{
public:
    using CAmFactory<gc_Class_s, CAmClassElement >::getElement;
    static std::shared_ptr<CAmClassElement > getElement(const std::string sourceName,
        const std::string sinkName);

    static std::shared_ptr<CAmClassElement > getElementBySourceClassID(
        const am_sourceClass_t sourceClassID);

    static std::shared_ptr<CAmClassElement > getElementBySinkClassID(
        const am_sinkClass_t sinkClassID);
    static void getElementsBySource(const std::string sourceName,
        std::vector<std::shared_ptr<CAmClassElement > > &listClasses);
    static void getElementsBySink(const std::string sinkName,
        std::vector<std::shared_ptr<CAmClassElement > > &listClasses);

    static std::shared_ptr<CAmClassElement > getElementByConnection(
        const std::string &connectionName);

};

} /* namespace gc */
} /* namespace am */
#endif /* GC_CLASSELEMENT_H_ */
