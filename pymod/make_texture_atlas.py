
import os


from pymod import texture_atlas


def run(raw_input_dir, asset_output_dir):
    print("raw_input_dir=%s, asset_output_dir=%s" % (raw_input_dir, asset_output_dir))
    assert(os.path.isdir(raw_input_dir))
    assert(os.path.isdir(asset_output_dir))
    src_dir = os.path.join(raw_input_dir, "gfx")
    print("src_dir=%s" % src_dir)
    assert(os.path.isdir(src_dir))
    dst_dir = os.path.join(asset_output_dir, "sprites")
    if not os.path.isdir(dst_dir):
        os.makedirs(dst_dir)
    assert(os.path.isdir(dst_dir))

    def src(s):
        return os.path.join(src_dir, s)

    def dst(d):
        return os.path.join(dst_dir, d)


    texture_atlas.create_animation(
        dst("cube"), [
            src("testing/cube/frames")
        ]
    )

    texture_atlas.create_animation(
        dst("suzanne"), [
            src("testing/suzanne/frames")
        ]
    )

    texture_atlas.create_image_collection(
        dst("patset"), [
            src("testing/patset/frames")
        ]
    )

    texture_atlas.create_animation(
        dst("test_cube_suzanne"), [
            src("testing/cube/frames")
        ,   src("testing/suzanne/frames")
        ]
    )



