#!/usr/bin/env python3

import os
import shutil
import subprocess

BUILD_DIR_NAME="build"
TPL_DIR="../tpl"
BUILD_CONFIG="Debug"
SCRIPT_PATH=os.path.dirname(os.path.realpath(__file__))


def main():
    print("Hello, World!")
    print("SCRIPT_PATH=%s" % SCRIPT_PATH)
    build_dir = os.path.join(SCRIPT_PATH, BUILD_DIR_NAME)
    src_dir = SCRIPT_PATH
    print("build_dir=%s, src_dir=%s" % (build_dir, src_dir))

    bldcfg = "-DCMAKE_BUILD_TYPE=%s" % BUILD_CONFIG

    subprocess.run(['cmake', '--build', build_dir, '--config', BUILD_CONFIG, '--verbose'])

if __name__ == "__main__":
    main()
