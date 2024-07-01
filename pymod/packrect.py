import os

from PIL import Image
from pymod import io
from pymod import binpack


class PackRect:
    def __init__(self, file_name):
        self.init_from_file(file_name)
        self.init_      = True
        self.border_l_  = 0
        self.border_t_  = 0
        self.border_b_  = 0
        self.border_r_  = 0
        self.border_w_  = 0
        self.index_     = 0
        self.packed_l_  = 0
        self.packed_t_  = 0
        self.packed_w_  = 0
        self.packed_h_  = 0
        self.packed_b_  = 0
        self.bin_w_     = 0
        self.bin_h_     = 0
        self.ax_        = 0
        self.ay_        = 0
        self.bx_        = 0
        self.by_        = 0
        self.cx_        = 0
        self.cy_        = 0
        self.dx_        = 0
        self.dy_        = 0

        self.au_        = 0
        self.av_        = 0
        self.bu_        = 0
        self.bv_        = 0
        self.cu_        = 0
        self.cv_        = 0
        self.du_        = 0
        self.dv_        = 0


    def is_valid(self):
        return self.image_ is not None

    def init_from_file(self, file_name):
        self.file_name_ = file_name
        self.image_     = create_image(self.file_name_)
        self.crop_rect_ = find_crop_rect(self.image_)
        self.crop_size_ = calc_crop_size(self.crop_rect_)
        self.crop_img_  = self.image_.crop(self.crop_rect_)

    def get_image_size(self):
        assert(self.is_valid())
        return self.image_.size

    def set_index(self, index=0):
        self.index_ = index

    def set_border(self, left=0, top=0, right=0, bottom=0):
        assert(left >= 0)
        assert(top >= 0)
        assert(right >= 0)
        assert(bottom >= 0)
        self.border_l_ = left
        self.border_t_ = top
        self.border_r_ = right
        self.border_b_ = bottom

    def set_packed(self, pl, pt, pw, ph, pb):
        self.packed_l_  = pl
        self.packed_t_  = pt
        self.packed_w_  = pw
        self.packed_h_  = ph
        self.packed_b_  = pb

    def set_bin_size(self, w, h):
        self.bin_w_ = w
        self.bin_h_ = h

    def get_border_width(self):
        assert(self.is_valid())
        assert(self.border_l_ >= 0)
        assert(self.border_r_ >= 0)
        return self.border_l_ + self.border_r_

    def get_border_height(self):
        assert(self.is_valid())
        assert(self.border_t_ >= 0)
        assert(self.border_b_ >= 0)
        return self.border_t_ + self.border_b_

    def get_pack_size(self):
        assert(self.is_valid())
        w = self.crop_size_[0] + self.get_border_width()
        h = self.crop_size_[1] + self.get_border_height()
        return (w, h)

    def dump(self):
        print("index=%3d name=%s img_size=%s crop_rect=%s crop_size=%s pack_size%s" %
        (self.index_, self.file_name_, str(self.get_image_size()), str(self.crop_rect_), str(self.crop_size_), str(self.get_pack_size())))


    def calc_pos_uv(self):
        x = self.packed_l_ + self.border_l_
        y = self.packed_t_ + self.border_t_
        w = self.crop_size_[0]
        h = self.crop_size_[1]

        # a-----b
        # |     |
        # |     |
        # c-----d

        self.ax_ = x
        self.ax_ = y
        self.bx_ = x + w
        self.bx_ = y
        self.cx_ = x
        self.cy_ = y + h
        self.dx_ = x + w
        self.dy_ = y + h

        def norm(t, n, m):
            return t * n / m

        tex_range = 1.0

        self.au_ = norm(self.ax_, tex_range, self.bin_w_)
        self.av_ = norm(self.ay_, tex_range, self.bin_h_)
        self.bu_ = norm(self.bx_, tex_range, self.bin_w_)
        self.bv_ = norm(self.by_, tex_range, self.bin_h_)
        self.cu_ = norm(self.cx_, tex_range, self.bin_w_)
        self.cv_ = norm(self.cy_, tex_range, self.bin_h_)
        self.du_ = norm(self.dx_, tex_range, self.bin_w_)
        self.dv_ = norm(self.dy_, tex_range, self.bin_h_)



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


def create_image(file_name):
    img = Image.open(file_name)
    assert(img.mode == "RGBA")
    return img


def create_pack_rect(file_name):
    #print("file_name=%s" % file_name)
    assert(os.path.isfile(file_name))
    pr = PackRect(file_name)
    pr.set_border(1, 1, 0, 0)
    return pr



def fill_image(img, rgba):
    for y in range(0, img.size[1]):
        for x in range(0, img.size[0]):
            img.putpixel((x, y), rgba)

# def fill_image_rect(img, left, top, width, height, rgba):
#     for y in range(top, height):
#         for x in range(left, width):
#             img.putpixel((x, y), rgba)

def copy_pack_rect(img, pr):
    l = pr.packed_l_
    t = pr.packed_t_
    w = l + pr.packed_w_
    h = t + pr.packed_h_
    #print("l=%d, t=%d, w=%d, h=%d" % (l, t, w, h))
    for y in range(t, h):
        for x in range(l, w):
            img.putpixel((x, y), 0x00ff00ff)

    w = pr.crop_size_[0]
    h = pr.crop_size_[1]
    for y in range(0, h):
        src_y = y + pr.crop_rect_[1]
        dst_y = y + pr.packed_t_ + pr.border_t_
        for x in range(0, w):
            src_x = x + pr.crop_rect_[0]
            dst_x = x + pr.packed_l_ + pr.border_l_
            rgba = pr.image_.getpixel((src_x, src_y))
            img.putpixel((dst_x, dst_y), rgba)


def test_pack_rects(rects):
    sizes = list()

    bin_w = 2048
    bin_h = 1024
    bin_size = (bin_w, bin_h)

    for r in rects:
        w, h = r.get_pack_size()
        sizes.append( (w, h, r.index_) )

    all_rects = binpack.do_pack(sizes, bin_size)

    all_rects_sorted = sorted(all_rects, key=lambda tup: tup[5])
    for r in all_rects_sorted:
        b, x, y, w, h, i = r
        rects[i].set_packed(x, y, w, h, b)
        rects[i].set_bin_size(bin_w, bin_h)
        rects[i].calc_pos_uv()
        #print(r)

    dst_img = Image.new("RGBA", bin_size)
    #dst_img.paste(0xffff00ff, (0, 0, bin_w, bin_h))
    fill_image(dst_img, 0x00000000)
    for r in rects:
        #x = r.packed_l_ + r.border_l_
        #y = r.packed_t_ + r.border_t_
        #dst_img.paste(r.crop_img_, (x, y))
        copy_pack_rect(dst_img, r)
    dst_img.save("test.png", "PNG")

    for r in rects:
        print("%d" % r.index_)
        print("%f, %f, %f, %f" % (r.ax_, r.ay_, r.au_, r.av_) )
        print("%f, %f, %f, %f" % (r.bx_, r.by_, r.bu_, r.bv_) )
        print("%f, %f, %f, %f" % (r.cx_, r.cy_, r.cu_, r.cv_) )
        print("%f, %f, %f, %f" % (r.dx_, r.dy_, r.du_, r.dv_) )



def create_pack_rects(src_dir, dst_dir):
    print("src_dir=%s, dst_dir=%s" % (src_dir, dst_dir))
    assert(os.path.isdir(src_dir))
    assert(os.path.isdir(dst_dir))
    files = io.get_sorted_absolute_files_in_dir(src_dir)
    pack_rects = list()
    for index, file_name in enumerate(files):
        pr = create_pack_rect(file_name)
        pr.set_index(index)
        pack_rects.append(pr)

    for pr in pack_rects:
        pr.dump()

    test_pack_rects(pack_rects)

    return pack_rects




