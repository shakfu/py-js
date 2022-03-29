#!/usr/bin/env python3

"""
check count of xcode's '** BUILD SUCCEEDED **' in a logfile

usage:

   python3 scripts/check_success.py logs/xxx.log

"""
import argparse
import os
import sys
from pathlib import Path


GREEN="\033[1;32m"
MAGENTA="\033[1;35m"
RESET="\033[m"

HOME=os.environ['HOME']

def check_success(path: str, requirement: int = 2):
   """check count of xcode successful build message in a log file

   """
   successes = 0

   with open(path) as f:
      lines = f.readlines()
   for line in lines:
      if "** BUILD SUCCEEDED **" in line:
         successes += 1

   if successes == requirement:
      msg = f"{GREEN}SUCCESS{RESET}"
   else:
      msg = f"{MAGENTA}FAILURE{RESET}"

   cpath = str(path).replace(HOME, '~') # remove $HOME prefix and replace with '~'
   print(f"{cpath:<46} -> {msg}: {successes} out of {requirement} builds OK")
   return successes


def check_version(path: str, requirement: int = 2):
   n_entries = 0
   total_successes = 0
   version_folder = Path(path)
   print()
   for log in version_folder.iterdir():
      # print(log)
      n_entries += 1
      total_successes += check_success(log, requirement)
   # print(f"{version_folder.name:<6}: {GREEN}{total_successes}{RESET} out of {n_entries * 2} builds OK")
   # return total_successes, n_entries


def check_logs(path: str, requirement: int = 2):
   logs_folder = Path(path)
   for version_folder in logs_folder.iterdir():
      check_version(version_folder, requirement)


if __name__ == '__main__':
   parser = argparse.ArgumentParser(description='check success in xcode build logs.')
   parser.add_argument('--entry', action="store_true", help='check log entry')
   parser.add_argument('--version', action='store_true', help='check version log')
   parser.add_argument('--logs', action='store_true', help='check logs')
   parser.add_argument('path', help='path to target')

   args = parser.parse_args()
   if args.entry:
      check_success(args.path)
   if args.version:
      check_version(args.path)
   if args.logs:
      check_logs(args.path)

