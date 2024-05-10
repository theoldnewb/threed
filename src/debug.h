#pragma once


#ifndef DEBUG_H_INC
#define DEBUG_H_INC


#include "types.h"


typedef struct debug_info
{
    char const *    file_ ;
    char const *    func_ ;
    int             line_ ;

} debug_info ;


typedef struct debug_counter
{
    debug_info  begin_info_ ;
    debug_info  end_info_ ;
    uint64_t    begin_time_ ;
    uint64_t    end_time_ ;

} debug_counter ;


void
begin_timed_block_impl(
    char const *    file
,   char const *    func
,   int const       line
) ;


void
end_timed_block_impl(
    char const *    file
,   char const *    func
,   int const       line
) ;


#define  ENABLE_TIMED_BLOCK

#ifdef  ENABLE_TIMED_BLOCK
#define begin_timed_block() begin_timed_block_impl(__FILE__, __func__, __LINE__)
#define end_timed_block()   end_timed_block_impl(__FILE__, __func__, __LINE__)
#else
#define begin_timed_block() def_noop
#define end_timed_block()   def_noop
#endif



#endif  // DEBUG_H_INC
