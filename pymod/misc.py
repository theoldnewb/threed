import io

from pymod import asset_sprite
from pymod import packrect
from pymod import bounding_circle
from pymod import font


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
    a.crop_w_                   = r.crop_w_
    a.crop_h_                   = r.crop_h_
    a.w_                        = r.image_w_
    a.h_                        = r.image_h_
    return a


def make_asset_rect_2d_info(r):
    a = asset_sprite.AssetRect2DInfo()
    a.texture_index_    = r.packed_bin_index_
    a.group_index_      = r.group_index_
    a.bounding_info_    = make_asset_bounding_info(r)
    return a


def find_min_max_crop_rect(v):
    assert(len(v) > 0)
    min_x = v[0].crop_l_
    min_y = v[0].crop_t_
    max_x = v[0].crop_r_
    max_y = v[0].crop_b_
    for r in v:
        min_x = min(min_x, r.crop_l_)
        min_y = min(min_y, r.crop_t_)
        max_x = max(max_x, r.crop_r_)
        max_y = max(max_y, r.crop_b_)
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


def load_unicode_blockfile(blockfile, is_verbose):
    assert(isfile(blockfile))
    blocklist = list()
    with open(blockfile, "r") as bf:
        while True:
            line = bf.readline()
            if not line:
                break
            line_stripped = line.strip()
            if not line_stripped:
                continue
            if line_stripped.startswith('#'):
                continue
            line_splitted = line_stripped.split(';')
            assert(2 == len(line_splitted))
            lhs = line_splitted[0].strip()
            rhs = line_splitted[1].strip()
            lhs_split = lhs.split('..')
            assert(2 == len(lhs_split))
            beg = lhs_split[0].strip()
            end = lhs_split[1].strip()
            beg_int = int(beg, 16)
            end_int = int(end, 16)
            if is_verbose:
                print("beg=<%s><%x> end<%s><%x> desc=<%s>" % (beg, beg_int, end, end_int, rhs))
            blocklist.append((beg_int, end_int, rhs))
    assert(blocklist)
    return blocklist


def load_unicode_blockstring(blockstring, is_verbose):
    blocklist = list()
    bf = io.StringIO(blockstring)
    while True:
        line = bf.readline()
        if not line:
            break
        line_stripped = line.strip()
        if not line_stripped:
            continue
        if line_stripped.startswith('#'):
            continue
        line_splitted = line_stripped.split(';')
        assert(2 == len(line_splitted))
        lhs = line_splitted[0].strip()
        rhs = line_splitted[1].strip()
        lhs_split = lhs.split('..')
        assert(2 == len(lhs_split))
        beg = lhs_split[0].strip()
        end = lhs_split[1].strip()
        beg_int = int(beg, 16)
        end_int = int(end, 16)
        if is_verbose:
            print("beg=<%s><%x> end<%s><%x> desc=<%s>" % (beg, beg_int, end, end_int, rhs))
        blocklist.append((beg_int, end_int, rhs))
    assert(blocklist)
    return blocklist


def filter_unicode_block_ranges(unicode_blocklist, desired_ranges, is_verbose):
    assert(desired_ranges is not None)
    assert(len(desired_ranges) > 0)
    unique_ub = list(set(desired_ranges))

    ubl = list()
    for dr in unique_ub:
        dr_ub = None
        for ub in unicode_blocklist:
            if dr == ub[2]:
                dr_ub = ub
        if not dr_ub:
            if is_verbose:
                print("Range <%s> not found!" % dr)
            continue
        ubl.append(dr_ub)
    sorted_ubl = sorted(ubl)

    if is_verbose:
        print(sorted_ubl)

    return sorted_ubl


def make_font_pack_rects(chars):
    assert(len(chars))

    rects = dict()
    sizes = list()
    for k, v in chars.items():
        g = v[0]
        if g is None:
            continue
        pr = packrect.PackRect()
        pr.init_from_image(g)
        pr.set_index(k, 0)
        if pr.crop_size_[0] != pr.get_image_size()[0] or pr.crop_size_[1] != pr.get_image_size()[1]:
            print("crop differnce r.index_=%s(%x)" % (str(r.index_), r.index_))
        #assert(r.crop_w_ == r.img_w_)
        #assert(r.crop_h_ == r.img_h_)
        pr.set_border(1, 1, 1, 1)
        rects[k] = pr
        w, h = pr.get_pack_size()
        u = pr.unique_index_
        sizes.append((w, h, u))

    # allow_rotation  = False
    # auto_bin_size   = True
    # allow_shrinking = True
    # bins, lbs = do_pack(sizes, allow_rotation, auto_bin_size, allow_shrinking, (tw, th))

    # pack_rects = dict()
    # for b in bins:
    #     rect_list = list()
    #     for r in bins[b]:
    #         #print(r)
    #         x, y, w, h, i, b = r
    #         pr = rects[i]
    #         pr.set_packed(x, y, w, h, b, lbs[0], lbs[1])
    #         rect_list.append(pr)
    #     assert(len(rect_list) > 0)
    #     #pack_rects.append(rect_list)
    #     pack_rects[b] = rect_list

    # images  = make_font_bitmap_bins(pack_rects, flipped)

    # return (pack_rects, images)



def make_and_pack_font(outputfile, fontfile, fontsize, fubl, is_verbose):
    face, chars = font.make_font_images(fontfile, int(fontsize), fubl, is_verbose)
    assert(chars[0][0] is None)
    #outdir = "/home/tom/prj/led/ass/chars"
    #dump_chars(chars, outdir)
    make_font_pack_rects(chars)



    # tw          = 4096
    # th          = 4096
    # bl          = 1
    # bt          = 1
    # br          = 0
    # bb          = 0
    # prs, images = make_font_pack_rects(chars, tw, th, bl, bt, br, bb, flipped)
    # assert(len(images) == 1)
    # img = images[0]

    # outdir = dirname(outputfile)
    # if not isdir(outdir):
    #     makedirs(outdir)
    # if isfile(outputfile):
    #     remove(outputfile)

    # newout = outputfile + ".png"
    # img.save(newout, "PNG")

    # at_font = AtFont()
    # at_font.face_width_                 = int(face.max_advance_width)
    # at_font.face_height_                = int(face.height)
    # at_font.space_horizontal_advance_   = int(chars[0][1])
    # at_font.bitmaps_.add_bitmap(make_bitmap_from_image(img, False))

    # # fonts only have one texture page.
    # assert(len(prs) == 1)
    # prl = sorted(prs[0], key=lambda x: x.index_, reverse=False)
    # for pr in prl:
    #     hor_adv = chars[pr.index_][1]
    #     left = chars[pr.index_][2]
    #     top = chars[pr.index_][3]
    #     print("index=%s, seq=%s, left=%s, top=%s" % (str(pr.index_), str(pr.sequence_), str(left), str(top)))
    #     fr = make_font_rect(pr, hor_adv, left, top, flipped)
    #     at_font.characters_.add_font_rect(fr)
    # w = AssetWriter()
    # at_font.write(w)
    # w.save_as(outputfile)

