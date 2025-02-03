#!/usr/bin/env python3

import os
from pathlib import Path
import shutil

CWD = Path.cwd()
DOCS_ROOT = Path(__file__).parent.parent
PROJECTS_DIR = DOCS_ROOT.parent / 'projects'
EXTERNALS_DIR = DOCS_ROOT / 'src' / 'externals'

assert PROJECTS_DIR.exists()

for p in PROJECTS_DIR.iterdir():
    if p.is_dir():
        dst = EXTERNALS_DIR /  f'{p.stem}.qmd'
        # print(f'{p.stem}.qmd')
        src = p / 'README.md'
        if dst.exists():
            dst.unlink()
        # dst.symlink_to(Path(f'../../../projects/{p.stem}/README.md'))
        src = PROJECTS_DIR / p.stem / 'README.md'
        try:
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
        except FileNotFoundError:
            print(f"failed: {dst}")
