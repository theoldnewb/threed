import freetype


from PIL import Image
from os.path import isfile
from os.path import isdir
from os.path import join


def get_glyph_index_list_from_unicode_range(face, beg, end):
    gil = list()
    for i in range(beg, end+1):
        cc = i
        gi = face.get_char_index(cc)
        if 0 != gi:
            gil.append((gi,cc))
    return gil


def get_desired_glyph_list(face, desired_unicode_block_range_list):
    gils = list()
    for dr in desired_unicode_block_range_list:
        gil = get_glyph_index_list_from_unicode_range(face, dr[0], dr[1])
        if len(gil) > 0:
            gils = gils + gil
    return gils


def make_char_images(face, gils, is_verbose):
    chars = dict()
    face.load_glyph(face.get_char_index(32))
    slot = face.glyph
    if is_verbose:
        print("advance=%s left=%s" % (str(slot.advance.x), str(slot.bitmap_left)))
    chars[0] = (None, slot.advance.x, 0, 0)
    for x in gils:
        gi = x[0]
        cc = x[1]
        if is_verbose:
            print("gi=%s -> cc=%s" % (str(gi), str(cc)))
        face.load_glyph(gi)
        slot            = face.glyph
        bitmap          = slot.bitmap
        bitmap_pitch    = slot.bitmap.pitch
        bitmap_width    = slot.bitmap.width
        bitmap_rows     = slot.bitmap.rows
        bitmap_left     = slot.bitmap_left
        bitmap_top      = slot.bitmap_top
        #print("bitmap_pitch=%s" % str(bitmap_pitch))
        #print("bitmap_width=%s" % str(bitmap_width))
        #print("bitmap_rows=%s"  % str(bitmap_rows))
        #print("bitmap_left=%s"  % str(bitmap_left))
        #print("bitmap_top=%s"   % str(bitmap_top))
        assert(bitmap_pitch == bitmap_width)
        if bitmap_width == 0 or bitmap_rows == 0:
            if is_verbose:
                print("gi=%s cc=%s slot_adv=%s" % (str(gi), str(cc), str(slot.advance.x>>6)))
            continue

        img = Image.new('RGBA', (bitmap_width, bitmap_rows))
        for y in range(0, bitmap_rows):
            for x in range(0, bitmap_width):
                v = bitmap.buffer[y*bitmap_pitch+x]
                if 0 == v:
                    c = (0, 0, 0, v)
                else:
                    c = (255, 255, 255, v)
                img.putpixel((x,y), c)
        #fn = ("testfont/foo_%08X.png" % cc)
        #img.save(fn, "png")
        #break
        chars[cc] = (img, slot.advance.x, bitmap_left, bitmap_top)
    return chars


def make_font_images(font_file, font_size, desired_unicode_block_range_list, is_verbose):
    assert(font_size > 0)
    assert(len(desired_unicode_block_range_list) > 0)

    if is_verbose:
        print(freetype.version())

    face = freetype.Face(font_file)
    face.set_char_size(font_size * 64)

    gils = get_desired_glyph_list(face, desired_unicode_block_range_list)
    if is_verbose:
        print(gils)

    chars = make_char_images(face, gils, is_verbose)
    assert(chars)
    return (face, chars)


def make_demo_png(chars, demo_png, is_verbose):
    img = Image.new("RGBA", (1920, 1080))

    #text = 'Henlo, World!'
    text = 'WXYabcdefHIJKLMqpQrR~^°@|{=?}_+-*/Henlo, World. <[€]> <[(Ω)]> World!'
    pen_x = 10
    pen_y = 100

    for t in text:
        cc = ord(t)
        if cc not in chars:
            ii = chars[0]
            if is_verbose:
                print("space=%s" % (str(ii[1] >> 6)))
            pen_x = pen_x + (ii[1] >> 6)
            continue
        #assert(t in chars)
        ii = chars[cc]
        if is_verbose:
            print(ii)

        dst_x = pen_x + ii[2]
        dst_y = pen_y - ii[3]

        img.paste(ii[0], (dst_x, dst_y))
        pen_x = pen_x + (ii[1] >> 6)

    img.save(demo_png, "png")



def dump_chars(chars, outdir):
    assert(isdir(outdir))
    for k, v in chars.items():
        img = v[0]
        if img is None:
            continue

        name = ("f_%04x.png" % int(k))
        full = join(outdir, name)
        print(full)
        img.save(full, "png")
