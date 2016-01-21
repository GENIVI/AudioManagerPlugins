/******************************************************************************
 * @file: CAmLogger.h
 *
 * This file contains the declaration of functions used for logging purpose
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

#ifndef GC_LOGGER_H_
#define GC_LOGGER_H_

#include "IAmControlCommon.h"

extern DltContext GenericControllerDlt;

namespace am {
namespace gc {

#define LOG_DEBUG_DEFAULT_VALUE 4
/**
 * logs a given value with infolevel with the default context
 * @param value
 */
template <typename T> void CAmLogInfo(T value)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value);
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 */
template <typename T, typename T1> void CAmLogInfo(T value, T1 value1)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value, value1);
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 */
template <typename T, typename T1, typename T2> void CAmLogInfo(T value, T1 value1, T2 value2)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value, value1, value2);
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 */
template <typename T, typename T1, typename T2, typename T3> void CAmLogInfo(T value, T1 value1,
                                                                             T2 value2, T3 value3)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value, value1, value2, value3);
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 */
template <typename T, typename T1, typename T2, typename T3, typename T4> void CAmLogInfo(T value,
                                                                                          T1 value1,
                                                                                          T2 value2,
                                                                                          T3 value3,
                                                                                          T4 value4)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value, value1, value2, value3, value4);
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 */
template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5> void CAmLogInfo(
                T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value, value1, value2, value3, value4, value5);
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 */
template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> void CAmLogInfo(
                T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value, value1, value2, value3, value4, value5, value6);
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 */
template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
                typename T7> void CAmLogInfo(T value, T1 value1, T2 value2, T3 value3, T4 value4,
                                             T5 value5, T6 value6, T7 value7)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value, value1, value2, value3, value4, value5, value6,
        value7);
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 */
template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
                typename T7, typename T8> void CAmLogInfo(T value, T1 value1, T2 value2, T3 value3,
                                                          T4 value4, T5 value5, T6 value6,
                                                          T7 value7, T8 value8)
{
    log(&GenericControllerDlt, DLT_LOG_INFO, value, value1, value2, value3, value4, value5, value6,
        value7, value8);
}

/**
 * logs a given value with error level with the default context
 * @param value
 */
template <typename T> void CAmLogError(T value)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value);
}

/**
 * logs a given value with error level with the default context
 * @param value
 * @param value1
 */
template <typename T, typename T1> void CAmLogError(T value, T1 value1)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value, value1);
}

/**
 * logs a given value with error level with the default context
 * @param value
 * @param value1
 * @param value2
 */
template <typename T, typename T1, typename T2> void CAmLogError(T value, T1 value1, T2 value2)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value, value1, value2);
}

/**
 * logs a given value with error level with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 */
template <typename T, typename T1, typename T2, typename T3> void CAmLogError(T value, T1 value1,
                                                                              T2 value2, T3 value3)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value, value1, value2, value3);
}

/**
 * logs a given value with error level with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 */
template <typename T, typename T1, typename T2, typename T3, typename T4> void CAmLogError(
                T value, T1 value1, T2 value2, T3 value3, T4 value4)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value, value1, value2, value3, value4);
}

/**
 * logs a given value with error level with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 */
template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5> void CAmLogError(
                T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value, value1, value2, value3, value4, value5);
}

/**
 * logs a given value with error level with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 */
template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> void CAmLogError(
                T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value, value1, value2, value3, value4, value5,
        value6);
}

/**
 * logs a given value with error level with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 */
template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
                typename T7> void CAmLogError(T value, T1 value1, T2 value2, T3 value3, T4 value4,
                                              T5 value5, T6 value6, T7 value7)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value, value1, value2, value3, value4, value5, value6,
        value7);
}

/**
 * logs a given value with error level with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 */
template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
                typename T7, typename T8> void CAmLogError(T value, T1 value1, T2 value2, T3 value3,
                                                           T4 value4, T5 value5, T6 value6,
                                                           T7 value7, T8 value8)
{
    log(&GenericControllerDlt, DLT_LOG_ERROR, value, value1, value2, value3, value4, value5, value6,
        value7, value8);
}

template <typename T> void CAmLogDebug(T value)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value);
}

template <typename T, typename T1> void CAmLogDebug(T value, T1 value1)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value, value1);
}

template <typename T, typename T1, typename T2> void CAmLogDebug(T value, T1 value1, T2 value2)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value, value1, value2);
}

template <typename T, typename T1, typename T2, typename T3> void CAmLogDebug(T value, T1 value1,
                                                                              T2 value2, T3 value3)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value, value1, value2, value3);
}

template <typename T, typename T1, typename T2, typename T3, typename T4> void CAmLogDebug(
                T value, T1 value1, T2 value2, T3 value3, T4 value4)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value, value1, value2, value3, value4);
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5> void CAmLogDebug(
                T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value, value1, value2, value3, value4, value5);
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> void CAmLogDebug(
                T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value, value1, value2, value3, value4, value5,
        value6);
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
                typename T7> void CAmLogDebug(T value, T1 value1, T2 value2, T3 value3, T4 value4,
                                              T5 value5, T6 value6, T7 value7)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value, value1, value2, value3, value4, value5, value6,
        value7);
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
                typename T7, typename T8> void CAmLogDebug(T value, T1 value1, T2 value2, T3 value3,
                                                           T4 value4, T5 value5, T6 value6,
                                                           T7 value7, T8 value8)
{
    log(&GenericControllerDlt, DLT_LOG_DEBUG, value, value1, value2, value3, value4, value5, value6,
        value7, value8);
}

#define LOG_FN_ENTRY(...)   CAmLogDebug(">> ",__FILENAME__,__func__, ##__VA_ARGS__)
#define LOG_FN_EXIT(...)    CAmLogDebug("<<",__FILENAME__,__func__, ##__VA_ARGS__)
#define LOG_FN_DEBUG(...)   CAmLogDebug(__FILENAME__,__func__, ##__VA_ARGS__)
#define LOG_FN_ERROR(...)   CAmLogError(__FILENAME__,__func__, ##__VA_ARGS__)
#define LOG_FN_INFO(...)    CAmLogInfo(__FILENAME__,__func__, ##__VA_ARGS__)
#define LOG_FN_REGISTER_CONTEXT()  \
{\
    CAmDltWrapper::instance()->registerContext(GenericControllerDlt,"AMCO","Generic Controller Context",DLT_LOG_DEFAULT,DLT_TRACE_STATUS_OFF);\
}
#define LOG_FN_CHANGE_LEVEL(A)  \
{\
    CAmDltWrapper::instance()->unregisterContext(GenericControllerDlt);\
    CAmDltWrapper::instance()->registerContext(GenericControllerDlt,"AMCO","Generic Controller Context",(DltLogLevelType)A,DLT_TRACE_STATUS_OFF);\
}
} /* namespace gc */
} /* namespace am */
#endif /* GC_LOGGER_H_ */
