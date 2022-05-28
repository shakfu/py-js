# ----------------------------------------------------------------------------
# Enumerations


def get_base_prefix(self):
    """gets the base prefix depending on the class"""

    if self.is_external:
        if isinstance(self, StaticExtBuilder):
            prefix = self.project.lib / self.product.name.lower()
            # /Users/sa/Downloads/projects/py-js/support/python3.9
            static_lib = self.project.lib / 'python-static' / 'lib' / self.project.python.staticlib  # type: ignore
            #
            # Does not need fix function

        if isinstance(self, SharedExtBuilder):
            prefix = self.project.lib / self.product.name.lower()
            # /Users/sa/Downloads/projects/py-js/support/python3.9

        if isinstance(self, FrameworkExtBuilder):
            prefix = self.project.lib / self.product.name.lower()
            # /Users/sa/Downloads/projects/py-js/support/python3.9

    if self.is_package:
        if isinstance(self, SharedPkgBuilder):
            prefix = self.project.lib / self.product.name.lower()
            # /Users/sa/Downloads/projects/py-js/support/python3.9

        if isinstance(self, FrameworkPkgBuilder):
            prefix = self.project.lib / self.product.name.lower()
            # /Users/sa/Downloads/projects/py-js/support/python3.9