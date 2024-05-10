#pragma once


#ifndef APP_H_INC
#define APP_H_INC


#include "types.h"


typedef struct SDL_Window SDL_Window;


typedef struct app
{
    char ** argv_ ;
    int     argc_ ;
    int     exit_code_ ;

    SDL_Window *    window_ ;
    int             window_width_ ;
    int             window_height_ ;

    int             running_ ;
    int             minimized_ ;
    int             keyboard_focus_ ;

    uint64_t        performance_counter_0_ ;
    char const *    base_path_ ;
    char const *    pref_path_ ;


} app ;


int
app_main(
    int     argc
,   char *  argv[]
) ;


extern app * app_ ;


#endif // APP_H_INC


