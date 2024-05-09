#include "app.h"
#include "types.h"
#include "defines.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_version.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>


#include <cglm/mat4.h>
#include <cglm/io.h>


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
static app the_app_ ;
app * app_ = &the_app_ ;


static char const   window_title_[] = "threed" ;


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
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
    SDL_Log("compiled with: %u, %u, %u", compiled.major, compiled.minor, compiled.patch) ;
    SDL_Log("linking with: %u, %u, %u", linked.major, linked.minor, linked.patch) ;
    SDL_Log("SDL_GetRevision()=%s", SDL_GetRevision()) ;
}


bool
create_app()
{
    if(check_sdl(0 == SDL_Init(SDL_INIT_VIDEO)))
    {
        return  false ;
    }

    app_->window_width_     = 1280 ;
    app_->window_height_    = 768 ;
    SDL_assert(!app_->window_) ;

    app_->window_ = SDL_CreateWindow(
        window_title_
    ,   app_->window_width_
    ,   app_->window_height_
    ,   SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
    ) ;

    if(check_sdl(0 != app_->window_))
    {
        return false ;
    }

    return true ;
}


bool
update_app()
{
    return true ;
}


void
handle_window_event(
    SDL_Event * event
)
{
    SDL_assert(event) ;

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
        SDL_Log("Resized: [%d,%d]", we->data1, we->data2) ;
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
    SDL_assert(event) ;
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
    SDL_assert(event) ;
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
    SDL_assert(event) ;

    handle_window_event(event) ;
    handle_keyboard_event(event) ;
    handle_quit_event(event) ;
}


bool
run_app()
{
    app_->running_          = true ;
    app_->minimized_        = false ;
    app_->keyboard_focus_   = false ;

    for( ; app_->running_ ; )
    {
        int const wait = app_->minimized_ || app_->keyboard_focus_ == false ;

        if(wait)
        {
            SDL_Event event ;
            if(check_sdl(SDL_TRUE == SDL_WaitEvent(&event)))
            {
                app_->running_ = false ;
                break ;
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

        //if(check(update_app()))
        if(!update_app())
        {
            app_->running_ = false ;
            break ;
        }
    }

    return true ;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//
int
app_main(
    int     argc
,   char *  argv[]
)
{
    app_->argv_ = argv ;
    app_->argc_ = argc ;
    app_->exit_code_ = 0 ;



    SDL_Log("Hello, World!") ;
    hello_sdl3() ;
    hello_cglm() ;

    if(check(create_app()))
    {
        return 1 ;
    }

    if(check(run_app()))
    {
        return 1 ;
    }

    return 0 ;
}