


from pymod import asset_tid
from pymod import io


# typedef union rect_2d_vertices
# {
#     struct
#     {
#         float pxpytutv_16_[16] ;
#     } ;
#     struct
#     {
#         float p0x_ ;
#         float p0y_ ;
#         float t0u_ ;
#         float t0v_ ;
#         float p1x_ ;
#         float p1y_ ;
#         float t1u_ ;
#         float t1v_ ;
#         float p2x_ ;
#         float p2y_ ;
#         float t2u_ ;
#         float t2v_ ;
#         float p3x_ ;
#         float p3y_ ;
#         float t3u_ ;
#         float t3v_ ;
#     } ;
#     struct
#     {
#         float pxpytutv_0_[4] ;
#         float pxpytutv_1_[4] ;
#         float pxpytutv_2_[4] ;
#         float pxpytutv_3_[4] ;
#     } ;
#     struct
#     {
#         float pxpytutv_4_4_[4][4] ;
#     } ;
# } rect_2d_vertices ;
class AssetRect2DVertices:
    def __init__(self):
        self.p0x_ = 0
        self.p0y_ = 0
        self.t0u_ = 0
        self.t0v_ = 0
        self.p1x_ = 0
        self.p1y_ = 0
        self.t1u_ = 0
        self.t1v_ = 0
        self.p2x_ = 0
        self.p2y_ = 0
        self.t2u_ = 0
        self.t2v_ = 0
        self.p3x_ = 0
        self.p3y_ = 0
        self.t3u_ = 0
        self.t3v_ = 0

    def write(self, w):
        assert(w.check_alignment(16))
        w.f32(self.p0x_)
        w.f32(self.p0y_)
        w.f32(self.t0u_)
        w.f32(self.t0v_)
        w.f32(self.p1x_)
        w.f32(self.p1y_)
        w.f32(self.t1u_)
        w.f32(self.t1v_)
        w.f32(self.p2x_)
        w.f32(self.p2y_)
        w.f32(self.t2u_)
        w.f32(self.t2v_)
        w.f32(self.p3x_)
        w.f32(self.p3y_)
        w.f32(self.t3u_)
        w.f32(self.t3v_)
        assert(w.check_alignment(16))


# typedef struct rect_2d_bounding_info
# {
#     uint32_t    circle_offset_x_ ;
#     uint32_t    circle_offset_y_ ;
#     uint32_t    circle_radius_ ;
#     uint32_t    circle_radius_squared_ ;
#     uint16_t    crop_w_ ;
#     uint16_t    crop_h_ ;
#     uint16_t    w_ ;
#     uint16_t    h_ ;
# } rect_2d_bounding_info ;
class AssetRect2DBoundingInfo:
    def __init__(self):
        self.circle_offset_x_       = 0
        self.circle_offset_y_       = 0
        self.circle_radius_         = 0
        self.circle_radius_squared_ = 0
        self.crop_w_                = 0
        self.crop_h_                = 0
        self.w_                     = 0
        self.h_                     = 0

    def write(self, w):
        assert(w.check_alignment())
        w.u32(int(self.circle_offset_x_))
        w.u32(int(self.circle_offset_y_))
        w.u32(int(self.circle_radius_))
        w.u32(int(self.circle_radius_squared_))
        w.u32(self.crop_w_)
        w.u32(self.crop_h_)
        w.u32(self.w_)
        w.u32(self.h_)
        assert(w.check_alignment())


# typedef struct rect_2d_info
# {
#     uint16_t              texture_index_ ;
#     uint16_t              group_index_ ;
#     uint16_t              pad1_ ;
#     uint16_t              pad2_ ;
#     rect_2d_bounding_info bounding_info_ ;
# } rect_2d_info ;
class AssetRect2DInfo:
    def __init__(self):
        self.texture_index_ = 0
        self.group_index_   = 0
        self.bounding_info_ = None

    def write(self, w):
        assert(self.bounding_info_ is not None)
        assert(w.check_alignment())
        w.u16(self.texture_index_)
        w.u16(self.group_index_)
        w.u16(0)
        w.u16(0)
        self.bounding_info_.write(w)
        assert(w.check_alignment())

# typedef struct sprite_2d_group
# {
#     uint16_t                frame_start_ ;
#     uint16_t                frame_count_ ;
#     uint16_t                pad1_ ;
#     uint16_t                pad2_ ;
#     rect_2d_bounding_info   bounding_info_ ;
# } sprite_2d_group ;
class AssetSprite2DGroup:
    def __init__(self):
        self.frame_start_   = 0
        self.frame_count_   = 0
        self.bounding_info_ = None

    def write(self, w):
        assert(self.frame_count_ > 0)
        assert(self.bounding_info_ is not None)
        assert(w.check_alignment())
        w.u16(self.frame_start_)
        w.u16(self.frame_count_)
        w.u16(0)
        w.u16(0)
        self.bounding_info_.write(w)
        assert(w.check_alignment())


# typedef struct sprite_2d
# {
#     uint16_t            tid_ ;
#     uint16_t            groups_count_ ;
#     uint16_t            vertices_count_ ;
#     uint16_t            infos_count_ ;
#     uint32_t            groups_offset_ ;
#     uint32_t            vertices_offset_ ;
#     uint32_t            infos_offset_ ;
# } sprite_2d ;
class AssetSprite2D:
    def __init__(self):
        self.om_        = io.PtrOffsetMap()
        self.groups_    = io.AssetList()
        self.vertices_  = io.AssetList()
        self.infos_     = io.AssetList()

    def append_group(self, g):
        self.groups_.append(g)

    def append_vertices(self, v):
        self.vertices_.append(v)

    def append_info(self, i):
        self.infos_.append(i)

    def write_head(self, w):
        assert(len(self.groups_))
        assert(w.check_alignment())
        w.u16(asset_tid.atid_sprite)
        w.u16(len(self.groups_))
        w.u16(len(self.vertices_))
        w.u16(len(self.infos_))
        assert(w.check_alignment())
        w.u32(self.om_.get(self.groups_))
        w.u32(self.om_.get(self.vertices_))
        w.u32(self.om_.get(self.infos_))
        w.align()
        assert(w.check_alignment())

    def write_body(self, w, offset=0):
        assert(w.check_alignment())
        self.om_.set(self.groups_, w.relative_tell(offset))
        self.groups_.write(w)
        assert(w.check_alignment())
        w.align(16)
        self.om_.set(self.vertices_, w.relative_tell(offset))
        self.vertices_.write(w)
        assert(w.check_alignment())
        self.om_.set(self.infos_, w.relative_tell(offset))
        self.infos_.write(w)
        assert(w.check_alignment())

    def write(self, w):
        assert(w.check_alignment())
        w_head_pos = w.tell()
        self.write_head(w)
        self.write_body(w, w_head_pos)
        w_end_pos = w.tell()
        w.goto(w_head_pos)
        self.write_head(w)
        w.goto(w_end_pos)
        assert(w.check_alignment())
