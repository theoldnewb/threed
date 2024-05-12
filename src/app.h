#pragma once


#include "types.h"


typedef struct SDL_Window SDL_Window;


typedef struct app
{
    char ** argv_ ;
    int     argc_ ;

    uint32_t        subsystems_ ;
    SDL_Window *    window_ ;
    int             window_width_ ;
    int             window_height_ ;

    bool            created_ ;
    bool            running_ ;
    bool            minimized_ ;
    bool            keyboard_focus_ ;

    uint64_t        performance_counter_0_ ;
    char const *    base_path_ ;
    char const *    pref_path_ ;


} app ;


int
main_app(
    int     argc
,   char *  argv[]
) ;


uint64_t
get_app_time() ;


extern app * app_ ;

