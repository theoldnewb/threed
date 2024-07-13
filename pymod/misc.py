

from pymod import asset_sprite
from pymod import packrect
from pymod import bounding_circle


def make_asset_rect_2d_vertices(r):
    a = asset_sprite.AssetRect2DVertices()
    a.p0x_ = r.ax_
    a.p0y_ = r.ay_
    a.t0u_ = r.au_
    a.t0v_ = r.av_
    a.p1x_ = r.bx_
    a.p1y_ = r.by_
    a.t1u_ = r.bu_
    a.t1v_ = r.bv_
    a.p2x_ = r.cx_
    a.p2y_ = r.cy_
    a.t2u_ = r.cu_
    a.t2v_ = r.cv_
    a.p3x_ = r.dx_
    a.p3y_ = r.dy_
    a.t3u_ = r.du_
    a.t3v_ = r.dv_
    return a


def make_asset_bounding_info(r):
    b = bounding_circle.BoundingCircle()
    b.add_pos(r.ax_, r.ay_)
    b.add_pos(r.bx_, r.by_)
    b.add_pos(r.cx_, r.cy_)
    b.add_pos(r.dx_, r.dy_)

    a = asset_sprite.AssetRect2DBoundingInfo()
    a.circle_offset_x_          = b.px()
    a.circle_offset_y_          = b.py()
    a.circle_radius_            = b.radius()
    a.circle_radius_squared_    = b.radius_squared()
    a.crop_w_                   = r.crop_size_[0]
    a.crop_h_                   = r.crop_size_[1]
    a.w_                        = r.get_image_size()[0]
    a.h_                        = r.get_image_size()[1]
    return a


def make_asset_rect_2d_info(r):
    a = asset_sprite.AssetRect2DInfo()
    a.texture_index_    = r.packed_bin_index_
    a.group_index_      = r.group_index_
    a.bounding_info_    = make_asset_bounding_info(r)
    return a


def find_min_max_crop_rect(v):
    assert(len(v) > 0)
    min_x = v[0].crop_rect_[0]
    min_y = v[0].crop_rect_[1]
    max_x = v[0].crop_rect_[2]
    max_y = v[0].crop_rect_[3]
    for r in v:
        min_x = min(min_x, r.crop_rect_[0])
        min_y = min(min_y, r.crop_rect_[1])
        max_x = max(max_x, r.crop_rect_[2])
        max_y = max(max_y, r.crop_rect_[3])
    assert(min_x >= 0)
    assert(min_x <= max_x)
    assert(max_x >= 0)
    assert(max_x >= min_x)
    assert(min_y >= 0)
    assert(min_y <= max_y)
    assert(max_y >= 0)
    assert(max_y >= min_y)
    return (min_x, min_y, max_x, max_y)


def make_asset_group_bounding_info(v):
    assert(len(v) > 0)
    b = bounding_circle.BoundingCircle()
    for r in v:
        b.add_pos(r.ax_, r.ay_)
        b.add_pos(r.bx_, r.by_)
        b.add_pos(r.cx_, r.cy_)
        b.add_pos(r.dx_, r.dy_)

    cr = find_min_max_crop_rect(v)

    a = asset_sprite.AssetRect2DBoundingInfo()
    a.circle_offset_x_          = b.px()
    a.circle_offset_y_          = b.py()
    a.circle_radius_            = b.radius()
    a.circle_radius_squared_    = b.radius_squared()
    a.crop_w_                   = 1 + cr[2] - cr[0]
    a.crop_h_                   = 1 + cr[3] - cr[1]
    a.w_                        = cr[2]
    a.h_                        = cr[3]
    return a


def make_asset_sprite_2d_group(v, frame_start, frame_count):
    a = asset_sprite.AssetSprite2DGroup()
    a.frame_start_      = frame_start
    a.frame_count_      = frame_count
    a.bounding_info_    = make_asset_group_bounding_info(v)
    return a

