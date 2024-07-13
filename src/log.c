#include "app.h"
#include "log.h"
#include "defines.h"


#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_iostream.h>


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

    if(!message)
    {
        return ;
    }

    static char const fmt_debug[]   = "[DEBUG] " ;
    static char const fmt_info[]    = "[INFO] " ;
    static char const fmt_error[]   = "[ERROR] " ;
    static char const fmt_unknown[] = "[UNKNO] " ;
    static char buf[max_log_buf] = { 0 } ;

    require(log_file_mutex_) ;
    if(log_file_mutex_)
    {
        SDL_LockMutex(log_file_mutex_) ;
    }

    size_t n = 0 ;

    switch(priority)
    {
    case SDL_LOG_PRIORITY_VERBOSE:
    case SDL_LOG_PRIORITY_DEBUG:
        n = SDL_strlcpy(buf, fmt_debug, max_log_buf) ;
        require(n < max_log_buf) ;
        break ;
    case SDL_LOG_PRIORITY_INFO:
        n = SDL_strlcpy(buf, fmt_info, max_log_buf) ;
        require(n < max_log_buf) ;
        break ;

    case SDL_LOG_PRIORITY_WARN:
    case SDL_LOG_PRIORITY_ERROR:
    case SDL_LOG_PRIORITY_CRITICAL:
        n = SDL_strlcpy(buf, fmt_error, max_log_buf) ;
        require(n < max_log_buf) ;
        break ;
    default:
        n = SDL_strlcpy(buf, fmt_unknown, max_log_buf) ;
        require(n < max_log_buf) ;
        break ;
    }

    n = SDL_strlcat(buf, message, max_log_buf) ;
    require(n < max_log_buf) ;
    buf[n++] = '\n' ;
    require(n < max_log_buf) ;
    buf[n] = 0 ;
    require(n+1 < max_log_buf) ;

    size_t const written = SDL_WriteIO(log_file_, buf, n) ;
    require(written == n) ;

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

    require(!log_file_ ) ;

    size_t n = 0 ;
    n = SDL_strlcpy(log_full_name_, app_->pref_path_, max_log_buf) ;
    require(n < max_log_buf) ;
    n = SDL_strlcat(log_full_name_, log_file_name_, max_log_buf) ;
    require(n < max_log_buf) ;

    log_debug_str(log_full_name_) ;

    log_file_ = SDL_IOFromFile(log_full_name_, "wb") ;
    require(log_file_) ;
    if(!log_file_)
    {
        return ;
    }

    require(!log_file_mutex_) ;
    log_file_mutex_ = SDL_CreateMutex() ;
    require(log_file_mutex_) ;

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
        require(0 == res) ;
    }
}




static char const *
basename(
    char const * p
)
{
    require(p) ;

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

    static char const fmt_debug[] = "%12" SDL_PRIu64 "%2u %30s(%4d) : %s : %s" ;
    static char const fmt_info[] = " %12" SDL_PRIu64 "%2u %30s(%4d) : %s : %s" ;
    static char const fmt_error[] = "%12" SDL_PRIu64 "%2u %30s(%4d) : %s : %s" ;
    char buf[max_log_buf] ;
    va_list args ;
    va_start(args, fmt) ;
    SDL_vsnprintf(buf, max_log_buf, fmt, args) ;
    va_end(args) ;
    unsigned int const current_thread_id = (unsigned char)SDL_GetCurrentThreadID() ;
    Uint64 const current_time = get_app_time() ;

    switch(prio)
    {
    case LOG_PRI_DEBUG:
        SDL_LogDebug(
            SDL_LOG_CATEGORY_APPLICATION
        ,   fmt_debug
        ,   current_time
        ,   current_thread_id
        ,   basename(file)
        ,   line
        ,   func
        ,   buf
        ) ;
        break ;

    case LOG_PRI_INFO:
        SDL_LogInfo(
            SDL_LOG_CATEGORY_APPLICATION
        ,   fmt_info
        ,   current_time
        ,   current_thread_id
        ,   basename(file)
        ,   line
        ,   func
        ,   buf
        ) ;
        break ;

    case LOG_PRI_ERROR:
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION
        ,   fmt_error
        ,   current_time
        ,   current_thread_id
        ,   basename(file)
        ,   line
        ,   func
        ,   buf
        ) ;
        break ;
    }

}
