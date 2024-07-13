#pragma once


#include "types.h"


typedef union rect_2d_vertices
{
    struct
    {
        float pxpytutv_16_[16] ;
    } ;

    struct
    {
        float p0x_ ;
        float p0y_ ;
        float t0u_ ;
        float t0v_ ;
        float p1x_ ;
        float p1y_ ;
        float t1u_ ;
        float t1v_ ;
        float p2x_ ;
        float p2y_ ;
        float t2u_ ;
        float t2v_ ;
        float p3x_ ;
        float p3y_ ;
        float t3u_ ;
        float t3v_ ;
    } ;

    struct
    {
        float pxpytutv_0_[4] ;
        float pxpytutv_1_[4] ;
        float pxpytutv_2_[4] ;
        float pxpytutv_3_[4] ;
    } ;

    struct
    {
        float pxpytutv_4_4_[4][4] ;
    } ;

} rect_2d_vertices ;


typedef struct rect_2d_bounding_info
{
    uint32_t    circle_offset_x_ ;
    uint32_t    circle_offset_y_ ;

    uint32_t    circle_radius_ ;
    uint32_t    circle_radius_squared_ ;

    uint32_t    crop_w_ ;
    uint32_t    crop_h_ ;

    uint32_t    w_ ;
    uint32_t    h_ ;
} rect_2d_bounding_info ;


typedef struct rect_2d_info
{
    uint16_t                texture_index_ ;
    uint16_t                group_index_ ;
    uint16_t                pad1_ ;
    uint16_t                pad2_ ;
    rect_2d_bounding_info   bounding_info_ ;
} rect_2d_info ;


typedef struct rect_2d_group
{
    uint16_t                frame_start_ ;
    uint16_t                frame_count_ ;
    uint16_t                pad1_ ;
    uint16_t                pad2_ ;
    rect_2d_bounding_info   bounding_info_ ;
} rect_2d_group ;


typedef struct sprite_2d
{
    uint16_t            tid_ ;
    uint16_t            groups_count_ ;
    uint16_t            vertices_count_ ;
    uint16_t            infos_count_ ;

    uint32_t            groups_offset_ ;
    uint32_t            vertices_offset_ ;
    uint32_t            infos_offset_ ;

    //sprite_2d_groups  groups_[] ;
    //rect_2d_vertices  vertices_[] ;
    //sprite_2d_info    infos_[] ;
} sprite_2d ;


typedef struct sprite_2d_ptr
{
    uint64_t            size_ ;
    sprite_2d *         this_ ;
    rect_2d_group *     groups_ ;
    rect_2d_vertices *  vertices_ ;
    rect_2d_info *      infos_ ;
} sprite_2d_ptr ;



sprite_2d_ptr
load_asset_sprite(
    char const * const  fullname
) ;
