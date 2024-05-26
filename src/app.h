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


double
get_performance_frequency_inverse() ;


void *
alloc_memory_impl(
    size_t const    byte_count
,   int const       clear_memory
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
) ;


void *
alloc_array_impl(
    size_t const    count
,   size_t const    byte_size
,   int const       clear_memory
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
) ;


void
free_memory_impl(
    void *          mem
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
) ;


bool
load_file(
    void **             out_memory
,   uint64_t *          out_size
,   char const * const  fullname
) ;



#define alloc_memory(t, bs) ((t*)alloc_memory_impl(bs, 0, #t, __FILE__, __func__, __LINE__))
#define alloc_array(t, n)   ((t*)alloc_array_impl(n, sizeof(t), 0, #t, __FILE__, __func__, __LINE__))
#define free_memory(p)      free_memory_impl(p, #p, __FILE__, __func__, __LINE__)

extern app * app_ ;

