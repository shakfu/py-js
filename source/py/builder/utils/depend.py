"""depend: gather and resolve dylib depdencies.




"""
import os
import re
import shutil
import subprocess


class DependencyManager:
    """Aggreggates, copies dylib dependencies and fixed references.

    target: dylib to made relocatable
    frameworks_dir: where target dylib will be copied to with copied dependents
    exec_ref: back ref for executable or plugin
    """
    def __init__(self,
                 target,
                 frameworks_dir='build',
                 staticlibs_dir=None,
                 exec_ref='@loader_path/../Frameworks'):
        self.target = target
        self.frameworks_dir = frameworks_dir
        self.staticlibs_dir = staticlibs_dir
        self.exec_ref = exec_ref
        self.install_names = {}
        self.deps = []
        self.dep_list = []

    def is_valid_path(self, dep_path):
        return (dep_path == '' or dep_path.startswith('/opt/local/')
                or dep_path.startswith('/usr/local/')
                or dep_path.startswith('/User/'))

    def get_deps(self, target=None):
        if not target:
            target = self.target
        key = os.path.basename(target)
        self.install_names[key] = []
        result = subprocess.check_output(['otool', '-L', target])
        entries = [
            line.decode('utf-8').strip() for line in result.splitlines()
        ]
        for entry in entries:
            match = re.match(r'\s*(\S+)\s*\(compatibility version .+\)$',
                             entry)
            if match:
                path = match.group(1)
                (dep_path, dep_filename) = os.path.split(path)
                if self.is_valid_path(dep_path):
                    if dep_path == '':
                        path = os.path.join('/usr/local/lib', dep_filename)

                    dep_path, dep_filename = os.path.split(path)
                    item = (path, '@rpath/' + dep_filename)
                    self.install_names[key].append(item)
                    if path not in self.deps:
                        self.deps.append(path)
                        self.get_deps(path)

    def process_deps(self):
        for dep in self.deps:
            dep_path, dep_filename = os.path.split(dep)
            dest = os.path.join(self.frameworks_dir, dep_filename)
            self.dep_list.append([dep, '@rpath/' + dep_filename])

    def copy_dylibs(self):
        if not os.path.exists(self.frameworks_dir):
            os.mkdir(self.frameworks_dir)

        # cp target to frameworks_dir
        if os.path.dirname(self.target) != self.frameworks_dir:
            dest = os.path.join(self.frameworks_dir,
                                os.path.basename(self.target))
            shutil.copyfile(self.target, dest)
            os.chmod(dest, 0o644)
            cmdline = ['install_name_tool', '-id', self.exec_ref, dest]
            err = subprocess.call(cmdline)
            if err != 0:
                raise RuntimeError("Failed to change '{0}' '{1}'".format(
                    dest, self.exec_ref))

        # copy the rest
        for item in self.dep_list:
            orig_path, transformed = item
            dirname, dylib = os.path.split(orig_path)

            dest = os.path.join(self.frameworks_dir, dylib)

            if not os.path.exists(dest):
                shutil.copyfile(orig_path, dest)
                os.chmod(dest, 0o644)

    def change_install_names(self):
        for key in sorted(self.install_names.keys()):
            # print(key)
            # for i in self.install_names[key]:
            #     print('\t', i)
            # print()

            target = os.path.join(self.frameworks_dir, key)
            deps = self.install_names[key]
            for dep in deps:
                old, new = dep

                (old_name_path, old_name_filename) = os.path.split(old)
                if key == old_name_filename:
                    cmdline = ['install_name_tool', '-id', new, target]
                else:
                    cmdline = [
                        'install_name_tool', '-change', old, new, target
                    ]

                err = subprocess.call(cmdline)
                if err != 0:
                    raise RuntimeError(
                        "Failed to change '{0}' to '{1}' in '{2}".format(
                            old, new, target))

    def transform_exec(self, target):
        result = subprocess.check_output(['otool', '-L', target])
        entries = [
            line.decode('utf-8').strip() for line in result.splitlines()
        ]
        for entry in entries:
            match = re.match(r'\s*(\S+)\s*\(compatibility version .+\)$',
                             entry)
            if match:
                path = match.group(1)
                (dep_path, dep_filename) = os.path.split(path)
                if self.is_valid_path(dep_path):
                    if dep_path == '':
                        path = os.path.join('/usr/local/lib', dep_filename)

                    dep_path, dep_filename = os.path.split(path)

                    dest = os.path.join(self.exec_ref, dep_filename)
                    cmdline = [
                        'install_name_tool', '-change', path, dest, target
                    ]
                    subprocess.call(cmdline)

    def copy_staticlibs(self):
        if not self.staticlibs_dir:
            raise Exception("must set 'staticlibs_dir parameter")
        for i in self.deps:
            head, tail = os.path.split(i)
            name = tail.rstrip('.dylib')
            if '.' in name:
                name = os.path.splitext(name)[0] + '.a'
            static = os.path.join(head, name)
            exists = os.path.exists(static)
            if exists:
                shutil.copyfile(static, os.path.join(self.staticlibs_dir,
                                                     name))
            else:
                print("revise: not exists", static)

    def process(self):
        self.get_deps()
        self.process_deps()
        self.copy_staticlibs()
        self.copy_dylibs()
        self.change_install_names()
        self.transform_exec('./eg')
