#include "types.h"
#include "defines.h"
#include "debug.h"
#include "check.h"
#include "vulkan.h"

#include "vulkan_rob.h"
#include "vulkan_rob_sprite.h"



static bool
create_render_objects()
{
    begin_timed_block() ;

    {
        vulkan_render_object vr ;
        make_rob(&vr) ;
        create_vulkan_render_object(&vr) ;
    }

    // {
    //     vulkan_render_object vr ;
    //     make_rob_sprite(&vr) ;
    //     create_vulkan_render_object(&vr) ;
    // }

    end_timed_block() ;
    return true ;
}


static void
destroy_render_objects()
{
    begin_timed_block() ;

    end_timed_block() ;
}



int
create_gfx()
{
    begin_timed_block() ;

    if(check(create_vulkan()))
    {
        end_timed_block() ;
        return false ;
    }

    if(check(create_render_objects()))
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


void
destroy_gfx()
{
    begin_timed_block() ;

    destroy_render_objects() ;

    destroy_vulkan() ;

    end_timed_block() ;
}


int
draw_gfx()
{
    begin_timed_block() ;

    if(check(draw_vulkan()))
    {
        end_timed_block() ;
        return false ;
    }

    end_timed_block() ;
    return true ;
}


void
resize_gfx()
{
    begin_timed_block() ;

    resize_vulkan() ;

    end_timed_block() ;
}