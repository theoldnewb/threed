#!/usr/bin/env python3

import os
import shutil
import subprocess
import pathlib

SCRIPT_PATH=pathlib.Path(__file__).parent


def run_glslc(src, dst):
    subprocess.run(['glslc', src, '-o', dst])


def main():
    print("Hello, World!")
    print("SCRIPT_PATH=%s" % SCRIPT_PATH)
    dst_dir = SCRIPT_PATH / "ass/shaders"
    src_dir = SCRIPT_PATH / "src/shaders"
    print("dst_dir=%s, src_dir=%s" % (dst_dir, src_dir))

    #src_file = src_dir / "shader.vert"
    #dst_file = dst_dir / "shader.vert"
    #print("dst_file=%s, src_file=%s" % (dst_file, src_file))

    run_glslc(src_dir / "shader.vert", dst_dir / "shader.vert.spv" )
    run_glslc(src_dir / "shader.frag", dst_dir / "shader.frag.spv" )
    run_glslc(src_dir / "sprite_shader.vert", dst_dir / "sprite_shader.vert.spv" )
    run_glslc(src_dir / "sprite_shader.frag", dst_dir / "sprite_shader.frag.spv" )
    run_glslc(src_dir / "sprite_animation_shader.vert", dst_dir / "sprite_animation_shader.vert.spv" )
    run_glslc(src_dir / "sprite_animation_shader.frag", dst_dir / "sprite_animation_shader.frag.spv" )



if __name__ == "__main__":
    main()
