
from rectpack import newPacker
from rectpack import PackingMode
from rectpack import PackingBin
from rectpack import MaxRectsBl
from rectpack import MaxRectsBssf
from rectpack import MaxRectsBaf
from rectpack import MaxRectsBlsf
from rectpack import SORT_AREA
from rectpack import SORT_PERI
from rectpack import SORT_SSIDE
from rectpack import SORT_LSIDE
from rectpack import SORT_RATIO
from rectpack import SORT_NONE


def do_pack(sizes, bin_size, allow_rotation):

    packer = newPacker(mode=PackingMode.Offline, bin_algo=PackingBin.Global, pack_algo=MaxRectsBssf, rotation=allow_rotation)
    packer.add_bin(bin_size[0], bin_size[1], count=float("inf"))

    for size in sizes:
        packer.add_rect(size[0], size[1], size[2])
    packer.pack()

    all_rects = packer.rect_list()
    # for rect in all_rects:
    #     b, x, y, w, h, rid = rect
    #     print(rect)
    return all_rects


