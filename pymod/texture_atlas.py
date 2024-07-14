import os
#import operator
from collections import defaultdict

from PIL import Image

from pymod import io
from pymod import packrect
from pymod import binpack
from pymod import misc
from pymod import asset_sprite


def hello_texture_atlas():
    print("Hello from hello_texture_atlas")


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
    #col = 0xffff00ff
    col = 0x00000000
    for y in range(t, h):
        for x in range(l, w):
            img.putpixel((x, y), col)

    show_rotated = False
    if show_rotated:
        for y in range(t, h):
            for x in range(l, w):
                if pr.rotated_:
                    col = 0xFF0000FF
                else:
                    col = 0xFF00FF00
                img.putpixel((x, y), col)


    for y in range(0, pr.crop_h_):
        src_y = y + pr.crop_t_
        for x in range(0, pr.crop_w_):
            src_x = x + pr.crop_l_
            if pr.rotated_:
                dst_x = pr.packed_l_ + pr.border_t_ + pr.crop_h_ - 1 - y
                dst_y = pr.packed_t_ + pr.border_l_ + x
            else:
                dst_x = x + pr.packed_l_ + pr.border_l_
                dst_y = y + pr.packed_t_ + pr.border_t_

            rgba = pr.image_.getpixel((src_x, src_y))
            img.putpixel((dst_x, dst_y), rgba)



def pack_sprite_rects(pack_rects, dst_dir, create_animation):
    base_name = os.path.basename(dst_dir)
    print("base_name=%s" % base_name)

    atlas_name = os.path.join(dst_dir, base_name + ".png")
    sprf_name = os.path.join(dst_dir, base_name + ".sprf")

    sizes = list()
    all_prs = dict()
    for prs in pack_rects:
        if create_animation:
            all_crop_rects = list()
            for pr in prs:
                all_crop_rects.append(pr.get_crop_rect())
            min_max_crop_rect = packrect.find_min_max_crop_rect(all_crop_rects)
            for pr in prs:
                pr.set_min_max_crop_rect(min_max_crop_rect)
        else:
            for pr in prs:
                pr.set_min_max_crop_rect(None)

        for pr in prs:
            w = pr.get_pack_size()[0]
            h = pr.get_pack_size()[1]
            i = pr.get_unique_index()
            sizes.append((w, h, i))
            assert(i not in all_prs)
            all_prs[i] = pr

    allow_rotation  = True
    auto_bin_size   = True
    allow_shrinking = True
    bin_size        = (4096, 4096)
    bins, bs        = binpack.do_pack(sizes, allow_rotation, auto_bin_size, allow_shrinking, bin_size)

    groups = defaultdict(list)
    for bidx in bins:
        bin_img = Image.new("RGBA", bs)
        for r in bins[bidx]:
            x, y, w, h, i, b = r
            pr = all_prs[i]
            pr.set_packed(x, y, w, h, b, bs[0], bs[1])
            pr.calc_pos_uv()
            copy_pack_rect(bin_img, pr)
            groups[pr.group_index_].append(pr)
        img_name = base_name + "_%d.png" % bidx
        atlas_name = os.path.join(dst_dir, img_name)
        bin_img.save(atlas_name, "PNG")

    sorted_groups = defaultdict(list)
    for k, v in groups.items():
        #sorted_groups[k] = sorted(v, key=operator.attrgetter('local_index_'))
        sorted_groups[k] = sorted(v, key=lambda x: x.get_local_index())

    ass = asset_sprite.AssetSprite2D()
    frame_start = 0
    frame_count = 0
    for k, v in sorted_groups.items():
        #print("k=%s, len(v)=%d" % (str(k), len(v)))
        for x in v:
            x.dump()
            ass.append_vertices(misc.make_asset_rect_2d_vertices(x))
            ass.append_info(misc.make_asset_rect_2d_info(x))
            frame_count = frame_count + 1
        ass.append_group(misc.make_asset_sprite_2d_group(v, frame_start, frame_count - frame_start))
        frame_start = frame_count

    w = io.AssetWriter(8)
    ass.write(w)
    w.save_as(sprf_name)


def create_sprite_pack_rect(file_name, local_index, group_index):
    #print("file_name=%s" % file_name)
    assert(os.path.isfile(file_name))
    pr = packrect.PackRect()
    pr.init_from_file(file_name)
    pr.set_index(local_index, group_index)
    #pr.set_border(1, 1, 1, 1)
    pr.set_border(1, 1, 0, 0)
    return pr


def create_sprite_pack_rects(dst_dir, src_dirs, create_animation):
    print("src_dirs=%s, dst_dir=%s" % (src_dirs, dst_dir))
    for src_dir in src_dirs:
        assert(os.path.isdir(src_dir))
    if not os.path.isdir(dst_dir):
        os.makedirs(dst_dir)
    assert(os.path.isdir(dst_dir))

    pack_rects = list()
    for group_index, src_dir in enumerate(src_dirs):
        files = io.get_sorted_absolute_files_in_dir(src_dir)
        prs = list()
        for local_index, file_name in enumerate(files):
            pr = create_sprite_pack_rect(file_name, local_index, group_index)
            prs.append(pr)
        pack_rects.append(prs)

    pack_sprite_rects(pack_rects, dst_dir, create_animation)


def create_animation(dst_dir, src_dirs):
    create_sprite_pack_rects(dst_dir, src_dirs, True)


def create_image_collection(dst_dir, src_dirs):
    create_sprite_pack_rects(dst_dir, src_dirs, False)



