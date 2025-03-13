#!/usr/bin/env python3

import os
from pathlib import Path
import shutil

CWD = Path.cwd()
DOCS_ROOT = Path(__file__).parent.parent
PROJECTS_DIR = DOCS_ROOT.parent / 'projects'
PYJS_ROOT = DOCS_ROOT.parent.parent
SRC_DIR = DOCS_ROOT / 'src'
EXTERNALS_DIR = SRC_DIR / 'externals'


def update_file(src: Path, dst: Path):
    shutil.copy(src, dst)
    with open(dst) as f:
        newlines = []
        lines = f.readlines()
        for line in lines:
            if line.startswith("# "):
                title = line.lstrip("# ").strip()
                newlines.append("---\n")
                newlines.append(f'title: "{title}"\n')
                newlines.append("---\n")
            else:
                newlines.append(line)
    with open(dst, 'w') as f:
        f.writelines(newlines)
    
print(f"processing PYJS_ROOT: {PYJS_ROOT / 'README.md'} -> {SRC_DIR / 'overview.qmd'}")
update_file(PYJS_ROOT / 'README.md', SRC_DIR / 'overview.qmd')

print(f"processing FAQ: {PYJS_ROOT / 'FAQ.md'} -> {SRC_DIR / 'faq.qmd'}")
update_file(PYJS_ROOT / 'FAQ.md', SRC_DIR / 'faq.qmd')

assert PROJECTS_DIR.exists()

for p in PROJECTS_DIR.iterdir():
    if p.is_dir():
        dst = EXTERNALS_DIR /  f'{p.stem}.qmd'
        # print(f'{p.stem}.qmd')
        src = p / 'README.md'
        if dst.exists():
            dst.unlink()
        src = PROJECTS_DIR / p.stem / 'README.md'
        # print(f"processing {src} -> {dst}")
        print("upodating", dst.name)       
        try:
            update_file(src, dst)
        except FileNotFoundError:
            print(f"failed: {dst}")
