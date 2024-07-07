

import os
import io
import struct

from pymod import math


def get_sorted_absolute_files_in_dir(src):
    assert(os.path.isdir(src))
    files = list()
    for f in os.listdir(src):
        rp = os.path.join(src, f)
        fp = os.path.abspath(rp)
        files.append(fp)
    return sorted(files)


def make_u32(a,b,c,d):
    n = ord(a) << 24 | ord(b) << 16 | ord(c) << 8 | ord(d)
    #x = struct.pack("<I", n)
    return n

def make_u16(a, b):
    assert(a > -128 and a < 128)
    assert(b > -128 and b < 128)
    n = ((a & 0xFF) << 8) | (b & 0xFF)
    #x = struct.pack("<I", n)
    return n



#def write_u8(f, v):
#    x = struct.pack("<B", v)
#    f.write(x)


class AssetWriter:
    def __init__(self, alignment=8):
        assert(alignment == 16 or alignment == 8 or alignment == 4 or alignment == 2)
        self.io_    = io.BytesIO()
        self.align_ = alignment

    def save_as(self, fn):
        with open(fn, "wb") as f:
            f.write(self.io_.getbuffer())

    def u8(self, v):
        self.io_.write(struct.pack("<B", v))

    def u16(self, v):
        self.io_.write(struct.pack("<h", v))

    def u32(self, v):
        self.io_.write(struct.pack("<I", v))

    def f32(self, v):
        self.io_.write(struct.pack("<f", v))

    def f64(self, v):
        self.io_.write(struct.pack("<d", v))

    def u64(self, v):
        self.io_.write(struct.pack("<Q", v))

    def tell(self):
        return self.io_.tell()

    def relative_tell(self, v):
        t = self.io_.tell()
        s = t - v
        assert(s >= 0)
        assert(s <= t)
        return s

    def check_alignment(self, align=None):
        assert(self.align_ == 16 or self.align_ == 8 or self.align_ == 4 or self.align_ == 2)
        if align is None:
            align = self.align_
        assert(align == 16 or align == 8 or align == 4 or align == 2)
        return math.is_aligned(self.tell(), align)

    def goto(self, v):
        assert(v >= 0 and v <= self.io_.getbuffer().nbytes)
        self.io_.seek(v)


class PtrOffsetMap:
    def __init__(self):
        self.offsets_ = dict()

    def get(self, pob):
        if pob:
            if pob in self.offsets_:
                return self.offsets_[pob]
        return 0

    def set(self, pob, offset):
        if pob:
            if pob in self.offsets_:
                o = self.offsets_[pob]
                assert(o == 0 or o == offset)
            self.offsets_[pob] = offset

