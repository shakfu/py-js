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


def test_maxobject_method_replace():
    memory["buf"].method("replace", "vibes-a1.aif")


def test_maxobject_method_open():
    memory["buf"].method("open")


def test_maxobject_method_close():
    memory["buf"].method("wclose")


def test_maxobject_delete():
    del memory["buf"]
