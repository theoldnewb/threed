#include "debug.h"
#include <SDL3/SDL_timer.h>



void
begin_timed_block_impl(
    char const *    file
,   char const *    func
,   int const       line
)
{
    debug_counter dc ;
    debug_info * di = &dc.begin_info_ ;
    di->file_       = file ;
    di->func_       = func ;
    di->line_       = line ;
    dc.begin_time_  = SDL_GetPerformanceCounter() ;
    dc.end_time_    = 0 ;
}


void
end_timed_block_impl(
    char const *    file
,   char const *    func
,   int const       line
)
{

    debug_counter dc ;
    debug_info * di = &dc.end_info_ ;
    di->file_       = file ;
    di->func_       = func ;
    di->line_       = line ;
    dc.end_time_    = SDL_GetPerformanceCounter() ;
}


