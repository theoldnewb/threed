#pragma once


#ifndef LOG_H_INC
#define LOG_H_INC


#include "defines.h"


typedef enum
{
    LOG_PRI_DEBUG
,   LOG_PRI_INFO
,   LOG_PRI_ERROR
} log_prio ;


void
log_output_impl(
    char const *    file
,   char const *    func
,   int const       line
,   log_prio const  prio
,   char const *    fmt
,   ...
) ;


void
create_log_file(
    int const enable
) ;

void
destroy_log_file() ;


#define ENABLE_DEBUG_LOG

#ifdef  ENABLE_DEBUG_LOG
#define log_debug(fmt, ...) log_output_impl(__FILE__, __func__, __LINE__, LOG_PRI_DEBUG, fmt, ##__VA_ARGS__)
#define log_debug_str(a)    log_debug("%s=%s", #a, a)
#define log_debug_ptr(a)    log_debug("%s=%p", #a, a)
#define log_debug_u64(a)    log_debug("%s=%" SDL_PRIu64 , #a, a)
#define log_debug_u32(a)    log_debug("%s=%u", #a, a)
#else
#define log_debug(fmt, ...) def_noop
#define log_debug_str(a)    def_noop
#define log_debug_ptr(a)    def_noop
#define log_debug_u32(a)    def_noop
#endif


#define ENABLE_INFO_LOG

#ifdef  ENABLE_INFO_LOG
#define log_info(fmt, ...) log_output_impl(__FILE__, __func__, __LINE__, LOG_PRI_INFO, fmt, ##__VA_ARGS__)
#else
#define log_info(fmt, ...) def_noop
#endif


#define ENABLE_ERROR_LOG

#ifdef  ENABLE_ERROR_LOG
#define log_error(fmt, ...) log_output_impl(__FILE__, __func__, __LINE__, LOG_PRI_ERROR, fmt, ##__VA_ARGS__)
#else
#define log_error(fmt, ...) def_noop
#endif



#endif // LOG_H_INC


