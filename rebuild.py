#!/usr/bin/env python3

import os
import shutil
import subprocess
import pathlib

BUILD_DIR_NAME="build"
TPL_DIR="../tpl"
BUILD_CONFIG="Debug"
SCRIPT_PATH=pathlib.Path(__file__).parent

def main():
    print("Hello, World!")
    print("SCRIPT_PATH=%s" % SCRIPT_PATH)
    build_dir = SCRIPT_PATH / BUILD_DIR_NAME
    src_dir = SCRIPT_PATH
    print("build_dir=%s, src_dir=%s" % (build_dir, src_dir))

    if build_dir.is_dir():
        shutil.rmtree(build_dir)

    bldcfg = "-DCMAKE_BUILD_TYPE=%s" % BUILD_CONFIG

    subprocess.run(['cmake', bldcfg, '-S', src_dir, '-B', build_dir])
    subprocess.run(['cmake', '--build', build_dir, '--config', BUILD_CONFIG, '--clean-first', '--verbose'])


if __name__ == "__main__":
    main()
