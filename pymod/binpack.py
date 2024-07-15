from collections import defaultdict
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
from pymod import math
from sys import maxsize


def calc_min_max_size_info(sizes):
    assert(len(sizes) > 0)
    assert(len(sizes[0]) >= 2)
    min_w = maxsize
    min_h = maxsize
    min_a = maxsize
    max_w = 0
    max_h = 0
    max_a = 0
    total_w = 0
    total_h = 0
    total_a = 0

    for size in sizes:
        assert(len(size) >= 2)
        iw = size[0]
        ih = size[1]
        assert(iw > 0 and ih > 0)
        #if not (iw > 0 and ih > 0):
        #    continue
        ia = iw*ih
        min_w = min(min_w, iw)
        min_h = min(min_h, ih)
        min_a = min(min_a, ia)
        max_w = max(max_w, iw)
        max_h = max(max_h, ih)
        max_a = max(max_a, ia)
        total_w = total_w + iw
        total_h = total_h + ih
        total_a = total_a + ia

    return (total_w, total_h, total_a, min_w, min_h, min_a, max_w, max_h, max_a)


def calc_potential_bin_sizes(max_w, max_h, power_of_two):
    mw = max_w
    mh = max_h
    if power_of_two:
        mw = math.next_power_of_two(max_w)
        mh = math.next_power_of_two(max_h)
    mm = max(mw, mh)
    mw = mm
    mh = mm
    nw = mw
    nh = mh
    sizes = list()
    while True:
        if nw <= 1:
            break
        #print("nw=%d, nh=%d" % (nw, nh))
        sizes.append((nw, nh))
        if nh > 1:
            nh = nh >> 1
        # our algorithm below is shitty. We prefer wider to taller
        # so we smuggle in taller first, just so that wider comes later
        # which our algorithm will pick up.
        sizes.append((nh, nw))
        sizes.append((nw, nh))
        #print("nw=%d, nh=%d" % (nw, nh))
        nw = nw >> 1
    sizes.append((1, 1))
    #print(sizes)
    return sizes


def check_if_single_size_can_fit_bin(size, bin_size, allow_rotation):
    assert(len(size) >= 2)
    assert(len(bin_size) >= 2)
    if allow_rotation:
        norm = size[0] <= bin_size[0] and size[1] <= bin_size[1]
        rot  = size[1] <= bin_size[0] and size[0] <= bin_size[1]
        return (norm or rot)
    else:
        return size[0] <= bin_size[0] and size[1] <= bin_size[1]


def check_if_sizes_can_fit_bin(sizes, bin_size, allow_rotation):
    assert(len(sizes) > 0)
    assert(len(bin_size) >= 2)
    for size in sizes:
        if not check_if_single_size_can_fit_bin(size, bin_size, allow_rotation):
            #print("STOP rect cannot fit bin: %s, (%d, %d), bin_size: (%d, %d)" % (r.name_, r.pack_w_, r.pack_h_, bin_width, bin_height))
            return False
    return True


def have_unique_indices(sizes):
    assert(len(sizes) > 0)
    ids = list()
    for size in sizes:
        assert(len(size) >= 3)
        ids.append(size[2])
    ss = set(ids)
    return len(ss) == len(ids)


def have_contigues_indices(sizes):
    assert(len(sizes) > 0)
    assert(have_unique_indices(sizes))
    cl = sorted(sizes, key=lambda x: x[2], reverse=False)
    for index, size in enumerate(cl):
        if index != size[2]:
            return False
    return True


def re_index_sizes(sizes):
    assert(len(sizes) > 0)
    assert(have_unique_indices(sizes))
    re_map = dict()
    re_ind = list()
    for index, size in enumerate(sizes):
        assert(len(size) >= 3)
        oldidx = size[2]
        re_map[index] = oldidx
        size = (size[0], size[1], index)
        re_ind.append(size)
    assert(have_contigues_indices(re_ind))
    return (re_ind, re_map)


def re_map_rects(rects, re_map):
    assert(len(rects) > 0)
    assert(len(re_map) > 0)
    print("len(re_map)=%d, len(rects)=%d" % (len(re_map), len(rects)))
    assert(len(re_map) == len(rects))
    bins = defaultdict(list)
    for r in rects:
        b, x, y, w, h, i = r
        assert(i in re_map)
        i = re_map[i]
        bins[b].append((x, y, w, h, i, b))
    return bins


def pack_max_rects(sizes, bin_size, allow_rotation, algo):
    #assert(len(sizes) > 0)
    #assert(len(sizes[0]) >= 3)
    #assert(len(bin_size) >= 2)
    #assert(have_unique_indices(sizes))
    #assert(have_contigues_indices(sizes))
    #assert(check_if_sizes_can_fit_bin(sizes, bin_size, allow_rotation))
    packer = newPacker(mode=PackingMode.Offline, bin_algo=PackingBin.Global, pack_algo=algo, rotation=allow_rotation)
    packer.add_bin(bin_size[0], bin_size[1], count=float("inf"))
    #print("packing %s %s" % (str(len(sizes)), str(algo)))
    for size in sizes:
        packer.add_rect(size[0], size[1], size[2])
    packer.pack()
    #print("packer.rect_list=%d, bins=%d" % (len(packer.rect_list()), len(packer)))
    assert(len(sizes) == len(packer.rect_list()))
    return packer


def do_pack(sizes, allow_rotation, auto_bin_size=True, allow_shrinking=True, max_bin_size=(4096, 4096)):
    assert(len(sizes) > 0)
    assert(len(sizes[0]) >= 3)
    assert(len(max_bin_size) >= 2)
    assert(have_unique_indices(sizes))

    if auto_bin_size:
        mmsi = calc_min_max_size_info(sizes)
        max_bin_size = (mmsi[0], mmsi[1])

    assert(max_bin_size[0] > 0)
    assert(max_bin_size[1] > 0)
    # it has to fit in the largets bins if not then it's an error!
    assert(check_if_sizes_can_fit_bin(sizes, max_bin_size, allow_rotation))

    re_ind, re_map = re_index_sizes(sizes)
    assert(have_contigues_indices(re_ind))


    packer = None
    lbs = max_bin_size

    if allow_shrinking:
        bin_sizes = calc_potential_bin_sizes(max_bin_size[0], max_bin_size[1], True)
        result_list = list()

        pack_algo = [
            MaxRectsBl
        #,   MaxRectsBssf
        #,   MaxRectsBaf
        #,   MaxRectsBlsf
        ]

        for algo in pack_algo:
            lbs = max_bin_size
            one_more = 1
            cur_packer = None
            for bs in bin_sizes:
                print("Trying...<%d %d> (%s)" % (bs[0], bs[1], str(algo)))
                if not check_if_sizes_can_fit_bin(re_ind, bs, allow_rotation):
                    break
                cur_packer = pack_max_rects(re_ind, bs, allow_rotation, algo)
                # if we need multiple pages with max size, well then we can't go smaller anyway.
                if len(cur_packer) > 1 and bs[0] == max_bin_size[0] and bs[1] == max_bin_size[1]:
                    packer = cur_packer
                    lbs = bs
                    break
                # next round
                if len(cur_packer) == 1:
                    packer = cur_packer
                    lbs = bs
                    continue
                if len(cur_packer) > 1:
                    print("Trying...too small...<%d %d>" % (bs[0], bs[1]))
                    if one_more == 1:
                        one_more = 0
                        continue
                    break
            result_list.append((algo, packer, lbs, lbs[0]*lbs[1]))
            #print("len_rect_list=%d, len_packer=%d" % (len(result_list[len(result_list)-1][1].rect_list()), len(result_list[len(result_list)-1][1])))
        sorted_result_list = sorted(result_list, key=lambda x: x[3])
        packer = sorted_result_list[0][1]
        lbs = sorted_result_list[0][2]
    else:
        packer = pack_max_rects(re_ind, lbs, allow_rotation, MaxRectsBssf)

    bins = re_map_rects(packer.rect_list(), re_map)

    assert(bins)
    num_rects = 0
    for b in bins:
        num_rects = num_rects + len(bins[b])
    assert(len(bins) == len(packer))
    assert(num_rects == len(packer.rect_list()))

    print("packed %d rects in %d bins of [%d %d]" % (len(packer.rect_list()), len(packer), lbs[0], lbs[1]))
    return (bins, lbs)



# def do_pack(sizes, bin_size, allow_rotation):
#     assert(have_unique_indices(sizes))
#     packer = newPacker(mode=PackingMode.Offline, bin_algo=PackingBin.Global, pack_algo=MaxRectsBssf, rotation=allow_rotation)
#     packer.add_bin(bin_size[0], bin_size[1], count=float("inf"))

#     for size in sizes:
#         packer.add_rect(size[0], size[1], size[2])
#     packer.pack()

#     all_rects = packer.rect_list()
#     # for rect in all_rects:
#     #     b, x, y, w, h, rid = rect
#     #     print(rect)
#     all_rects_sorted = sorted(all_rects, key=lambda tup: tup[5])

#     return all_rects_sorted


