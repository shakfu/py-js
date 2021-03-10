"""recipes: build recipes
"""

from ..models import Recipe
from .builders import (FrameworkPythonBuilder, SharedPythonBuilder,
                       StaticPythonBuilder)
from .projects import PythonProject

static_python = Recipe(
    "static_python", project=PythonProject, builders=[SharedPythonBuilder]
)

shared_python = Recipe(
    "shared_python", project=PythonProject, builders=[SharedPythonProject]
)

framework_python = Recipe(
    "framework_python", project=PythonProject, builders=[FrameworkPythonProject]
)
