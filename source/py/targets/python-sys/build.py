import sysconfig
import os

VERSION = sysconfig.get_python_version()
PREFIX = sysconfig.get_config_var('prefix')

print(f'configuring python-sys xcode project for python {VERSION}')
print(f'prefix: {PREFIX}')

def cmd(shellcmd):
    print(shellcmd)
    os.system(shellcmd)

config = """
#include "../common.xcconfig"

VERSION = {version}
PREFIX = {prefix}

PY_HEADERS = $(PREFIX)/include/python$(VERSION)
PY_LIBS = $(PREFIX)/lib
PY_LDFLAGS = -lpython$(VERSION) -ldl
""".format(
    prefix = PREFIX,
    version = VERSION
)

with open('py-js.xcconfig', 'w') as f:
    f.write(config)


print("configuration DONE.")


print("running xcode")


for target in ['py', 'pyjs']:
    cmd(f'xcodebuild -project py-js.xcodeproj -target {target}')
