#include "app.h"
#include "check.h"
#include "defines.h"
#include "log.h"
#include "debug.h"
#include "vulkan.h"


#include <SDL3/SDL_log.h>
#include <SDL3/SDL_version.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_filesystem.h>

#include <cglm/mat4.h>
#include <cglm/io.h>


#include <vulkan/vulkan.h>

#include <sys/stat.h>


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static app the_app_ ;
app * app_ = &the_app_ ;


static char const   org_name_[] = "whatever" ;
static char const   app_name_[] = "threed" ;



////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
void
hello_vulkan()
{
    uint32_t vulkan_api_version ;
    VkResult const res = vkEnumerateInstanceVersion(&vulkan_api_version) ;
    require(res == VK_SUCCESS) ;
    uint32_t const major = VK_VERSION_MAJOR(vulkan_api_version) ;
    uint32_t const minor = VK_VERSION_MINOR(vulkan_api_version) ;
    uint32_t const patch = VK_VERSION_PATCH(vulkan_api_version) ;
    log_debug("Vulkan Version: major=%d, minor=%d, patch=%d", major, minor, patch) ;
}

void
hello_cglm()
{
    mat4 m4 ;
    glm_mat4_identity(m4) ;
    glm_mat4_print(m4, stdout) ;
}


void
hello_sdl3()
{
    SDL_Version compiled ;
    SDL_Version linked ;
    SDL_VERSION(&compiled) ;
    SDL_GetVersion(&linked) ;
    log_debug("compiled with: %u, %u, %u", compiled.major, compiled.minor, compiled.patch) ;
    log_debug("linking with: %u, %u, %u", linked.major, linked.minor, linked.patch) ;
    log_debug("SDL_GetRevision()=%s", SDL_GetRevision()) ;
}


void
hello_debug()
{

    log_debug("test_debug_file_func=%d", test_debug_file_func()) ;

    begin_timed_block() ;

    SDL_Delay(30) ;

    end_timed_block() ;
}


uint64_t
get_app_time()
{
    require(app_->performance_counter_0_) ;
    return SDL_GetPerformanceCounter() - app_->performance_counter_0_ ;
}


bool
create_app()
{
#ifndef NDEBUG
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG) ;
#endif
    // BUG(tom): if this call actually returns 0,which is highly unlikely.
    // Then get_app_time() will fail. For now, we don't care and hope that
    // calling it twice will make it physically impossible...
    app_->performance_counter_0_ = SDL_GetPerformanceCounter() ;
    app_->performance_counter_0_ = SDL_GetPerformanceCounter() ;
    require(app_->performance_counter_0_) ;

    app_->base_path_ = SDL_GetBasePath() ;
    require(app_->base_path_) ;
    app_->pref_path_ = SDL_GetPrefPath(org_name_, app_name_) ;
    require(app_->pref_path_) ;

#ifdef  ENABLE_LOG_FILE
    create_log_file(true) ;
#endif

    log_debug("Hello, World!") ;
    log_info("Hello, World!") ;
    log_error("Hello, World!") ;
    log_debug_str(app_->base_path_) ;
    log_debug_str(app_->pref_path_) ;

    app_->subsystems_ = SDL_INIT_VIDEO ;
    if(check_sdl(0 == SDL_Init(app_->subsystems_)))
    {
        return  false ;
    }

    app_->window_width_     = 1280 ;
    app_->window_height_    = 768 ;
    require(!app_->window_) ;

    app_->window_ = SDL_CreateWindow(
        app_name_
    ,   app_->window_width_
    ,   app_->window_height_
    ,   SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
    ) ;

    if(check_sdl(0 != app_->window_))
    {
        return false ;
    }

    app_->created_ = true ;
    return true ;
}


void
destroy_app()
{
    if(app_->window_)
    {
        SDL_DestroyWindow(app_->window_) ;
        app_->window_ = NULL ;
    }

    if(app_->subsystems_)
    {
        SDL_QuitSubSystem(app_->subsystems_) ;
        app_->subsystems_ = 0 ;
    }

    log_debug("And we are done.") ;

#ifdef  ENABLE_LOG_FILE
    destroy_log_file() ;
#endif
}


bool
update_app()
{
    //begin_timed_block() ;
    //SDL_Delay(10) ;
    //end_timed_block() ;
    return true ;
}


void
handle_window_event(
    SDL_Event * event
)
{
    require(event) ;

    SDL_WindowEvent * we = &event->window ;
    switch(event->type)
    {
    case SDL_EVENT_WINDOW_SHOWN:
        break ;
    case SDL_EVENT_WINDOW_HIDDEN:
        break ;
    case SDL_EVENT_WINDOW_EXPOSED:
        break ;
    case SDL_EVENT_WINDOW_MOVED:
        break ;
    case SDL_EVENT_WINDOW_RESIZED:
        log_debug("Resized: [%d,%d]", we->data1, we->data2) ;
        break ;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        break ;
    case SDL_EVENT_WINDOW_MINIMIZED:
        app_->minimized_ = true ;
        break ;
    case SDL_EVENT_WINDOW_MAXIMIZED:
        app_->minimized_ = false ;
        break ;
    case SDL_EVENT_WINDOW_RESTORED:
        app_->minimized_ = false ;
        break ;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
        break ;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        break ;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        app_->keyboard_focus_ = true ;
        break ;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        app_->keyboard_focus_ = false ;
        break ;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        break ;
    case SDL_EVENT_WINDOW_TAKE_FOCUS:
        break ;
    case SDL_EVENT_WINDOW_HIT_TEST:
        break ;
    case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
        break ;
    case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        break ;
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        break ;
    case SDL_EVENT_WINDOW_OCCLUDED:
        break ;
    case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
        break ;
    case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        break ;
    case SDL_EVENT_WINDOW_DESTROYED:
        app_->running_ = false ;
        break ;
    default:
        break ;
    }

}


void
push_quit_event()
{
    SDL_Event qe = { 0 } ;
    qe.type             = SDL_EVENT_QUIT ;
    qe.quit.type        = SDL_EVENT_QUIT ;
    qe.quit.timestamp   = SDL_GetTicks() ;
    SDL_PushEvent(&qe) ;
}



void
handle_quit_event(
    SDL_Event * event
)
{
    require(event) ;
    switch(event->type)
    {
    case SDL_EVENT_QUIT:
        app_->running_ = false ;
        break ;
    default:
        break ;
    }
}



void
handle_keyboard_event(
    SDL_Event * event
)
{
    require(event) ;
    SDL_KeyboardEvent * ke = NULL ;

    switch(event->type)
    {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
    case SDL_EVENT_TEXT_EDITING:
    case SDL_EVENT_TEXT_INPUT:
    case SDL_EVENT_KEYMAP_CHANGED:
    case SDL_EVENT_KEYBOARD_ADDED:
    case SDL_EVENT_KEYBOARD_REMOVED:
        ke = &event->key ;
        break ;
    default:
        break ;
    }

    if(!ke)
    {
        return ;
    }

    switch(ke->type)
    {
    case SDL_EVENT_KEY_DOWN:
        switch(ke->keysym.sym)
        {
        case SDLK_F4:
            if(ke->keysym.mod == SDL_KMOD_LALT)
            {
                push_quit_event() ;
            }
            break ;
        case SDLK_ESCAPE:
            push_quit_event() ;
            break ;
        default:
            break ;
        }
        break ;

    default:
        break ;
    }
}


void
handle_event(
    SDL_Event * event
)
{
    require(event) ;

    handle_window_event(event) ;
    handle_keyboard_event(event) ;
    handle_quit_event(event) ;
}


bool
run_app()
{
    begin_timed_block() ;

    app_->running_          = true ;
    app_->minimized_        = false ;
    app_->keyboard_focus_   = false ;

    for( ; app_->running_ ; )
    {
        bool const wait = app_->minimized_ || app_->keyboard_focus_ == false ;

        if(wait)
        {
            SDL_Event event ;
            if(check_sdl(SDL_TRUE == SDL_WaitEvent(&event)))
            {
                app_->running_ = false ;
                end_timed_block() ;
                return false ;
            }

            handle_event(&event) ;
        }
        else
        {
            SDL_Event event ;
            for( ; SDL_PollEvent(&event) ; )
            {
                handle_event(&event) ;
            }
        }

        if(!update_app())
        {
            app_->running_ = false ;
            end_timed_block() ;
            return false ;
        }
    }

    end_timed_block() ;
    return true ;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
int
main_app(
    int     argc
,   char *  argv[]
)
{
    app_->argv_ = argv ;
    app_->argc_ = argc ;

    if(check(create_app()))
    {
        destroy_app() ;
        return 1 ;
    }

    begin_timed_block() ;

    void * p = NULL ;

    if(check(p = malloc(23)))
    {
        end_timed_block() ;
        destroy_app() ;
        return 1 ;
    }


    if(check(create_vulkan()))
    {
        destroy_app() ;
        return 1 ;
    }

    hello_sdl3() ;
    hello_cglm() ;
    hello_vulkan() ;
    hello_debug() ;

    if(check(run_app()))
    {
        end_timed_block() ;
        destroy_app() ;
        return 1 ;
    }

    destroy_vulkan() ;

    end_timed_block() ;
    //dump_all_debug_counter_keepers() ;
    destroy_app() ;
    return 0 ;
}


void *
alloc_memory_impl(
    size_t const    byte_count
,   int const       clear_memory
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
)
{
    void * p = SDL_malloc(byte_count) ;
    require(p) ;
    if(!p)
    {
        log_output_impl(
            file
        ,   func
        ,   line
        ,   LOG_PRI_ERROR
        ,   "allocating memory failed! %s (%p)" "(" PRIu64 ") bytes (cleared=%d)"
        ,   expr
        ,   p
        ,   (uint64_t)byte_count
        ,   clear_memory
        ) ;

        return NULL ;
    }

    if(clear_memory)
    {
        SDL_memset(p, 0, byte_count) ;
    }

    log_output_impl(
        file
    ,   func
    ,   line
    ,   LOG_PRI_DEBUG
    ,   "allocating %s (%p)" "(" PRIu64 ") bytes (cleared=%d)"
    ,   expr
    ,   p
    ,   (uint64_t)byte_count
    ,   clear_memory
    ) ;

    return p ;
}


void *
alloc_array_impl(
    size_t const    count
,   size_t const    byte_size
,   int const       clear_memory
,   char const *    expr
,   char const *    file
,   char const *    func
,   int const       line
)
{
    require(count) ;
    require(byte_size) ;
    size_t const total = count * byte_size ;
    void * p = SDL_malloc(total) ;
    require(p) ;
    // if(!p)
    // {
    //     log_output_impl(
    //         file
    //     ,   func
    //     ,   line
    //     ,   LOG_PRI_ERROR
    //     ,   "allocating array failed! %s (%p) total " "(" PRIu64 ") bytes (cleared=%d)"
    //     ,   expr
    //     ,   p
    //     ,   (uint64_t)total
    //     ,   clear_memory
    //     ) ;

    //     return NULL ;
    // }

    if(clear_memory)
    {
        SDL_memset(p, 0, total) ;
    }

    log_output_impl(
        file
    ,   func
    ,   line
    ,   LOG_PRI_DEBUG
    ,   "allocating array type=%s (count=%u) (size=%u) (ptr=%p) (total bytes=%u) (cleared=%u)"
    ,   expr
    ,   (uint32_t)count
    ,   (uint32_t)byte_size
    ,   p
    ,   (uint32_t)total
    ,   (uint32_t)clear_memory
    ) ;

    return p ;
}
