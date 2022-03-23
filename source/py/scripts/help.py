
YELLOW="\033[1;33m"
BLUE="\033[1;34m"
GREEN="\033[1;32m"
MAGENTA="\033[1;35m"
CYAN="\033[1;36m"
RESET="\033[m"

def section(title):
    print(f"{MAGENTA}>>> {title} {RESET}")


pyjs_targets = {
    "(default)"         : "non-portable pyjs externals linked to your system",
    "homebrew-pkg"      : "portable package with pyjs (requires homebrew python)",
    "homebrew-ext "     : "portable pyjs externals (requires homebrew python)",
    "shared-pkg "       : "portable package with pyjs externals (shared)",
    "shared-ext"        : "portable pyjs externals (shared)",
    "static-ext"        : "portable pyjs externals (static)",
    "framework-pkg"     : "portable package with pyjs externals (framework)",
    "framework-ext"     : "portable pyjs externals (framework)",
    "relocatable-pkg"   : "portable package with more custom options (framework)",
    "pymx"              : "non-portable alternative python3 externals (min-lib)",
}

python_targets = {
    "python-shared"         : "minimal enable-shared python build",
    "python-shared-ext"     : "minimal enable-shared python build for externals",
    "python-shared-pkg"     : "minimal enable-shared python build for packages",
    "python-static"         : "minimal statically-linked python build",
    "python-static-full"    : "statically-linked python build",
    "python-framework"      : "minimal framework python build",
    "python-framework-ext"  : "minimal framework python build for externals",
    "python-framework-pkg"  : "minimal framework python build for packages",
    "python-relocatable"    : "custom relocatable framework python build",    
}

section("pyjs targets")

for t in pyjs_targets:
    print(f"{CYAN}make{RESET} {GREEN}{t:<20}{RESET} : {pyjs_targets[t]}")

print()
section("python targets")


for t in python_targets:
    print(f"{CYAN}make{RESET} {GREEN}{t:<20}{RESET} : {python_targets[t]}")
    