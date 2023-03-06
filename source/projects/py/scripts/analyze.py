import os
from pathlib import Path

def otool(name, path):
    # terminal codes
    WHITE = "\x1b[97;20m"
    RESET = "\x1b[0m"
    print('-'*79)
    print(f'{WHITE}{name} {path.stem} dependencies{RESET}')
    os.system(f'otool -L {path}')

def analyze():
    for d in Path.cwd().iterdir():
        if d.is_dir():
            if d.stem.endswith('-pkg'):
                py = d / 'externals' / 'py.mxo' / 'Contents' / 'MacOS' / 'py'
                pyjs = d / 'externals' / 'pyjs.mxo' / 'Contents' / 'MacOS' / 'pyjs'
            else:
                py = d / 'py.mxo' / 'Contents' / 'MacOS' / 'py'
                pyjs = d / 'pyjs.mxo' / 'Contents' / 'MacOS' / 'pyjs'
            otool(d.stem, py)
            otool(d.stem, pyjs)


if __name__ == '__main__':
    analyze()