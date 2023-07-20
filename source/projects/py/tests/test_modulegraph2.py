"""
BuiltinModule
FrozenModule
SourceModule
ExtensionModule
Package
"""
from argparse import Namespace

import modulegraph2

DEFAULT_EXCLUDES = [
   'test',
   'distutils',
   'setuptools',
   'unittest',
   'email',
]

class DepReport:
   def __init__(self, module, excludes=None):
      self.module = module
      self.mg = modulegraph2.ModuleGraph()
      if not excludes:
         excludes = DEFAULT_EXCLUDES
      self.mg.add_excludes(excludes)
      self.mg.add_module(module)
      self.nodes = []
      self.packages = set()
      self.packages_n = 0
      self.builtins = set()
      self.builtins_n = 0
      self.extensions = set()
      self.extensions_n = 0
      self.frozen = set()
      self.frozen_n = 0
      self.source = set()
      self.source_n = 0
      self.bytcode = set()    
      self.bytcode_n = 0
      self.namespace_packages = set()
      self.namespace_packages_n = 0            
      self.setup()

   def __repr__(self):
      return f"<DepReport '{self.module}' s:{self.n_source} p:{self.n_packages} b:{self.n_builtins} e:{self.n_extensions} f:{self.n_frozen}>"

   def outgoing_for(self, name):
      return list(i[1].name for i in self.mg.outgoing(self.mg.find_node(name)))

   def incoming_for(self, name):
      return list(i[1].name for i in self.mg.incoming(self.mg.find_node(name)))

   @property
   def outgoing(self):
      return list(i[1].name for i in self.mg.outgoing(self.nodes[0]))

   @property
   def incoming(self):
      return list(i[1].name for i in self.mg.incoming(self.nodes[0]))


   def setup(self):
      norm = lambda name: name.split('.')[0] if '.' in name else name

      self.nodes = list(self.mg.iter_graph()) 

      for i in self.nodes:
         if isinstance(i, modulegraph2.Package):
            self.packages.add(norm(i.name))
            self.n_packages += 1

         elif isinstance(i, modulegraph2.ExtensionModule):
            self.extensions.add(norm(i.name))
            self.n_extensions += 1

         elif isinstance(i, modulegraph2.BuiltinModule):
            self.builtins.add(norm(i.name))
            self.n_builtins += 1

         elif isinstance(i, modulegraph2.FrozenModule):
            self.frozen.add(norm(i.name))
            self.n_frozen += 1

         elif isinstance(i, modulegraph2.SourceModule):
            self.source.add(i.name)
            self.n_source += 1

         elif isinstance(i, modulegraph2.BytecodeModule):
            self.bytecode.add(i.name)
            self.n_bytecode += 1

         elif isinstance(i, modulegraph2.NamespacePackage):
            self.namespace_packages.add(i.name)
            self.n_namespace_packages += 1

         else:
            continue

if __name__ == '__main__':
   profile = False
   if not profile:
      d = DepReport('sqlite3')
   else:
      import cProfile
      import pstats

      cProfile.run("DepReport('sqlite3')", "depreport_stats")
      p = pstats.Stats("depreport_stats")
      p.sort_stats("cumulative").print_stats()

