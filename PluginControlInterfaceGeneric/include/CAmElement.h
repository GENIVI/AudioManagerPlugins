/**************************************************************************//**
 * @file CAmElement.h
 *
 * This file contains the declaration of element class (member functions and
 * data members) used as base class for source, sink & gateway element classes
 *
 * @component AudioManager Generic Controller
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
 */

#ifndef GC_ELEMENT_H_
#define GC_ELEMENT_H_

#include "CAmTypes.h"
#include "IAmControlCommon.h"
#include "CAmLogger.h"
#include <algorithm>
#include <vector>
#include <set>
using namespace std;
namespace am {
namespace gc {

#define DEFAULT_ELEMENT_PRIORITY (100)

class CAmElement;

static am_Error_e registerElement(std::shared_ptr<CAmElement > pElement);
static am_Error_e unregisterElement(std::shared_ptr<CAmElement > pElement);

#define SHARED_COUNT 1

/***********************************************************************//**
 * @class am::gc::CAmElement
 * @copydoc  CAmElement.h
 */

class CAmElement
{

#ifndef UNIT_TEST
protected:
#else
// we need this for the unit test
public:
#endif

    /**
     * @brief It is the constructor of element class. Initialize the member
     * variables with default value. It will be invoked during creation of any
     * element (gateway/source/sink).
     * @param type: type of the derived class
     *        name: name of the element
     *        pControlReceive: pointer to IAmControlReceive interface provided by AM daemon
     */
    CAmElement(gc_Element_e type, const std::string &name, IAmControlReceive *pControlReceive);

public:
    /**
     * @brief It is the destructor of element class.
     * @param none
     */
    virtual ~CAmElement();

    IAmControlReceive *getControlReceive(void);

    /**
     * @brief This API is used to get the type of element
     * @return type of element
     */
    gc_Element_e getType(void) const;

    /**
     * @brief This API is used to get the name of element
     * @param none
     * @return element name
     */
    std::string getName(void) const;

    /**
     * @brief This API is used to set the ID of element
     * @param ID: element ID as in DB
     * @return none
     */
    void setID(const uint16_t ID);

    /**
     * @brief This API is used to get the ID of element
     * @param none
     * @return element ID as in DB
     */
    uint16_t getID(void) const;

    /**
     * @brief This API is used to get the priority of element as defined in configuration file.
     * @return priority
     */
    virtual int32_t getPriority(void) const;

    virtual int update(std::shared_ptr<CAmElement > pNotifierElement,
        const am_mainVolume_t &mainVolume);

    am_Error_e attach(std::shared_ptr<CAmElement > pSubject);
    am_Error_e detach(std::shared_ptr<CAmElement > pSubject);
    int getObserverCount(void) const;

    int getObserverCount(gc_Element_e, std::vector<std::shared_ptr<CAmElement > > * = NULL) const;
    int getSubjectCount(void) const;

    template <typename TupdateParam>
    void notify(TupdateParam updateData)
    {
        std::vector<std::shared_ptr<CAmElement > >::iterator itListObservers = mListObservers.begin();
        for (; itListObservers != mListObservers.end(); ++itListObservers)
        {
            LOG_FN_DEBUG(__FILENAME__, __func__, "notify update to element :",
                (*itListObservers)->getName());
            (*itListObservers)->update(this->getElement(), updateData);
        }
    }

    /*Searching of elements*/
    template <typename TelemntFilterType>
    void getListElements(TelemntFilterType elementFilter,
        std::vector<std::shared_ptr<CAmElement > > &listOfSubjects) const
    {
        std::vector<std::shared_ptr<CAmElement > >::const_iterator itListSubjects = mListSubjects.begin();
        for (; itListSubjects != mListSubjects.end(); ++itListSubjects)
        {
            if (_isFilterMatch(*itListSubjects, elementFilter))
            {
                LOG_FN_DEBUG(__FILENAME__, __func__, "element found:",
                    (*itListSubjects)->getType(), (*itListSubjects)->getName(), "in", mType, mName);
                /*add element to the list*/
                std::vector<std::shared_ptr<CAmElement > >::const_iterator it = std::find(
                        listOfSubjects.begin(), listOfSubjects.end(),
                        (std::static_pointer_cast<CAmElement >)(*itListSubjects));
                if (it == listOfSubjects.end())
                {
                    listOfSubjects.push_back(*itListSubjects);
                }
            }

            (*itListSubjects)->getListElements(elementFilter, listOfSubjects);
        }
    }

    friend am_Error_e registerElement(std::shared_ptr<CAmElement > pElement);
    friend am_Error_e unregisterElement(std::shared_ptr<CAmElement > pElement);

    virtual std::shared_ptr<CAmElement > getElement();
    void removeObservers();

protected:
    /**
     * @brief This API is used to set the ID of element
     * @param ID: element ID as in DB
     * @return none
     */
    virtual am_Error_e _register(void)
    {
        return E_OK;
    }

    virtual am_Error_e _unregister(void)
    {
        _detachAll();
        return E_OK;
    }

    am_Error_e _deregister(std::shared_ptr<CAmElement > pObserver);
    am_Error_e _register(std::shared_ptr<CAmElement > pObserver);
    void _detachAll();
    bool _isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
        const gc_Element_e &elementType) const;
    bool _isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
        const std::string &elementName) const;
    bool _isFilterMatch(std::shared_ptr<CAmElement > pAmElement, const int &elementPriority) const;
    bool _isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
        const gc_ElementTypeName_s &elementTypeName) const;
    bool _isFilterMatch(std::shared_ptr<CAmElement > pAmElement,
        const struct gc_ElementTypeID_s &elementTypeID) const;

    IAmControlReceive *const                   mpControlReceive;
    const std::string                          mName;
    uint16_t                                   mID;
    const gc_Element_e                         mType;
    /* List of the registered observer*/
    std::vector<std::shared_ptr<CAmElement > > mListObservers;
    /* List of the subjects*/
    std::vector<std::shared_ptr<CAmElement > > mListSubjects;
};

/***************************************************************************//**
 * @class am::gc::CAmLimitableElement
 *
 * Add-On to support multiple independent volume limits
 */

class CAmLimitableElement  : public CAmElement
{
public:
    /** @brief Set or release any of arbitrary volume limit, distinguished by pattern information.
     * 
     *  @param state:   LS_LIMITIED or LS_UNLIMITED
     *         limit:   LT_ABSOLUTE or LT_RELATIVE and maximum volume specification
     *         pattern: bitfield to distinguish between multiple limits, defaults to 0xFFFFFFFF
     */
    void setLimitState(gc_LimitState_e state, const gc_LimitVolume_s &limit,
        uint32_t pattern = DEFAULT_LIMIT_PATTERN);

     /** @brief Request the state of a specific limit, identified by its pattern
     * 
     *   @param limits:  external pair LT_ABSOLUTE or LT_RELATIVE and maximum volume specification
     *   @return Limit state: LS_LIMITED or LS_UNLIMITED
     */
    gc_LimitState_e  getLimit(uint32_t pattern, gc_LimitVolume_s &limit) const;

     /** @brief Populate external list with stored limits.
     * 
     *   @param limits:  external list of pairs LT_ABSOLUTE or LT_RELATIVE and maximum volume specification
     */
    void getLimits(std::list<gc_LimitVolume_s > &limits) const;

     /** @brief Determine if element is muted.
     * 
     *  @return MS_MUTED if any of the stored limits matches mute volume, MS_UNMUTED otherwise.
     */
    virtual am_MuteState_e getMuteState() const;

protected:
    CAmLimitableElement(gc_Element_e type, const std::string &name, IAmControlReceive *pControlReceive);
    virtual ~CAmLimitableElement()  { }

private:
    // list to store the limit volume information, ordered by pattern
    std::map<uint32_t, gc_LimitVolume_s > mMapLimitVolumes; 
};

// *****************************************************************************

static am_Error_e registerElement(std::shared_ptr<CAmElement > pElement)
{
    return pElement->_register();
}

am_Error_e unregisterElement(std::shared_ptr<CAmElement > pElement)
{
    return pElement->_unregister();
}

// *****************************************************************************

template <typename TconstructorParam, typename Telement>
class CAmFactory
{
public:
    CAmFactory()
    {
    }

    virtual ~CAmFactory()
    {
    }

    static am_Error_e destroyElement(void)
    {
        // no need to delete shared pointer as it get deleted when ref. count reaches to zero

        mMapElements.clear();
        return E_OK;
    }

    static am_Error_e destroyElement(const std::string name)
    {
        am_Error_e returnValue = E_OK;
        /*
         * first search if the element with such a name
         * is present in the map
         */
        std::shared_ptr<Telement > pElement = getElement(name);
        if (pElement)
        {
            if (E_OK == unregisterElement(pElement))
            {
                mMapElements.erase(name);
            }
        }

        return returnValue;
    }

    static am_Error_e destroyElement(uint16_t ID)
    {
        am_Error_e returnValue = E_OK;
        /*
         * first search if the element with such a name
         * is present in the map
         */
        std::shared_ptr<Telement > pElement = getElement(ID);
        if (pElement)
        {
            if (E_OK == unregisterElement(pElement))
            {
                mMapElements.erase(pElement->getName());
            }
        }

        return returnValue;
    }

    static std::shared_ptr<Telement > getElement(std::string name)
    {
        std::shared_ptr<Telement >                                            pElement = nullptr;
        typename std::map<std::string, std::shared_ptr<Telement > >::iterator itMapElements;

        itMapElements = mMapElements.find(name);
        if (itMapElements != mMapElements.end())
        {
            pElement = itMapElements->second;
        }

        return pElement;
    }

    static std::shared_ptr<Telement > getElement(uint16_t ID)
    {
        if (ID == 0)
        {
            return nullptr;
        }

        for (auto &itMapElements : mMapElements)
        {
            if (itMapElements.second->getID() == ID)
            {
                return itMapElements.second;
            }
        }

        return nullptr;
    }

    static void getListElements(std::vector<std::shared_ptr<Telement > > &listElements)
    {
        typename std::map<std::string, std::shared_ptr<Telement > >::iterator itMapElements;
        for (itMapElements = mMapElements.begin(); itMapElements != mMapElements.end();
             ++itMapElements)
        {
            listElements.push_back(itMapElements->second);
        }
    }

    static std::shared_ptr<Telement > createElement(const TconstructorParam &t,
        IAmControlReceive *pControlReceive)
    {
        /*
         * first search if the element with such a name
         * is present in the map
         */
        std::shared_ptr<Telement > pElement = getElement(t.name);
        if (pElement == nullptr)
        {
            pElement = std::make_shared < Telement > (t, pControlReceive);
            if (pElement != nullptr)
            {
                if (E_OK != registerElement(pElement))
                {
                    // TBD if element is not register in AM database then decide
                    LOG_FN_ERROR(__FILENAME__, __func__,
                        "element registration with Audio Manager failed", t.name);
                    pElement = nullptr;
                    return nullptr;
                }

                mMapElements[t.name] = pElement;
            }
        }

        return pElement;
    }

protected:
    static std::map<std::string, std::shared_ptr<Telement > > mMapElements;
};

// *****************************************************************************

template <typename TconstructorParam, typename Telement>
std::map<std::string, std::shared_ptr<Telement > > CAmFactory<TconstructorParam, Telement >::mMapElements;

} /* namespace gc */
} /* namespace am */
#endif /* GC_ELEMENT_H_ */
