

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


def make_asset_rect_2d_info(r):
    b = bounding_circle.BoundingCircle()
    b.add_pos(r.ax_, r.ay_)
    b.add_pos(r.bx_, r.by_)
    b.add_pos(r.cx_, r.cy_)
    b.add_pos(r.dx_, r.dy_)

    a = asset_sprite.AssetRect2DInfo()
    a.texture_index_     = r.packed_bin_index_
    a.group_index_       = r.group_index_
    a.offset_x_          = b.px()
    a.offset_y_          = b.py()
    a.radius_            = b.radius()
    a.radius_squared_    = b.radius_squared()
    a.crop_w_            = r.crop_size_[0]
    a.crop_h_            = r.crop_size_[1]
    a.w_                 = r.get_image_size()[0]
    a.h_                 = r.get_image_size()[1]
    return a

