#!/usr/bin/env python3

import os
from fnmatch import fnmatch
from pathlib import Path
from typing import Callable, Union

Pathlike = Union[str, Path]
MatchFn = Callable[[Path], bool]
ActionFn = Callable[[Path], None]


PATTERNS = [
    "*.dylib",
    "*.so",
    "*.mxo/Contents/MacOS/*"
]


def walk(root: Pathlike, match_func: MatchFn, action_func: ActionFn, skip_patterns: list[str]):
    """general recursive walk from root path with match and action functions"""
    for root_, dirs, filenames in os.walk(root):
        _root = Path(root_)
        if skip_patterns:
            for skip_pat in skip_patterns:
                if skip_pat in dirs:
                    dirs.remove(skip_pat)
        for _dir in dirs:
            current = _root / _dir
            if match_func(current):
                action_func(current)

        for _file in filenames:
            current = _root / _file
            if match_func(current):
                action_func(current)


def glob_remove(root: str, patterns: list[str], skip_dirs: list[str]):
    """applies recursive glob remove using a list of patterns"""

    def match(entry: Path) -> bool:
        return any(fnmatch(entry, p) for p in patterns)

    def strip(entry: Path):
        print(f"stripping {entry}...")
        os.system(f"strip -x {entry}")

    walk(root, match_func=match, action_func=strip, skip_patterns=skip_dirs)


if __name__ == '__main__':
    glob_remove("externals", patterns=PATTERNS, skip_dirs=["*.git"])
