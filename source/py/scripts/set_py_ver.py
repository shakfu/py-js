import os
import platform

env_file = os.getenv('GITHUB_ENV')
#env_file = 'env.log'

python_version = platform.python_version()

with open(env_file, "a") as fopen:
    fopen.write(f"PY_VERSION={python_version}")
