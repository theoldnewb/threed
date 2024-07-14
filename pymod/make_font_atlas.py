
import os


#from pymod import packrect
from pymod import unicode_blocks
from pymod import misc


def run(raw_input_dir, asset_output_dir):
    print("raw_input_dir=%s, asset_output_dir=%s" % (raw_input_dir, asset_output_dir))
    assert(os.path.isdir(raw_input_dir))
    assert(os.path.isdir(asset_output_dir))
    src_dir = os.path.join(raw_input_dir, "gfx")
    print("src_dir=%s" % src_dir)
    assert(os.path.isdir(src_dir))
    dst_dir = os.path.join(asset_output_dir, "fonts")
    if not os.path.isdir(dst_dir):
        os.makedirs(dst_dir)
    assert(os.path.isdir(dst_dir))

    #def src(fs, ff):
    #    return (fs, ff)

    def dst(d):
        return os.path.join(dst_dir, d)

    is_verbose = True

    ubl = misc.load_unicode_blockstring(unicode_blocks.unicode_blocks_string, is_verbose)
    assert(ubl)
    for u in ubl:
        print(u)

    desired_unicode_blocks = [
        "Basic Latin"
    ,   "Latin-1 Supplement"
    ,   "Greek and Coptic"
    ,   "Currency Symbols"
    ,   "Cyrillic"
    ,   "Cyrillic Supplement"
    ]

    fubl = misc.filter_unicode_block_ranges(ubl, desired_unicode_blocks, is_verbose)

    font_file = "/usr/share/fonts/TTF/OpenSans-Bold.ttf"

    packrect.create_font(
        dst("test_font"), [
            (34, font_file)
        ]
    )

    out_file = "test.fnt"

    misc.make_and_pack_font(out_file, font_file, font_size, fubl, is_verbose)




# FF=/usr/share/fonts/gnu-free/FreeSansBold.otf
# FT=bold
# FS=34
# FL=latin
# OF=${ASSPATH}/fonts/freesans_${FT}_${FS}_${FL}.fnt

# "${PYTCODE}/generate_font.py" -v -f \
# -o "${OF}" \
# -ubf "${UBF}" \
# -ub "Basic Latin" "Latin-1 Supplement" "Currency Symbols" \
# -ub "Cyrillic" "Cyrillic Supplement" \
# -ff "${FF}" \
# -fs "${FS}"



