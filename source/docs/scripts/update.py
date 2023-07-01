#!/usr/bin/env python3

import os
from pathlib import Path

CWD = Path.cwd()
DOCS_ROOT = Path(__file__).parent.parent
PROJECTS_DIR = DOCS_ROOT.parent / 'projects'
EXTERNALS_DIR = DOCS_ROOT / 'src' / 'externals'

assert PROJECTS_DIR.exists()

for p in PROJECTS_DIR.iterdir():
    if p.is_dir():
        dst = EXTERNALS_DIR /  f'{p.stem}.md'
        print(f'{p.stem}.md')
        src = p / 'README.md'
        dst.unlink()
        dst.symlink_to(Path(f'../../../projects/{p.stem}/README.md'))
