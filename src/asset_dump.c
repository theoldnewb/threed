#include "asset_dump.h"
#include "asset_sprite.h"
#include "defines.h"
#include "log.h"



static void
dump_rect_2d_vertices(
    rect_2d_vertices const * p
)
{
    require(p) ;
    log_debug_f32(p->p0x_) ;
    log_debug_f32(p->p0y_) ;
    log_debug_f32(p->t0u_) ;
    log_debug_f32(p->t0v_) ;
    log_debug_f32(p->p1x_) ;
    log_debug_f32(p->p1y_) ;
    log_debug_f32(p->t1u_) ;
    log_debug_f32(p->t1v_) ;
    log_debug_f32(p->p2x_) ;
    log_debug_f32(p->p2y_) ;
    log_debug_f32(p->t2u_) ;
    log_debug_f32(p->t2v_) ;
    log_debug_f32(p->p3x_) ;
    log_debug_f32(p->p3y_) ;
    log_debug_f32(p->t3u_) ;
    log_debug_f32(p->t3v_) ;
}


static void
dump_rect_2d_bounding_info(
    rect_2d_bounding_info const * p
)
{
    require(p) ;
    log_debug_u32(p->circle_offset_x_) ;
    log_debug_u32(p->circle_offset_x_) ;
    log_debug_u32(p->circle_radius_) ;
    log_debug_u32(p->circle_radius_squared_) ;

    log_debug_u16(p->crop_w_) ;
    log_debug_u16(p->crop_h_) ;
    log_debug_u16(p->w_) ;
    log_debug_u16(p->h_) ;
}


static void
dump_rect_2d_info(
    rect_2d_info const * p
)
{
    require(p) ;
    log_debug_u16(p->texture_index_) ;
    log_debug_u16(p->group_index_) ;
    log_debug_u16(p->pad1_) ;
    log_debug_u16(p->pad2_) ;
    dump_rect_2d_bounding_info(&p->bounding_info_) ;
}


static void
dump_sprite_2d_vertices(
    sprite_2d_vertices const * p
)
{
    require(p) ;

    log_debug_u16(p->vertices_count_) ;
    log_debug_u16(p->pad1_) ;
    log_debug_u16(p->pad2_) ;
    log_debug_u16(p->pad3_) ;

    log_debug_u16(p->pad4_) ;
    log_debug_u16(p->pad5_) ;
    log_debug_u16(p->pad6_) ;
    log_debug_u16(p->pad7_) ;

    for(
        uint16_t i = 0
    ;   i < p->vertices_count_
    ;   ++i
    )
    {
        log_debug_u16(i) ;
        dump_rect_2d_vertices(&p->vertices_[i]) ;
    }
}

static void
dump_sprite_2d_infos(
    sprite_2d_infos const * p
)
{
    require(p) ;

    log_debug_u16(p->infos_count_) ;
    log_debug_u16(p->pad1_) ;
    log_debug_u16(p->pad2_) ;
    log_debug_u16(p->pad3_) ;

    for(
        uint16_t i = 0
    ;   i < p->infos_count_
    ;   ++i
    )
    {
        log_debug_u16(i) ;
        dump_rect_2d_info(&p->infos_[i]) ;
    }
}

void
dump_sprite_2d(
    sprite_2d const * p
)
{
    require(p) ;

    log_debug_u16(p->tid_) ;
    log_debug_u16(p->pad1_) ;
    log_debug_u16(p->pad2_) ;
    log_debug_u16(p->pad3_) ;

    log_debug_u32(p->vertices_offset_) ;
    log_debug_u32(p->infos_offset_) ;

    sprite_2d_vertices *    sv = asset_ref(sprite_2d_vertices, p, p->vertices_offset_) ;
    sprite_2d_infos *       si = asset_ref(sprite_2d_infos, p, p->infos_offset_) ;


    // for(
    //     uint16_t i = 0
    // ;   i < p->group_count_
    // ;   ++i
    // )
    // {
    //     log_debug_u16(i) ;
    //     dump_sprite_2d_vertices(&sv[i]) ;
    //     dump_sprite_2d_infos(&si[i]) ;
    // }
}

