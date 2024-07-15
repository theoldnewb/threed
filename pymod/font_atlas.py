import os
from collections import defaultdict

from PIL import Image
from pymod import font
from pymod import packrect
from pymod import binpack


def create_font(ubl, dst_font_file, font_size_name_list):
    dst_dir = os.path.dirname(dst_font_file)
    base_name = os.path.basename(dst_font_file)
    print("dst_dir=%s base_name=%s" % (dst_dir, base_name))

    sizes = list()
    all_prs = dict()
    for group_index, fi in enumerate(font_size_name_list):
        fs, fn = fi
        assert(fs > 0)
        assert(os.path.isfile(fn))

        face, chars = font.make_font_images(fn, int(fs), ubl, True)
        assert(chars[0][0] is None)

        for k, v in chars.items():
            g = v[0]
            if g is None:
                continue
            pr = packrect.PackRect()
            pr.init_from_image(g)
            pr.set_index(k, group_index)
            pr.set_border(1, 1, 0, 0)
            if pr.get_crop_size() != pr.get_image_size():
                pr.dump()
            w = pr.get_pack_size()[0]
            h = pr.get_pack_size()[1]
            i = pr.get_unique_index()
            assert(i not in all_prs)
            all_prs[i] = pr
            sizes.append((w, h, i))

    allow_rotation  = False
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
            packrect.copy_pack_rect(bin_img, pr)
            groups[pr.group_index_].append(pr)
        img_name = base_name + "_%d.png" % bidx
        atlas_name = os.path.join(dst_dir, img_name)
        bin_img.save(atlas_name, "PNG")

