import api

from pathlib import Path

# fixture
def buf(name="drum"):
    return api.get_buffer(name)

# ----------------------------------------------------------------------

def test_bang():
    buf().bang()

def test_clear():
    buf().clear()

def test_apply():
    buf().apply("gain", "0.3")

def test_clearlow():
    buf().clearlow()

def test_crop():
    buf().crop(100, 10_000)

def test_duplicate():
    buf().duplicate("other") # duplicate from <other-buf>

def test_enumerate():
    buf().enumerate()

def test_fill():
    buf().fill("sin", 12)

def test_import1():
    buf().import_("drumLoop.aif")

def test_import2():
    buf().import_("drumLoop.aif", start=100)

def test_import3():
    buf().import_("drumLoop.aif", duration=1000)

def test_import4():
    buf().import_("drumLoop.aif", channels=1)

def test_importreplace():
    buf().importreplace("drumLoop.aif")

def test_rename():
    buf("drum").rename("drumx")
    buf("drumx").rename("drum")

def test_normalize():
    buf().normalize(0.4)

def test_open():
    buf().open()

def test_close():
    buf().close()

def test_printmodtime():
    buf().printmodtime()

def test_write():
    endings = [".wav", ".aiff", ".raw", ".flac"]
    name = "drum"
    buf = api.get_buffer(name)

    tmp = Path("/tmp")

    for ending in endings:
        p = tmp / f"{name}{ending}"
        buf.write(str(p))
        assert p.exists()

