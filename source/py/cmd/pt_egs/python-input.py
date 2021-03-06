#!/usr/bin/env python
"""
"""
from ptpython.python_input import PythonInput


def main():
    prompt = PythonInput(vi_mode=True)

    text = prompt.app.run()
    print("You said: " + text)


if __name__ == "__main__":
    main()
