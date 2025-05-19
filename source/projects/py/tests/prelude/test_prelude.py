"""
def from_list(xs: list[str]) -> tuple[Callable, tuple[Any, ...], dict[str, Any]]
def from_string(s: str) -> tuple[Callable, tuple[Any, ...], dict[str, Any]]
def apply(xs: list[Any]) -> Any
def edit(path: str) -> None
def product(*args) -> int | float
def sig(func) -> str
"""
import ast
from typing import Any, Optional
import pytest
import prelude


@pytest.fixture
def containers():
    xlist = [1, 2, 3]
    xtuple = (1, 2, 3)
    xset = set(xlist)
    return (xlist, xtuple, xset)

@pytest.fixture
def gdict():
    def f1(*args, **kwds) -> int: return sum(args)
    def f2(x: int = 10) -> int: return x+1
    def f3(*args, **kwds): return sum(args)
    return dict(
        A_INT=100,
        A_FLOAT=12.5,
        FUNC=lambda x: 100,
        add=lambda x, y: x + y,
        f=lambda x: x + 100,
        g=lambda x: x * 2,
        f1 = f1,
        f2 = f2,
        f3 = f3,
    )

def test_is_sequence(containers):
    xlist, xtuple, xset = containers
    is_sequence = prelude.is_sequence
    assert prelude.is_sequence(xlist)
    assert prelude.is_sequence(xtuple)
    assert not prelude.is_sequence(xset)  # set is not a sequence

def test_is_iterable(containers):
    xlist, xtuple, xset = containers
    is_iterable = prelude.is_iterable
    assert prelude.is_iterable(xlist)
    assert prelude.is_iterable(xtuple)
    assert prelude.is_iterable(xset)

def test_to_val(gdict):
    to_val = prelude.__to_val
    assert to_val(1) == 1
    assert to_val(1.5) == 1.5
    assert to_val({"a": 1}) == {"a": 1}
    assert to_val((1, 2)) == (1, 2)
    assert to_val("1") == 1
    assert to_val("1.2") == 1.2
    assert to_val("A_INT", gdict) == gdict["A_INT"]
    assert to_val("A_FLOAT", gdict) == gdict["A_FLOAT"]
    assert to_val("FUNC", gdict) == gdict["FUNC"]

def test_to_fn(gdict):
    to_fn = prelude.__to_fn
    assert to_fn("add", gdict) == gdict["add"]
    assert to_fn("f", gdict) == gdict["f"]
    assert to_fn("g", gdict) == gdict["g"]

def test_compose(gdict):
    compose = prelude.compose
    assert compose(gdict["f"], gdict["g"])(10) == 120

def test_analyze(gdict):
    analyze = prelude.__analyze
    assert analyze("add 1 2 3", gdict) == ([gdict["add"]], [1, 2, 3], [])

def test_list_to_dict():
    assert prelude.list_to_dict(["a", ":", "1", "b", ":", "2", "3"]) == {
        "a": "1",
        "b": ["2", "3"],
    }
    assert prelude.list_to_dict(
        ["a", ":", "1", "b", ":", "2", "3"], eval_values=True) == {
        "a": 1, 
        "b": [2, 3],
    }

def test_shell():
    assert prelude.shell('echo "hello"') == "hello"

def test_dict_to_list():
    assert prelude.dict_to_list({'a': 1, 'b': [1,2,3,4]}) == [
        'a', ':', 1, 'b', ':', 1, 2, 3, 4]

def test_pipe(gdict):
    pipe = prelude.pipe
    assert pipe("10 f g", gdict) == 220
    assert pipe("f g 10 20 30", gdict) == [220, 240, 260]
    assert pipe("f g sum 10 20 30", gdict) == 720

def test_call():
    assert prelude.call("sum 1 2 3") == 6

def test_fold(gdict):
    fold = prelude.fold
    assert fold("add 1 2 3", gdict) == 6
    assert fold("add 1 2 3 4 5", gdict) == 15

def test_to_string():
    to_string = prelude.__to_string
    assert to_string("f2", 1, 2, 3, a=10, b=[1, 2]) == "f2 1 2 3 a : 10 b : 1 2"

def test_from_list(gdict):
    from_list = prelude.__from_list
    assert from_list(['f1', '1', '2', '3', 'a', ':', '5', '6', 'b', ':', '10'], gdict) == (
        gdict["f1"], (1, 2, 3), {"a": [5, 6], "b": 10}
    )

def test_from_string(gdict):
    from_string = prelude.__from_string
    assert from_string('f1 1 2 3 a : 5 6 b : 10', gdict) == (
        gdict["f1"], (1, 2, 3), {"a": [5, 6], "b": 10}
    )

def test_apply(gdict):
    assert prelude.apply('f3 1 2 3 a : 5 6 b : 10', gdict) == 6

def test_edit():
    prelude.edit("test_prelude.py")

def test_product():
    assert prelude.product(1, 2, 3, 4, 5) == 120
    assert prelude.product(1, 2, 3, 4, 5, 6) == 720

def test_sig(gdict):
    assert prelude.sig(gdict["f2"]) == "<function gdict.<locals>.f2(x: int = 10) -> int>"

def test_flatten():
    assert prelude.flatten([[1,2], [3,4], [5]]) == [1, 2, 3, 4, 5]

