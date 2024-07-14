

from PIL import Image
from pymod import math


def hello_packrect():
    print("Hello from packrect.")


def find_crop_rect(img):
    assert(img.size[0] > 0)
    assert(img.size[1] > 0)
    assert(img.mode == "RGBA")
    w = img.size[0]
    h = img.size[1]
    min_x = w+1
    min_y = h+1
    max_x = -1
    max_y = -1
    for y in range(0, h):
        for x in range(0, w):
            r, g, b, a = img.getpixel((x, y))
            if a != 0:
                min_x = min(min_x, x)
                min_y = min(min_y, y)
                max_x = max(max_x, x)
                max_y = max(max_y, y)
    assert(min_x >= 0)
    assert(min_x <= w)
    assert(min_x <= max_x)
    assert(max_x >= 0)
    assert(max_x <= w)
    assert(max_x >= min_x)
    assert(min_y >= 0)
    assert(min_y <= h)
    assert(min_y <= max_y)
    assert(max_y >= 0)
    assert(max_y <= h)
    assert(max_y >= min_y)
    return (min_x, min_y, max_x, max_y)


def calc_crop_size(crop_rect):
    assert(crop_rect[0] >= 0)
    assert(crop_rect[1] >= 0)
    assert(crop_rect[2] >= 0)
    assert(crop_rect[3] >= 0)
    assert(crop_rect[0] <= crop_rect[2])
    assert(crop_rect[1] <= crop_rect[3])
    assert(1 + crop_rect[2] - crop_rect[0] >= 0)
    assert(1 + crop_rect[3] - crop_rect[1] >= 0)
    return (1 + crop_rect[2] - crop_rect[0], 1 + crop_rect[3] - crop_rect[1])


def find_min_max_crop_rect(crop_rects):
    assert(len(crop_rects) > 0)
    min_x = crop_rects[0][0]
    min_y = crop_rects[0][1]
    max_x = crop_rects[0][2]
    max_y = crop_rects[0][3]
    for r in crop_rects:
        min_x = min(min_x, r[0])
        min_y = min(min_y, r[1])
        max_x = max(max_x, r[2])
        max_y = max(max_y, r[3])
    assert(min_x >= 0)
    assert(min_x <= max_x)
    assert(max_x >= 0)
    assert(max_x >= min_x)
    assert(min_y >= 0)
    assert(min_y <= max_y)
    assert(max_y >= 0)
    assert(max_y >= min_y)
    return (min_x, min_y, max_x, max_y)


class PackRect:


    def __init__(self):
        self.file_name_         = None
        self.image_             = None
        self.image_w_           = 0
        self.image_h_           = 0
        self.crop_l_            = 0
        self.crop_t_            = 0
        self.crop_r_            = 0
        self.crop_b_            = 0
        self.crop_w_            = 0
        self.crop_h_            = 0
        self.border_l_          = 0
        self.border_t_          = 0
        self.border_b_          = 0
        self.border_r_          = 0
        self.border_w_          = 0
        self.border_h_          = 0
        self.local_index_       = 0
        self.group_index_       = 0
        self.unique_index       = 0
        self.min_max_crop_rect_ = None
        self.pack_w_            = 0
        self.pack_h_            = 0
        self.packed_l_          = 0
        self.packed_t_          = 0
        self.packed_w_          = 0
        self.packed_h_          = 0
        self.packed_bin_index_  = 0
        self.packed_bin_w_      = 0
        self.packed_bin_h_      = 0
        self.rotated_           = False
        self.ax_                = 0
        self.ay_                = 0
        self.bx_                = 0
        self.by_                = 0
        self.cx_                = 0
        self.cy_                = 0
        self.dx_                = 0
        self.dy_                = 0
        self.au_                = 0
        self.av_                = 0
        self.bu_                = 0
        self.bv_                = 0
        self.cu_                = 0
        self.cv_                = 0
        self.du_                = 0
        self.dv_                = 0


    def init_from_image(self, image):
        assert(image is not None)
        assert(image.mode == "RGBA")
        self.image_ = image
        sz = self.image_.size
        cr = find_crop_rect(self.image_)
        cs = calc_crop_size(cr)
        self.image_w_   = sz[0]
        self.image_h_   = sz[1]
        self.crop_l_    = cr[0]
        self.crop_t_    = cr[1]
        self.crop_r_    = cr[2]
        self.crop_b_    = cr[3]
        self.crop_w_    = cs[0]
        self.crop_h_    = cs[1]
        self.pack_w_    = cs[0]
        self.pack_h_    = cs[1]


    def is_valid(self):
        return self.image_ is not None


    def init_from_file(self, file_name):
        img = Image.open(file_name)
        assert(img.mode == "RGBA")
        self.file_name_ = file_name
        self.init_from_image(img)


    def set_index(self, local_index, group_index):
        self.local_index_   = local_index
        self.group_index_   = group_index
        self.unique_index_  = str(self.group_index_) + "_" + str(self.local_index_)


    def set_border(self, left=0, top=0, right=0, bottom=0):
        assert(left >= 0)
        assert(top >= 0)
        assert(right >= 0)
        assert(bottom >= 0)
        self.border_l_  = left
        self.border_t_  = top
        self.border_r_  = right
        self.border_b_  = bottom
        self.border_w_  = left + right
        self.border_h_  = top + bottom
        self.pack_w_    = self.crop_w_ + self.border_w_
        self.pack_h_    = self.crop_h_ + self.border_h_


    def set_packed(self, pl, pt, pw, ph, pb, pbw, pbh):
        assert(self.is_valid())
        assert(pl >= 0)
        assert(pt >= 0)
        assert(pw > 0)
        assert(ph > 0)
        assert(pbw > 0)
        assert(pbh > 0)
        assert(self.pack_w_ > 0)
        assert(self.pack_h_ > 0)
        self.packed_l_          = pl
        self.packed_t_          = pt
        self.packed_w_          = pw
        self.packed_h_          = ph
        self.packed_bin_index_  = pb
        self.packed_bin_w_      = pbw
        self.packed_bin_h_      = pbh
        self.rotated_           = False
        w = self.pack_w_
        h = self.pack_h_
        if int(w) != int(h):
            self.rotated_ = int(w) == int(self.packed_h_) and int(h) == int(self.packed_w_)


    def set_min_max_crop_rect(self, min_max_crop_rect):
        self.min_max_crop_rect_ = min_max_crop_rect


    def get_image_size(self):
        assert(self.is_valid())
        return (self.image_w_, self.image_h_)


    def get_border_width(self):
        assert(self.is_valid())
        assert(self.border_l_ >= 0)
        assert(self.border_r_ >= 0)
        assert(self.border_w_ >= 0)
        assert(self.border_w_ == self.border_l_ + self.border_r_)
        return self.border_w_


    def get_border_height(self):
        assert(self.is_valid())
        assert(self.border_t_ >= 0)
        assert(self.border_b_ >= 0)
        assert(self.border_h_ >= 0)
        assert(self.border_h_ == self.border_t_ + self.border_b_)
        return self.border_t_ + self.border_b_


    def get_crop_rect(self):
        assert(self.is_valid())
        return (self.crop_l_, self.crop_t_, self.crop_r_, self.crop_b_)


    def get_crop_size(self):
        assert(self.is_valid())
        return (self.crop_w_, self.crop_h_)


    def get_pack_size(self):
        assert(self.is_valid())
        return (self.pack_w_, self.pack_h_)


    def get_local_index(self):
        assert(self.is_valid())
        return (self.local_index_)


    def get_group_index(self):
        assert(self.is_valid())
        return (self.group_index_)


    def get_unique_index(self):
        assert(self.is_valid())
        return (self.unique_index_)


    def get_pack_size_with_index(self):
        assert(self.is_valid())
        return (self.pack_w_, self.pack_h_, self.unique_index_)


    def dump(self):
        print("index=%s name=%s bin=%d, rotated=%s, img_size=%s crop_rect=%s crop_size=%s pack_size%s" %
        (self.unique_index_, self.file_name_, self.packed_bin_index_, str(self.rotated_), str(self.get_image_size()), str(self.get_crop_rect()), str(self.get_crop_size()), str(self.get_pack_size())))


    def calc_pos_uv(self):
        assert(self.is_valid())
        px = 0
        py = 0
        pw = self.crop_w_
        ph = self.crop_h_
        if self.min_max_crop_rect_ is not None:
            px = self.crop_l_ - self.min_max_crop_rect_[0]
            py = self.crop_t_ - self.min_max_crop_rect_[1]

        self.ax_ = px
        self.ay_ = py
        self.bx_ = px + pw
        self.by_ = py
        self.cx_ = px + pw
        self.cy_ = py + ph
        self.dx_ = px
        self.dy_ = py + ph

        tu = self.packed_l_ + self.border_l_
        tv = self.packed_t_ + self.border_t_
        tw = pw
        th = ph

        tax = tu
        tay = tv
        tbx = tu + tw
        tby = tv
        tcx = tu + tw
        tcy = tv + th
        tdx = tu
        tdy = tv + th

        if self.rotated_:
            tu = self.packed_l_ + self.border_t_
            tv = self.packed_t_ + self.border_l_
            tw = ph
            th = pw
            tax = tu + tw
            tay = tv
            tbx = tu + tw
            tby = tv + th
            tcx = tu
            tcy = tv + th
            tdx = tu
            tdy = tv

        tex_range = 1.0
        self.au_ = math.normalize(tax, tex_range, self.packed_bin_w_)
        self.av_ = math.normalize(tay, tex_range, self.packed_bin_h_)
        self.bu_ = math.normalize(tbx, tex_range, self.packed_bin_w_)
        self.bv_ = math.normalize(tby, tex_range, self.packed_bin_h_)
        self.cu_ = math.normalize(tcx, tex_range, self.packed_bin_w_)
        self.cv_ = math.normalize(tcy, tex_range, self.packed_bin_h_)
        self.du_ = math.normalize(tdx, tex_range, self.packed_bin_w_)
        self.dv_ = math.normalize(tdy, tex_range, self.packed_bin_h_)

