#!/usr/bin/env python3

import os
import shutil
import subprocess

BUILD_DIR_NAME="build"
TPL_DIR="../tpl"
PRJ_DIR="../"
BUILD_CONFIG="Release"
SCRIPT_PATH=os.path.dirname(os.path.realpath(__file__))


def build_and_install(tpl_name):
    build_dir = os.path.join(SCRIPT_PATH, BUILD_DIR_NAME)
    tpl_dir = os.path.join(SCRIPT_PATH, TPL_DIR, tpl_name)
    print("build_dir=%s, tpl_dir=%s, tpl_name=%s" % (build_dir, tpl_dir, tpl_name))

    if os.path.isdir(build_dir):
        shutil.rmtree(build_dir)

    os.mkdir(build_dir)

    if os.path.isdir(tpl_dir):
        shutil.rmtree(tpl_dir)

    bldcfg = "-DCMAKE_BUILD_TYPE=%s" % BUILD_CONFIG

    print("bldcfg=%s" % bldcfg)

    subprocess.run(['cmake', bldcfg, '-S', tpl_name, '-B', build_dir])
    subprocess.run(['cmake', '--build', build_dir, '--config', BUILD_CONFIG, '--verbose'])
    subprocess.run(['cmake', '--install', build_dir, '--prefix', tpl_dir, '--verbose'])

    if os.path.isdir(build_dir):
        shutil.rmtree(build_dir)


def just_copy_dir(tpl_name):
    build_dir = os.path.join(SCRIPT_PATH, tpl_name)
    tpl_dir = os.path.join(SCRIPT_PATH, TPL_DIR, tpl_name)
    print("build_dir=%s, tpl_dir=%s, tpl_name=%s" % (build_dir, tpl_dir, tpl_name))
    if os.path.isdir(tpl_dir):
        shutil.rmtree(tpl_dir)

    shutil.copytree(build_dir, tpl_dir)


def create_python_venv(name):
    build_dir = os.path.join(SCRIPT_PATH, TPL_DIR, name)
    print("build_dir=%s" % build_dir)
    subprocess.run(['python', '-m', 'venv', build_dir])


def copy_threed_data(src_name, dst_name):
    src_dir = os.path.join(SCRIPT_PATH, src_name)
    dst_dir = os.path.join(SCRIPT_PATH, PRJ_DIR, dst_name)
    print("src=%s dst =%s" % (src_dir, dst_dir))
    shutil.copytree(src_dir, dst_dir)


def main():
    print("Hello, World!")
    print("SCRIPT_PATH=%s" % SCRIPT_PATH)
    build_and_install("SDL")
    build_and_install("cglm")
    just_copy_dir("stb")
    create_python_venv("python")
    copy_threed_data("threed_data/dat", "dat")

if __name__ == "__main__":
    main()



