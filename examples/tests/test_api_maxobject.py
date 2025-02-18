"""

The `api.Maxobject` extension class is a generic wrapper around `t_object` types

In this test, the global `memory` dict serves as a kind of module-scope namespace
which allows for a basic way to keep Maxobject instances alive for the duration
of the test, and can be used to manage the lifescycle of instances on a granular basis
"""

import api

memory = {}


def test_maxobject_init():
    memory["buf"] = api.MaxObject("buffer~", "buf", "jongly.aif")


def test_maxobject_from_str():
    memory["buf"] = api.MaxObject.from_str("buffer~", "buf jongly.aif")


def test_maxobject_call_replace():
    memory["buf"].call("replace", "vibes-a1.aif")


def test_maxobject_call_open():
    memory["buf"].call("open")


def test_maxobject_call_wclose():
    memory["buf"].call("wclose")


def test_maxobject_method_exists():
    if memory["buf"].method_exists("wclose"):
        api.post("wclose method exists")


def test_maxobject_delete():
    del memory["buf"]
