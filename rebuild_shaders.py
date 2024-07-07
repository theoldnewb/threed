#!/usr/bin/env python3

import os
import shutil
import subprocess

SCRIPT_PATH=os.path.dirname(os.path.realpath(__file__))


def run_glslc(src, dst):
    print("Generate shader %s -> %s" % (src, dst))
    subprocess.run(['glslc', src, '-o', dst])


def main():
    print("Hello, World!")
    print("SCRIPT_PATH=%s" % SCRIPT_PATH)
    src_dir = os.path.join(SCRIPT_PATH, "src/shaders")
    dst_dir = os.path.join(SCRIPT_PATH, "ass/shaders")
    print("dst_dir=%s, src_dir=%s" % (dst_dir, src_dir))
    if not os.path.isdir(dst_dir):
        os.makedirs(dst_dir)
    assert(os.path.isdir(dst_dir))

    #src_file = src_dir / "shader.vert"
    #dst_file = dst_dir / "shader.vert"
    #print("dst_file=%s, src_file=%s" % (dst_file, src_file))

    def src(s):
        return os.path.join(src_dir, s)

    def dst(d):
        return os.path.join(dst_dir, d)

    def run(s):
        run_glslc(src(s), dst(s + ".spv"))


    run("shader.vert")
    run("shader.frag")
    run("shader_test.vert")
    run("shader_test.frag")
    run("sprite_shader.vert")
    run("sprite_shader.frag")
    run("sprite_animation_shader.vert")
    run("sprite_animation_shader.frag")



if __name__ == "__main__":
    main()
