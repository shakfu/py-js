import os
import platform

env_file = os.getenv('GITHUB_ENV')

python_version = platform.python_version()

with open(env_file, "a") as f:
    f.write(f"PY_VERSION={python_version}")

