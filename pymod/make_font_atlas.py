

import os


from pymod import font_atlas
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


    # desired_unicode_blocks = [
    #     "Basic Latin"
    # ,   "Latin-1 Supplement"
    # ,   "Greek and Coptic"
    # ,   "Currency Symbols"
    # ,   "Cyrillic"
    # ,   "Cyrillic Supplement"
    # ]

    # ubl = misc.get_unicode_block_ranges(desired_unicode_blocks, False)

    # font_file = "/usr/share/fonts/TTF/OpenSans-Bold.ttf"
    # font_atlas.create_font(ubl, dst("test_font"), [
    #         (14, font_file)
    #     ,   (34, font_file)
    #     ]
    # )


    # latin_unicode_blocks = [
    #     "Basic Latin"
    # #,   "Latin-1 Supplement"
    # #,   "Latin Extended-A"
    # #,   "Latin Extended-B"
    # #,   "IPA Extensions"
    # ]

    # ubl_latin = misc.get_unicode_block_ranges(latin_unicode_blocks, False)

    # font_file = "/usr/share/fonts/noto/NotoSans-Bold.ttf"
    # font_atlas.create_font(ubl_latin, dst("test_font_noto"), [
    #         (64, font_file)
    #     #,   (12, font_file)
    #     #,   (16, font_file)
    #     #,   (22, font_file)
    #     #,   (32, font_file)
    #     #,   (44, font_file)
    #     ]
    # )


    cjk_unicode_blocks = [
        "Hiragana"
    ,   "Katakana"
    #,   "CJK Unified Ideographs Extension A"
    ,   "Enclosed CJK Letters and Months"
    ,   "CJK Compatibility"
    #    "CJK Unified Ideographs"
    #,   "Latin-1 Supplement"
    #,   "Latin Extended-A"
    #,   "Latin Extended-B"
    #,   "IPA Extensions"
    ]

    ubl_cjk = misc.get_unicode_block_ranges(cjk_unicode_blocks, False)

    font_file = "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc"
    font_atlas.create_font(ubl_cjk, dst("test_font_noto-cjk"), [
            (32, font_file)
        #,   (12, font_file)
        #,   (16, font_file)
        #,   (22, font_file)
        #,   (32, font_file)
        #,   (44, font_file)
        ]
    )



