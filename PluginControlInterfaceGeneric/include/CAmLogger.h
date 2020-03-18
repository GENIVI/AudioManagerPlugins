/******************************************************************************
 * @file  CAmLogger.h
 *
 * This file contains the declaration of functions used for logging purpose
 *
 * @component{AudioManager Generic Controller}
 *
 * @authors Toshiaki Isogai <tisogai@jp.adit-jv.com>,\n
 *          Kapildev Patel  <kpatel@jp.adit-jv.com>,\n
 *          Prashant Jain   <pjain@jp.adit-jv.com>,\n
 *          Martin Koch     <mkoch@de.adit-jv.com>
 *
 * @copyright (c) 2015 - 2020 Advanced Driver Information Technology.\n
 * This code is developed by Advanced Driver Information Technology.\n
 * Copyright of Advanced Driver Information Technology, Bosch, and DENSO.\n
 * All rights reserved.
 *
 *****************************************************************************/

#ifndef GC_LOGGER_H_
#define GC_LOGGER_H_

#include "IAmControlCommon.h"

#define GC_CONTEXT_STRING      "GC"
#define GC_CONTEXT_DESCRIPTION "Logging Context for GenIVI Generic Audio-Manager-Controller"

namespace am {
namespace gc {

/**
 * Lazy-initialized Singleton instance of logging context.
 * Implementation moved to CAmCommonUtility.cpp
 * 
 * @param loglevel: optional severity threshold for log output. -1 leaves default untouched.
 */
extern am::IAmLogContext &GenericControllerLogger(am_LogLevel_e loglevel = static_cast<am_LogLevel_e>(-1));

#define LOG_DEBUG_DEFAULT_VALUE   LL_INFO

/*
 * Helper functions to log
 */
template <typename... Args>
inline void LOG_FN_ENTRY(Args... args)
{
    GenericControllerLogger().debug(">> ", args...);
}

template <typename... Args>
inline void LOG_FN_EXIT(Args... args)
{
    GenericControllerLogger().debug("<<", args...);
}

template <typename... Args>
inline void LOG_FN_DEBUG(Args... args)
{
    GenericControllerLogger().debug(args...);
}

/**
 * logs a given value with warning level with the default context
 * @param ...
 */
template <typename... Args>
inline void LOG_FN_WARN(Args... args)
{
    GenericControllerLogger().warn(args...);
}

/**
 * logs a given value with error level with the default context
 * @param ...
 */
template <typename... Args>
inline void LOG_FN_ERROR(Args... args)
{
    GenericControllerLogger().error(args...);
}

/**
 * logs a given value with infolevel with the default context
 * @param ...
 */
template <typename... Args>
inline void LOG_FN_INFO(Args... args)
{
    GenericControllerLogger().info(args...);
}

inline void LOG_FN_CHANGE_LEVEL(const am_LogLevel_e loglevel)
{
    (void)GenericControllerLogger(loglevel);
}

} /* namespace gc */
} /* namespace am */
#endif /* GC_LOGGER_H_ */
