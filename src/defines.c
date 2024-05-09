#include "defines.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_error.h>

#include <errno.h>

int
check_impl(
    int             result
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
)
{
    SDL_Log("%s(%d) %s: %d=%s", file, line, func, result, expr) ;

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
        SDL_Log("%s(%d) %s: %d=%s", file, line, func, result, expr) ;
        return 0 ;
    }

    SDL_Log("%s(%d) %s: %d=%s failed. errno=%d", file, line, func, result, expr, errno) ;

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
        SDL_Log("%s(%d) %s: %d=%s", file, line, func, result, expr) ;
        return 0 ;
    }

    SDL_Log("%s(%d) %s: %d=%s failed. SDL_GetError()=%s", file, line, func, result, expr, SDL_GetError()) ;

    return 1 ;
}

