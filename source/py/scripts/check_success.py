#!/usr/bin/env python3

"""
check count of xcode's '** BUILD SUCCEEDED **' in a logfile

usage:

   python3 scripts/check_success.py logs/xxx.log

"""

import sys

GREEN="\033[1;32m"
MAGENTA="\033[1;35m"
RESET="\033[m"

def check_success(path: str, requirement:int = 2):
   """check count of xcode successful build message in a log file

   """
   count = 0
   with open(path) as f:
      lines = f.readlines()
   for line in lines:
      if "** BUILD SUCCEEDED **" in line:
         count += 1

   if count == requirement:
      msg = f"{GREEN}SUCCESS{RESET}"
   else:
      msg = f"{MAGENTA}FAILURE{RESET}"

   print(f"{path:<26} -> {msg}: {count} out of {requirement} builds succeeded")



if __name__ == '__main__':
   if len(sys.argv) >= 2:
      check_success(sys.argv[1])
