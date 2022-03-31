
import platform
import subprocess
import time
import argparse
import os
import sys
from pathlib import Path

from tqdm import tqdm


HOME = os.environ['HOME']

# BASEDIR=f"{HOME}/.build_pyjs"
BASEDIR="."

BASELOGSDIR=f"{BASEDIR}/logs"

PYTHON_VERSION = platform.python_version()

LOGDIR=f"{BASELOGSDIR}/{PYTHON_VERSION}"

#ESCAPED_HOME=$(echo $HOME | sed 's_/_\\/_g')
ESCAPED_HOME = HOME.replace("/", "\\/")

# colors
YELLOW="\033[1;33m"
BLUE="\033[1;34m"
GREEN="\033[1;32m"
MAGENTA="\033[1;35m"
CYAN="\033[1;36m"
RESET="\033[m"



PYJS_TARGETS = {
    "default"           : "non-portable pyjs externals linked to your system",
    "homebrew-pkg"      : "portable package w/ pyjs (requires homebrew python)",
    "homebrew-ext"      : "portable pyjs externals (requires homebrew python)",
    "shared-pkg"        : "portable package with pyjs externals (shared)",
    "shared-ext"        : "portable pyjs externals (shared)",
    "static-ext"        : "portable pyjs externals (static)",
    "framework-pkg"     : "portable package with pyjs externals (framework)",
    "framework-ext"     : "portable pyjs externals (framework)",
    "relocatable-pkg"   : "portable package w/ more custom options (framework)",
    # "pymx"              : "non-portable alternative python3 externals (min-lib)",
}

PYTHON_TARGETS = {
    "python-shared"         : "minimal enable-shared python build",
    "python-shared-ext"     : "minimal enable-shared python build for externals",
    "python-shared-pkg"     : "minimal enable-shared python build for packages",
    "python-static"         : "minimal statically-linked python build",
    "python-framework"      : "minimal framework python build",
    "python-framework-ext"  : "minimal framework python build for externals",
    "python-framework-pkg"  : "minimal framework python build for packages",
    "python-relocatable"    : "custom relocatable python framework build",    
}

N_LINES = {
    "default"          : 210,
    "homebrew-pkg"     : 275,
    "homebrew-ext"     : 278,
    "shared-pkg"       : 18746,
    "shared-ext"       : 14652,
    "static-ext"       : 14064,
    "framework-pkg"    : 14842,
    "framework-ext"    : 14852,
    "relocatable-pkg"  : 409,
    "vanilla-ext"      : 18169,
    "vanilla-ext"      : 18244,
}


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


def display_menu():
    section("pyjs targets")

    for t in PYJS_TARGETS:
        print(f"{CYAN}make{RESET} {GREEN}{t:<20}{RESET} : {PYJS_TARGETS[t]}")

    print()
    section("python targets")

    for t in PYTHON_TARGETS:
        print(f"{CYAN}make{RESET} {GREEN}{t:<20}{RESET} : {PYTHON_TARGETS[t]}")


def cleaned(line):
    line = line.replace("\\[1;36m", "")
    line = line.replace("\\[m", "")
    line = line.replace(HOME, "~")
    return line


def runlog(target, requirement=2):
    cmd(f'mkdir -p {LOGDIR}')
    logfile = f"{LOGDIR}/{target}.log"

    print()
    print(f"running 'make {target}'")

    successes = 0

    with open(logfile, 'w') as f:
        t = tqdm(total=N_LINES[target]) # Initialise
        for line in proc(['make', target]):
            print(cleaned(line), end="", file=f)
            if "** BUILD SUCCEEDED **" in line:
                successes += 1
            t.update(1)
        t.close()

    if successes == requirement:
      msg = f"{GREEN}SUCCESS{RESET}"
    else:
      msg = f"{MAGENTA}FAILURE{RESET}"

    # print()
    print(f"{target:<20} -> {msg}: {successes} out of {requirement} builds OK")
    # print()

def runlog_all(with_homebrew=True):
    for t in PYJS_TARGETS:
        if (not with_homebrew and t.startswith('homebrew')):
            continue
        runlog(t)


def open_log_dir():
    os.system(f"open {LOGDIR}")


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



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='run logging tests.')
    parser.add_argument('--runlog-all',         action="store_true",  help='run all logged tests')
    parser.add_argument('--check-version-logs', action='store_true',  help='show logs results for current version')
    parser.add_argument('--check-all-logs',     action='store_true',  help='show logs results for all versions')
    parser.add_argument('--open-logs',          action='store_true',  help='check logs')
    parser.add_argument('--display-menu',       action='store_true',  help='display menu')
    parser.add_argument('--without-homebrew',   action='store_false', help='exclude homebrew entries')

    args = parser.parse_args()

    if args.runlog_all:
        runlog_all(with_homebrew=args.without_homebrew)

    elif args.check_version_logs:
        check_current(with_homebrew=args.without_homebrew)

    elif args.check_all_logs:
        check_all()

    elif args.open_logs:
        open_log_dir()

    elif args.display_menu:
        display_menu()

    else:
        parser.print_help()
        parser.exit()

