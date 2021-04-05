#!/usr/bin/env python3

import shutil
import sys
from pathlib import Path


def copy(src, dst):
    """copy file or folders -- tries to be behave like `cp -rf`"""
    src, dst = Path(src), Path(dst)
    if dst.exists():
        dst = dst / src.name
    if src.is_dir():
        shutil.copytree(src, dst)
    else:
        shutil.copyfile(src, dst)

try:
    src, dst = sys.argv[1], sys.argv[2]
    copy(src, dst)
except ValueError:
    print("usage: pycp.py <src> <dst>")
