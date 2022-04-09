
import argparse
import os
import platform
import subprocess
import sys
import time
from pathlib import Path

from .config import (BASEDIR, BASELOGSDIR, CURRENT_PYTHON_VERSION, CYAN, GREEN,
                     HOME, LOGDIR, MAGENTA, PYJS_TARGETS, PYTHON_TARGETS,
                     RESET)
# from .ext.tqdm import tqdm
from .ext.pbar import ProgressBar


def cmd(shellcmd):
    os.system(shellcmd)

def run(shellcmd):
    return subprocess.run(
        shellcmd.split(), 
        stdout=subprocess.PIPE, 
        stderr=subprocess.STDOUT,
        text=True
    )


def proc(arglist):
    """iterate over a subprocess command

    >>> for line in proc(['ls']):
    ...:    print(line, end="")

    """
    _proc = subprocess.Popen(
        arglist, 
        stdout=subprocess.PIPE, 
        stderr=subprocess.STDOUT, 
        text=True)

    code = None

    while True:
        line = _proc.stdout.readline()
        code = _proc.poll()
        if line == '':
            if code != None:
                break
            else:
                continue
        yield line


def section(title):
    print(f"{MAGENTA}>>> {title} {RESET}")


def display_help():
    section("pyjs targets")

    for t in PYJS_TARGETS:
        print(f"{CYAN}make{RESET} {GREEN}{t:<20}{RESET} : {PYJS_TARGETS[t]['desc']}")

    print()
    section("python targets")

    for t in PYTHON_TARGETS:
        print(f"{CYAN}make{RESET} {GREEN}{t:<20}{RESET} : {PYTHON_TARGETS[t]}")
    print()


def cleaned(line):
    line = line.replace("[1;36m", "")
    line = line.replace("[m", "")
    line = line.replace(HOME, "~")
    return line.strip()

def runlog(target, requirement=2):
    cmd(f'mkdir -p {LOGDIR}')
    logfile = f"{LOGDIR}/{target}.log"

    print()
    print(f"running 'make {target}'")

    successes = 0

    with open(logfile, 'w') as f:
        
        # t = tqdm(total=PYJS_TARGETS[target]['lines'], desc=target)
        t = tqdm(total=PYJS_TARGETS[target]['lines'], desc=target)

        progressbar = ProgressBar(PYJS_TARGETS[target]['lines'], target)
        for line in proc(['make', '-C', str(BASEDIR), target]):
            print(cleaned(line), file=f)
            if "** BUILD SUCCEEDED **" in line:
                successes += 1
            # t.update(1)
            progressbar.update()
        # t.close()

    if successes == requirement:
        msg = f"{GREEN}SUCCESS{RESET}"
    else:
        msg = f"{MAGENTA}FAILURE{RESET}"

    print(f"  \u2514-> {msg}: {successes} out of {requirement} builds OK")
    print()
    # print(f"{target:<15} -> {msg}: {successes} out of {requirement} builds OK")


def run_logged_tests(target=None, with_homebrew=True):
    if target:
        runlog(t)
    else:
        for t in PYJS_TARGETS:
            if (not with_homebrew and t.startswith('homebrew')):
                continue
            runlog(t)


def open_log_dir():
    cmd(f"open {LOGDIR}")


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


def check_current(with_homebrew=True):
    logdir = Path(LOGDIR)
    for t in logdir.iterdir():
        if (not with_homebrew and t.startswith('homebrew')):
            continue
        check_success(t)


def check_all():
    check_logs(BASELOGSDIR)

