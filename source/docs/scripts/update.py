#!/usr/bin/env python3

import os
from pathlib import Path
import shutil

class DocumentIntegrator:
    """Integrate project documents into the quarto documentation.
    
    This script is used to integrate project documents into the quarto documentation.
    It is used to update the `overview.qmd` file with the contents of the `README.md` file
    of the `py` external.

    It is also used to update the `faq.qmd` file with the contents of the `FAQ.md` file
    of the `py` external.
    
    It is also used to update the `externals.qmd` file with the contents of the `README.md`
    file of each external.
    """

    def __init__(self):
        self.cwd = Path.cwd()
        self.docs_root = Path(__file__).parent.parent
        self.projects_dir = self.docs_root.parent / 'projects'
        self.pyjs_root = self.docs_root.parent.parent
        self.src_dir = self.docs_root / 'src'
        self.externals_dir = self.src_dir / 'externals'

    def update_file(self, src: Path, dst: Path):
        """Update a file with the contents of another file."""
        shutil.copy(src, dst)
        with open(dst) as f:
            newlines = []
            lines = f.readlines()
            for line in lines:
                if line.startswith("# "):
                    title = line.lstrip("# ").strip()
                    newlines.append("---\n")
                    newlines.append(f'title: "{title}"\n')
                    newlines.append("---\n")
                else:
                    newlines.append(line)
        with open(dst, 'w') as f:
            f.writelines(newlines)

    def update_changelog(self, name: str, changelog_file: Path) -> list[str]:
        """Update the changelog file with the contents of the changelog file of the external."""
        with open(changelog_file) as f:
            lines = f.readlines()
            return lines

    def process(self):
        """Process the project documents."""
        print(f"processing PYJS_ROOT: {self.pyjs_root / 'README.md'} -> {self.src_dir / 'overview.qmd'}")
        self.update_file(self.pyjs_root / 'README.md', self.src_dir / 'overview.qmd')

        print(f"processing FAQ: {self.pyjs_root / 'FAQ.md'} -> {self.src_dir / 'faq.qmd'}")
        self.update_file(self.pyjs_root / 'FAQ.md', self.src_dir / 'faq.qmd')

        assert self.projects_dir.exists()
        # changelog = []
        for p in self.projects_dir.iterdir():
            if p.is_dir():
                dst = self.externals_dir /  f'{p.stem}.qmd'
                # print(f'{p.stem}.qmd')
                src = p / 'README.md'
                if dst.exists():
                    dst.unlink()
                src = self.projects_dir / p.stem / 'README.md'
                # print(f"processing {src} -> {dst}")
                print("updating", dst.name)       
                try:
                    self.update_file(src, dst)
                except FileNotFoundError:
                    print(f"failed: {dst}")

        #         changelog_file = p / 'CHANGELOG.md'
        #         if changelog_file.exists():
        #             changelog += self.update_changelog(p.stem, changelog_file)
        # if changelog:
        #     cl_file = self.externals_dir / 'CHANGELOG.qmd'
        #     with open(cl_file, 'w') as f:
        #         f.write("".join(changelog))        


if __name__ == "__main__":
    integrator = DocumentIntegrator()
    integrator.process()

