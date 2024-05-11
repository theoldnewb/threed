#include "log.h"
#include "app.h"
#include "defines.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_thread.h>

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
#define max_log_buf 4096


static SDL_LogOutputFunction    default_log_output_function_    = NULL ;
static void *                   default_log_output_user_data_   = NULL ;
static SDL_Mutex *              log_file_mutex_                 = NULL ;
static SDL_IOStream *           log_file_                       = NULL ;
static char const               log_file_name_[]                = "threed.log" ;
static char                     log_full_name_[max_log_buf]     = { 0 } ;


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static void SDLCALL
log_output_function(
    void *          userdata
,   int             category
,   SDL_LogPriority priority
,   const char *    message
)
{
    if(default_log_output_function_)
    {
        default_log_output_function_(userdata, category, priority, message) ;
    }

    if(!log_file_)
    {
        return ;
    }

    static char const fmt_debug[] = "[DEBUG] " ;
    static char const fmt_info[] = "[INFO] " ;
    static char const fmt_error[] = "[ERROR] " ;
    static char buf[max_log_buf] = { 0 } ;

    SDL_assert(log_file_mutex_) ;
    if(log_file_mutex_)
    {
        SDL_LockMutex(log_file_mutex_) ;
    }

    size_t n = 0 ;

    bool write_to_file = false ;

    switch(priority)
    {
    case SDL_LOG_PRIORITY_VERBOSE:
    case SDL_LOG_PRIORITY_DEBUG:
        n = SDL_strlcpy(buf, fmt_debug, max_log_buf) ;
        SDL_assert(n < max_log_buf) ;
        write_to_file = true ;
        break ;
    case SDL_LOG_PRIORITY_INFO:
        n = SDL_strlcpy(buf, fmt_info, max_log_buf) ;
        SDL_assert(n < max_log_buf) ;
        write_to_file = true ;
        break ;

    case SDL_LOG_PRIORITY_WARN:
    case SDL_LOG_PRIORITY_ERROR:
    case SDL_LOG_PRIORITY_CRITICAL:
        n = SDL_strlcpy(buf, fmt_error, max_log_buf) ;
        SDL_assert(n < max_log_buf) ;
        write_to_file = true ;
        break ;
    default:
        break ;
    }

    if(write_to_file)
    {
        n = SDL_strlcat(buf, message, max_log_buf) ;
        SDL_assert(n < max_log_buf) ;
        buf[n++] = '\n' ;
        SDL_assert(n < max_log_buf) ;
        buf[n] = 0 ;
        SDL_assert(n+1 < max_log_buf) ;

        size_t const written = SDL_WriteIO(log_file_, buf, n) ;
        SDL_assert(written == n) ;
    }

    if(log_file_mutex_)
    {
        SDL_UnlockMutex(log_file_mutex_) ;
    }
}


void
create_log_file(
    int const enable
)
{
    if(!enable)
    {
        return ;
    }

    SDL_assert(!log_file_ ) ;

    size_t n = 0 ;
    n = SDL_strlcpy(log_full_name_, app_->pref_path_, max_log_buf) ;
    SDL_assert(n < max_log_buf) ;
    n = SDL_strlcat(log_full_name_, log_file_name_, max_log_buf) ;
    SDL_assert(n < max_log_buf) ;

    log_debug_str(log_full_name_) ;

    log_file_ = SDL_IOFromFile(log_full_name_, "wb") ;
    SDL_assert(log_file_) ;
    if(!log_file_)
    {
        return ;
    }

    SDL_assert(!log_file_mutex_) ;
    log_file_mutex_ = SDL_CreateMutex() ;
    SDL_assert(log_file_mutex_) ;

    SDL_GetLogOutputFunction(&default_log_output_function_, &default_log_output_user_data_) ;
    SDL_SetLogOutputFunction(log_output_function, NULL) ;
}


void
destroy_log_file()
{
    if(log_file_mutex_)
    {
        SDL_DestroyMutex(log_file_mutex_) ;
        log_file_mutex_ = NULL ;
    }

    if(log_file_)
    {
        int const res = SDL_CloseIO(log_file_) ;
        log_file_ = NULL ;
        SDL_assert(0 == res) ;
    }


}




static char const *
basename(
    char const * p
)
{
    SDL_assert(p) ;

    int i = 0 ;
    int j = 0 ;

    for( i=0 ; p[i] ; ++i )
    {
        if(p[i] == '/' || p[i] == '\\')
        {
            j = i + 1 ;
        }
    }

    return p + j ;
}


void
log_output_impl(
    char const *    file
,   char const *    func
,   int const       line
,   log_prio const  prio
,   char const *    fmt
,   ...
)
{

    static char const fmt_debug[] = "%15" SDL_PRIu64 "%2u %16s(%4d) : %s : %s" ;
    static char const fmt_info[] = " %15" SDL_PRIu64 "%2u %16s(%4d) : %s : %s" ;
    static char const fmt_error[] = "%15" SDL_PRIu64 "%2u %16s(%4d) : %s : %s" ;
    char buf[max_log_buf] ;
    va_list args ;
    va_start(args, fmt) ;
    SDL_vsnprintf(buf, max_log_buf, fmt, args) ;
    va_end(args) ;
    unsigned int const current_thread_id = (unsigned char)SDL_GetCurrentThreadID() ;
    Uint64 const current_time = SDL_GetPerformanceCounter() - app_->performance_counter_0_ ;

    switch(prio)
    {
    case LOG_PRI_DEBUG:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, fmt_debug, current_time, current_thread_id, basename(file), line, func, buf) ;
        break ;
    case LOG_PRI_INFO:
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, fmt_info, current_time, current_thread_id, basename(file), line, func, buf) ;
        break ;
    case LOG_PRI_ERROR:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, fmt_error, current_time, current_thread_id, basename(file), line, func, buf) ;
        break ;
    }

}
