"""
This is pretty replicates how downloading and uncompressing
is down on `builder`.

If some of the the urls below give the same error
as before 'curl: (7) Couldn't connect to server'

Then change 'https' to 'http' and rerun.

"""

import os
from pathlib import Path

URLS = [
    'https:/sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz',
    'https:/www.openssl.org/source/openssl-1.1.1g.tar.gz',
    'http:/tukaani.org/xz/xz-5.2.5.tar.gz',
    'https:/www.python.org/ftp/python/3.8.9/Python-3.8.9.tgz',
]


def download(url):
    """download src using curl and tar.

    curl and tar are automatically available on mac platforms.
    """
    download_path = Path('downloads')
    src_path = Path('src')
    url = Path(url)

    # download
    if not download_path.exists():
        download_path.mkdir()
    dst = download_path / url.name
    if not dst.exists():
        print(f"downloading {url} to {dst}")
        os.system(f"curl -L --fail {url} -o {dst}")

    src = dst
    dst = src_path / url.stem.rstrip('.tar')

    # unpack
    if not src_path.exists():
        src_path.mkdir(parents=True, exist_ok=True)
    if not dst.exists():
        print(f"unpacking .. {src} -> {dst}")
        os.system(f"tar -xvf {src} --directory {src_path}")


for url in URLS:
    download(url)
