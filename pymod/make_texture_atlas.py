
import os

from pymod import packrect


def make_texture_atlas(src_dir, dst_dir):
    packrect.create_pack_rects(src_dir, dst_dir)


def run(raw_input_dir, asset_output_dir):
    print("raw_input_dir=%s, asset_output_dir=%s" % (raw_input_dir, asset_output_dir))
    assert(os.path.isdir(raw_input_dir))
    assert(os.path.isdir(asset_output_dir))

    def src(s):
        return os.path.join(raw_input_dir, s)

    def dst(d):
        return os.path.join(asset_output_dir, d)

    make_texture_atlas(src("suzanne/frames"), dst("suzanne"))

