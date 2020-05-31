import os

from conftest import cwd, join

# content of test_sample.py
def func(x):
    return x + 1


def test_answer():
    assert func(3) == 4

# def test_system_echo(capfd):
#     os.system('echo "hello"')
#     captured = capfd.readouterr()
#     assert captured.out == "hello\n"

def test_me():
    py = join(cwd, 'test_py')
    print(py)
    assert 1==1


def test_system_echo(capfd):
    os.system(f'./tests/test_py import os')
    captured = capfd.readouterr()
    assert captured.out == "imported: os\n"

