#!/usr/bin/env python3

import sys
from zipfile import PyZipFile
import argparse

def zipsrc(zipfile, srcdir, optlevel=-1):
    with PyZipFile(zipfile, "w", optimize=optlevel) as zipfp:
        zipfp.writepy(srcdir)

def main():
    parser = argparse.ArgumentParser(description='Create a zip archive of a python source directory')
    parser.add_argument('zipfile', help="path to the zip file to be created")
    parser.add_argument('srcdir', help="path to directory of python src files")
    parser.add_argument('--optlevel', help="optimization leve", type=int,
                        default=0, choices=list(range(-1, 3)))

    args = parser.parse_args()
    if (args.zipfile and args.srcdir):
        zipsrc(args.zipfile, args.srcdir, args.optlevel)
    else:
        parser.print_help()

if __name__=='__main__':
    main()
