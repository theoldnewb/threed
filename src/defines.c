#include "defines.h"
#include "log.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_error.h>

#include <errno.h>
#include <string.h>


int
check_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
)
{
    log_output_impl(file, func, line, LOG_PRI_DEBUG, "%d=%s", result, expr) ;

    if(result)
    {
        return 0 ;
    }

    return 1 ;
}


int
check_c_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
)
{

    if(0 == result)
    {
        log_output_impl(file, func, line, LOG_PRI_DEBUG, "%d=%s", result, expr) ;
        return 0 ;
    }

    log_output_impl(file, func, line, LOG_PRI_DEBUG, "%d=%s failed. errno=%d strerror=%s", result, expr, errno, strerror(errno)) ;

    return 1 ;
}



int
check_sdl_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
)
{
    if(result)
    {
        log_output_impl(file, func, line, LOG_PRI_DEBUG, "%d=%s", result, expr) ;
        return 0 ;
    }

    log_output_impl(file, func, line, LOG_PRI_DEBUG, "%d=%s failed. SDL_GetError=%s", result, expr, SDL_GetError()) ;

    return 1 ;
}

