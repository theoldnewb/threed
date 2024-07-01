

import os


def get_sorted_absolute_files_in_dir(src):
    assert(os.path.isdir(src))
    files = list()
    for f in os.listdir(src):
        rp = os.path.join(src, f)
        fp = os.path.abspath(rp)
        files.append(fp)
    return sorted(files)

