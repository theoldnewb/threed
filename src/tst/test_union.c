#include <stdio.h>
#include <errno.h>
#include <stdint.h>


#define log_u32(a)  printf("%s=%d\n", #a, a)


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


typedef struct rect_2d_info
{
    uint16_t    texture_index_ ;
    uint16_t    group_index_ ;
    uint16_t    pad1_ ;
    uint16_t    pad2_ ;

    uint32_t    offset_x_ ;
    uint32_t    offset_y_ ;

    uint32_t    radius_ ;
    uint32_t    radius_squared_ ;

    uint32_t    crop_w_ ;
    uint32_t    crop_h_ ;

    uint32_t    w_ ;
    uint32_t    h_ ;
} rect_2d_info ;


typedef struct sprite_2d_vertices
{
    uint16_t            vertices_count_ ;
    uint16_t            pad1_ ;
    uint16_t            pad2_ ;
    uint16_t            pad3_ ;

    uint16_t            pad4_ ;
    uint16_t            pad5_ ;
    uint16_t            pad6_ ;
    uint16_t            pad7_ ;

    rect_2d_vertices    vertices_[] ;
} sprite_2d_vertices ;


typedef struct sprite_2d_infos
{
    uint16_t            infos_count_ ;
    uint16_t            pad1_ ;
    uint16_t            pad2_ ;
    uint16_t            pad3_ ;

    rect_2d_info        infos_[] ;
} sprite_2d_infos ;


typedef struct sprite_2d
{
    uint16_t            tid_ ;
    uint16_t            group_count_ ;
    uint16_t            pad1_ ;
    uint16_t            pad2_ ;

    uint32_t            group_vertices_offset_ ;
    uint32_t            group_infos_offset_ ;

    //rect_2d_vertices  vertices_[] ;
    //sprite_2d_info    infos_[]
} sprite_2d ;


void
do_test()
{
    log_u32(sizeof(rect_2d_vertices)) ;
    log_u32(sizeof(rect_2d_info)) ;
    log_u32(sizeof(sprite_2d_vertices)) ;
    log_u32(sizeof(sprite_2d_infos)) ;
    log_u32(sizeof(sprite_2d)) ;

}



int
main(
    int     argc
,   char *  argv[]
)
{
    do_test() ;
    return 0 ;
}