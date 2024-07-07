#include "asset_dump.h"
#include "asset_sprite.h"
#include "defines.h"
#include "log.h"
#include "app.h"
#include "check.h"



sprite_2d *
load_asset_sprite(
    char const * const  fullname
)
{
    require(fullname) ;
    require(*fullname) ;

    sprite_2d * p = NULL ;
    uint64_t    s = 0 ;

    if(check(load_file((void **)&p, &s, fullname)))
    {
        return NULL ;
    }

    dump_sprite_2d(p) ;

    return p ;
}
