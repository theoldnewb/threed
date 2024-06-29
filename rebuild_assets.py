#!tpl/python/bin/python


#!/usr/bin/env python3

import os
import shutil
import subprocess
import pathlib
import argparse
from PIL import Image

SCRIPT_PATH=pathlib.Path(__file__).parent
#PYTHON_BIN="tpl/python/bin/python"

def main():
    #pybin = os.path.join(SCRIPT_PATH, PYTHON_BIN)
    print("SCRIPT_PATH=%s" % SCRIPT_PATH)
    #print("pybin=%s" % pybin)


if __name__ == "__main__":
    main()
