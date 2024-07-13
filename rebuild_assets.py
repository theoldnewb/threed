#!tpl/python/bin/python

import os

from pymod import make_texture_atlas

def main():
    root_path = os.path.dirname(os.path.realpath(__file__))
    src_path = os.path.join(root_path, "dat")
    dst_path = os.path.join(root_path, "ass")
    if not os.path.isdir(dst_path):
        os.makedirs(dst_path)

    make_texture_atlas.run(src_path, dst_path)


if __name__ == "__main__":
    main()
