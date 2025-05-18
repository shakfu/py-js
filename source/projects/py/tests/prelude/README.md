# prelude

This file is the development version of the `py_prelude.py` python builtin file which is converted into a c-string and then preloaded into the namespace of every `py` instance. It includes some changes to facilitate testing using pytest.

Changes are made here first and fully tested and then copied to `py` project directory.


```python
def is_sequence(obj) -> bool
def is_iterable(obj) -> bool
def to_val(elem: Any, gdict: Optional[dict] = None) -> Any
def to_fn(s: str, gdict: Optional[dict] = None) -> Callable
def compose(f: Callable, g: Callable) -> Callable
def analyze(s: str, gdict: Optional[dict] = None) -> tuple[list[Callable], list[Any], list[tuple[Any, Any]]]
def list_to_dict(xs: list, eval_values=False) -> dict
def shell(cmd: str, err_func: Optional[Callable] = None) -> Optional[Any]
def out_dict(py_dict: dict) -> list
def pipe(s: str, gdict: Optional[dict] = None) -> Any
def call(s: str) -> Any
def fold(s: str, gdict: Optional[dict] = None) -> Any
def to_string(func, *args, **kwds) -> str
def from_list(xs: list[str]) -> tuple[Callable, tuple[Any, ...], dict[str, Any]]
def from_string(s: str) -> tuple[Callable, tuple[Any, ...], dict[str, Any]]
def apply(xs: list[Any]) -> Any
def edit(path: str) -> None
def product(*args) -> int | float
def sig(func) -> str
```