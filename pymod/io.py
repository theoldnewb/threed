

import os
import io
import struct


def get_sorted_absolute_files_in_dir(src):
    assert(os.path.isdir(src))
    files = list()
    for f in os.listdir(src):
        rp = os.path.join(src, f)
        fp = os.path.abspath(rp)
        files.append(fp)
    return sorted(files)



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


