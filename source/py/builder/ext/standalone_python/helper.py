"""
get a standalone python distribution from the python-build-standalone project

https://github.com/indygreg/python-build-standalone/releases

"""
import os
from pathlib import Path

BUILD_TYPES = ['pgo+lto', 'pgo', 'lto', 'install_only', 'debug']
ARCHS = ['x86_64', 'aarch64']

VERSIONS = {
    '3.8.13': {
        'release': '20220318',
        'archs': ARCHS,
        'build_types':  BUILD_TYPES,
    },
    '3.9.11': {
        'release': '20220318',
        'archs': ARCHS,
        'build_types':  BUILD_TYPES,
    },
    '3.10.3': {
        'release': '20220318',
        'archs': ARCHS,
        'build_types':  BUILD_TYPES,
    },
}

def cmd(shellcmd):
    print(shellcmd)
    os.system(shellcmd)

def get_python_url(version: str = '3.9.11', build_type: str = 'pgo+lto', 
                   arch: str = 'x86_64'):
    """gets a standalone build from the python build standalone project

    see: https://github.com/indygreg/python-build-standalone
    """
    # checks
    assert version in VERSIONS, f"version {version} not available"
    _version = VERSIONS[version]
    assert arch in _version['archs'], f"arch {arch } not available"
    assert build_type in _version['build_types'], f"build_type {build_type} not available"
    release = _version['release']

    if build_type in ['pgo+lto', 'pgo', 'lto', 'debug']:
        suffix = 'zst'
        build_type = f"{build_type}-full"
    else:
        suffix = 'gz'

    url = (f"https://github.com/indygreg/python-build-standalone/releases/download/"
           f"{release}/cpython-{version}+{release}-{arch}-apple-darwin-"
           f"{build_type}.tar.{suffix}")

    return url


def test_get_python_url():
    

    TESTS = [
        'cpython-3.10.3+20220318-x86_64-apple-darwin-debug-full.tar.zst',
        'cpython-3.10.3+20220318-x86_64-apple-darwin-install_only.tar.gz',
        'cpython-3.10.3+20220318-x86_64-apple-darwin-lto-full.tar.zst',
        'cpython-3.10.3+20220318-x86_64-apple-darwin-pgo+lto-full.tar.zst',
        'cpython-3.10.3+20220318-x86_64-apple-darwin-pgo-full.tar.zst',
    ]
    for bt in BUILD_TYPES:
        url = get_python_url(build_type=bt)
        archive = Path(url).name
        # print(archive)
        assert str(archive) in TESTS, f"{archive}"


def download_standalone_python(
        destination_dir: str, version: str = '3.9.11',
        arch: str = 'x86_64', build_type: str = 'pgo+lto',
        keep_archive=False):
    dst_dir = Path(destination_dir)
    url = Path(get_python_url(version, build_type, arch))
    dst = dst_dir / url.name
    print(f"retrieving {url} to {dst}")
    if dst.exists():
        print("skipping download ...")
    else:
        cmd(f"curl -L --fail '{url}' -o '{dst}'")
    if not dst.exists():
        print(f"could not retrieve {url}")
        return
    print("unpacking..")
    if dst.suffix == ".zst":
        tarfile = dst_dir / url.stem
        cmd(f"zstd -d '{dst}' -o '{tarfile}'")
        cmd(f"tar -xvf '{tarfile}' --directory '{destination_dir}'")
        cmd(f"rm -f '{tarfile}'")
    else:
        cmd(f"tar -xvf '{dst}' --directory '{destination_dir}'")
    if not keep_archive:
        cmd(f"rm -f '{dst}'")
    print('DONE.')




