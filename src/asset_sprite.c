#include "asset_dump.h"
#include "asset_sprite.h"
#include "defines.h"
#include "log.h"
#include "app.h"
#include "check.h"


sprite_2d_ptr
load_asset_sprite(
    char const * const  fullname
)
{
    require(fullname) ;
    require(*fullname) ;

    sprite_2d_ptr ptr = { 0 } ;

    if(check(load_file((void **)&ptr.this_, &ptr.size_, fullname)))
    {
        require(0) ;
        return ptr ;
    }
    require(ptr.size_) ;
    require(ptr.this_) ;

    sprite_2d * p = ptr.this_ ;

    ptr.groups_   = asset_ref(rect_2d_group,    p, p->groups_offset_) ;
    ptr.vertices_ = asset_ref(rect_2d_vertices, p, p->vertices_offset_) ;
    ptr.infos_    = asset_ref(rect_2d_info,     p, p->infos_offset_) ;

    dump_sprite_2d(p) ;

    return ptr ;
}
