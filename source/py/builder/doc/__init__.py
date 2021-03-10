from types import SimpleNamespace

import model
import yaml

with open('recipes/static-ext.yml') as f:
    content = f.read()

cfg = SimpleNamespace(**yaml.safe_load(content))

print(cfg)



r1 = model.Recipe(**cfg.__dict__)
