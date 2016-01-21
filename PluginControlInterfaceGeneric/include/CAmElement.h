/******************************************************************************
 * @file: CAmElement.h
 *
 * This file contains the declaration of element class (member functions and
 * data members) used as base class for source, sink & gateway element classes
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

#ifndef GC_ELEMENT_H_
#define GC_ELEMENT_H_

#include "CAmTypes.h"
#include "IAmControlCommon.h"
#include "CAmLogger.h"

namespace am {
namespace gc {

#define DEFAULT_ELEMENT_PRIORITY    (100)
class CAmControlReceive;
class CAmElement;

static am_Error_e Register(CAmElement* pElement);
static am_Error_e UnRegister(CAmElement* pElement);

class CAmElement
{
public:
    /**
     * @brief It is the constructor of element class. Initialize the member
     * variables with default value. It will be invoked during creation of any
     * element (gateway/source/sink).
     * @param name: name of the element
     *        pControlReceive: pointer to CAmControlReceive Class object
     * @return none
     */
    CAmElement(const std::string& name, CAmControlReceive* pControlReceive);
    /**
     * @brief It is the destructor of element class.
     * @param none
     * @return none
     */
    virtual ~CAmElement();
    virtual CAmControlReceive* getControlReceive(void);
    /**
     * @brief This API is used to set the type of element
     * @return type of element
     */
    virtual int setType(const gc_Element_e type);
    /**
     * @brief This API is used to get the type of element
     * @return type of element
     */
    virtual gc_Element_e getType(void) const;
    /**
     * @brief This API is used to get the name of element
     * @param none
     * @return element name
     */
    virtual std::string getName(void) const;
    /**
     * @brief This API is used to set the ID of element
     * @param ID: element ID as in DB
     * @return none
     */
    virtual void setID(const uint16_t ID);
    /**
     * @brief This API is used to get the ID of element
     * @param none
     * @return element ID as in DB
     */
    virtual uint16_t getID(void) const;
    /**
     * @brief This API is used to get the priority of element as defined in configuration file.
     * @param[in] priority priority of element
     * @return E_OK
     */
    virtual am_Error_e setPriority(const int32_t priority);
    /**
     * @brief This API is used to get the priority of element as defined in configuration file.
     * @param priority: variable in which priority of element will be returned
     * @return E_OK
     */
    virtual am_Error_e getPriority(int32_t& priority) const;
    virtual am_Error_e setState(int state);
    virtual am_Error_e getState(int& state) const;
    virtual am_Error_e setVolume(const am_volume_t volume);
    virtual am_Error_e getVolume(am_volume_t& volume) const;
    virtual am_Error_e setMainVolume(const am_mainVolume_t volume);
    virtual am_Error_e getMainVolume(am_mainVolume_t& mainVolume) const;
    /**
     * @brief This API is used to set the offset volume of element as needed for limit/unlimit action action.
     * @param volume: offset volume of element
     * @return none
     */
    virtual am_Error_e setLimitVolume(const gc_LimitVolume_s& limitVolume);
    /**
     * @brief This API is used to get the offset volume of element as needed for limit/unlimit volume action.
     * @param none
     * @return offset volume of element
     */
    virtual am_Error_e getLimitVolume(gc_LimitVolume_s& limitVolume) const;
    virtual am_Error_e setMuteState(const am_MuteState_e muteState);
    virtual am_Error_e getMuteState(am_MuteState_e& muteState) const;
    virtual am_Error_e setInterruptState(const am_InterruptState_e interruptState);
    virtual am_Error_e getInterruptState(am_InterruptState_e& interruptState) const;
    /**
     * @brief This API is used to update the availability of element as in database.
     * It is just the dummy implementation and actual implementation will be provided by derived element class.
     * @param availability: variable in which availability will be returned
     * @return E_NOT_POSSIBLE because derived class should implement this interface as valid interface
     */
    virtual am_Error_e setAvailability(const am_Availability_s& availability);
    /**
     * @brief This API is used to get the availability of element as in database.
     * It is just the dummy implementation and actual implementation will be provided by derived element class.
     * @param availability: variable in which availability will be returned
     * @return E_NOT_POSSIBLE because derived class should implement this interface as valid interface
     */
    virtual am_Error_e getAvailability(am_Availability_s& availability) const;

    friend am_Error_e Register(CAmElement* pElement);
    friend am_Error_e UnRegister(CAmElement* pElement);
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
        return E_OK;
    }

private:
    CAmControlReceive* mpControlRecieve;
    std::string mName;
    uint16_t mID;
    gc_Element_e mType;
    int32_t mPriority;
    am_volume_t mVolume;
    am_mainVolume_t mMainVolume;
    gc_LimitVolume_s mLimitVolume;
    am_MuteState_e mMuteState;
    int mState;
    am_InterruptState_e mInterruptState;
    am_Availability_s mAvailability;
};

static am_Error_e Register(CAmElement* pElement)
{
    return pElement->_register();
}

static am_Error_e UnRegister(CAmElement* pElement)
{
    return pElement->_unregister();
}

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
        typename std::map<std::string, Telement* >::iterator itMapElements;
        for (itMapElements = mMapElements.begin(); itMapElements != mMapElements.end();
                        ++itMapElements)
        {
            delete itMapElements->second;
        }
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
        Telement* pElement = getElement(name);
        if (pElement != NULL)
        {
            if (E_OK == UnRegister(pElement))
            {
                mMapElements.erase(name);
                delete pElement;
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
        Telement* pElement = getElement(ID);
        if (pElement != NULL)
        {

            if (E_OK == UnRegister(pElement))
            {
                mMapElements.erase(pElement->getName());
                delete pElement;
            }
        }
        return returnValue;
    }

    static Telement* getElement(std::string name)
    {
        Telement* pElement = NULL;
        typename std::map<std::string, Telement* >::iterator itMapElements;
        itMapElements = mMapElements.find(name);
        if (itMapElements != mMapElements.end())
        {
            pElement = itMapElements->second;
        }
        return pElement;
    }

    static Telement* getElement(uint16_t ID)
    {
        Telement* pElement = NULL;
        if (ID != 0)
        {
            typename std::map<std::string, Telement* >::iterator itMapElements;
            for (itMapElements = mMapElements.begin(); itMapElements != mMapElements.end();
                            ++itMapElements)
            {
                if (itMapElements->second->getID() == ID)
                {
                    pElement = itMapElements->second;
                    break;
                }
            }
        }
        return pElement;
    }

    static void getListElements(std::vector<Telement* >& listElements)
    {
        typename std::map<std::string, Telement* >::iterator itMapElements;
        for (itMapElements = mMapElements.begin(); itMapElements != mMapElements.end();
                        ++itMapElements)
        {
            listElements.push_back(itMapElements->second);
        }
    }

    static Telement* createElement(const TconstructorParam& t, CAmControlReceive* pControlReceive)
    {
        int returnValue;
        /*
         * first search if the element with such a name
         * is present in the map
         */
        Telement* pElement = getElement(t.name);
        if (pElement == NULL)
        {
            pElement = new Telement(t, pControlReceive);
            if (pElement != NULL)
            {

                if (E_OK != Register(pElement))
                {
                    delete pElement;
                    pElement = NULL;
                }
            }
            if (pElement)
            {
                LOG_FN_DEBUG(t.name);
                mMapElements[t.name] = pElement;
            }
        }
        return pElement;
    }

private:
    static std::map<std::string, Telement* > mMapElements;
};
template <typename TconstructorParam, typename Telement>
std::map<std::string, Telement* > CAmFactory<TconstructorParam, Telement >::mMapElements;

} /* namespace gc */
} /* namespace am */
#endif /* GC_ELEMENT_H_ */
