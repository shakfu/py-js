import api



def test_bang():
	buf = api.get_buffer("drum")
	buf.bang()

def test_clear():
	buf = api.get_buffer("drum")
	buf.clear()

def test_apply():
	buf = api.get_buffer("drum")
	buf.apply("gain", "0.3")

def test_clearlow():
	buf = api.get_buffer("drum")
	buf.clearlow()

def test_crop():
	buf = api.get_buffer("drum")
	buf.crop(100, 10_000)

def test_duplicate():
	buf = api.get_buffer("drum")
	buf.duplicate("other") # duplicate from <other-buf>

def test_enumerate():
	buf = api.get_buffer("drum")
	buf.enumerate()

def test_fill():
    buf = api.get_buffer("drum")
    buf.fill("sin", 12)

def test_import1():
    buf = api.get_buffer("drum")
    buf.import_("drumLoop.aif")

def test_import2():
    buf = api.get_buffer("drum")
    buf.import_("drumLoop.aif", start=100)

def test_import3():
    buf = api.get_buffer("drum")
    buf.import_("drumLoop.aif", duration=1000)

def test_import4():
    buf = api.get_buffer("drum")
    buf.import_("drumLoop.aif", channels=1)

