"""
get tweaked / statically built components from beeware Python-Apple-support project

https://github.com/beeware/Python-Apple-support

"""
import os
from pathlib import Path

URL = ('https://github.com/beeware/Python-Apple-support/releases/'
      'download/{version}-{patch}/Python-{version}-macOS-support.{patch}.tar.gz')


VERSIONS = {
    '3.7' : 'b8',
    '3.8' : 'b7',
    '3.9' : 'b5',
    '3.10': 'b1',
}

def cmd(shellcmd):
    print(shellcmd)
    os.system(shellcmd)

def get_python_libs(destination_dir: str, version: str = '3.9'):
    """get tweaked / statically built components from beeware Python-Apple-support project

    see: https://github.com/beeware/Python-Apple-support
    """
    # checks
    assert version in VERSIONS, f"version {version} not available"
    patch = VERSIONS[version]
    dst_dir = Path(destination_dir)
    url = Path(URL.format(version=version, patch=patch))
    dst = dst_dir / url.name
    print(f"retrieving {url} to {dst}")
    if dst.exists():
        print("skipping download ...")
    else:
        cmd(f"curl -L --fail '{url}' -o '{dst}'")
    if not dst.exists():
        print(f"could not retrieve {url}")
        return
    print("unpacking...")
    tarfile = dst_dir / url.name
    cmd(f"tar -xvf '{tarfile}' --directory '{destination_dir}'")
    cmd(f"rm -f '{tarfile}'")
    print('DONE.')




