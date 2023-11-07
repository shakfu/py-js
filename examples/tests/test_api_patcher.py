import api


def test_patcher_init():
    ext = api.PyExternal()
    p = ext.get_patcher()
    assert p.is_patcher(), "object is not a patcher!"


def test_patcher_misc():
    ext = api.PyExternal()
    p = ext.get_patcher()
    api.post(f"filepath: {p.get_filepath()}")

def test_patcher_attrs():
    ext = api.PyExternal()
    p = ext.get_patcher()
    # dump patch attributes
    # symbol attrs
    api.post(f"patcher.name: {p.name}")
    api.post(f"patcher.filepath: {p.filepath}")
    # int attrs
    api.post(f"patcher.fileversion: {p.fileversion}")
    api.post(f"patcher.count: {p.count}")
    api.post(f"patcher.fgcount: {p.fgcount}")
    api.post(f"patcher.bgcount: {p.bgcount}")
    api.post(f"patcher.numviews: {p.numviews}")
    api.post(f"patcher.numwindowviews: {p.numwindowviews}")
    # chars/bool attrs
    api.post(f"patcher.locked: {p.locked}")
    api.post(f"patcher.bglocked: {p.bglocked}")
    api.post(f"patcher.presentation: {p.presentation}")
    api.post(f"patcher.openinpresentation: {p.openinpresentation}")
    api.post(f"patcher.cansave: {p.cansave}")
    api.post(f"patcher.dirty: {p.dirty}")
    api.post(f"patcher.toolbarvisible: {p.toolbarvisible}")
    # array attr
    api.post(f"patcher.rect: {p.rect}")


def test_patcher_add_box():
    ext = api.PyExternal()
    p = ext.get_patcher()
    p.add_box("toggle", 240.0, 150.0)

def test_patcher_add_textbox():
    ext = api.PyExternal()
    p = ext.get_patcher()
    p.add_textbox("metro 400", 100.0, 150.0)

def test_patcher_add_tbox():
    ext = api.PyExternal()
    p = ext.get_patcher()
    p.add_tbox("buffer~ buf jongly.aif")

def test_patcher_newdefault():
    ext = api.PyExternal()
    p = ext.get_patcher()
    p.newdefault("var3", )

def test_patcher_connect():
    ext = api.PyExternal()
    p = ext.get_patcher()
    # p.assign("var1", "number", 1)
    # p.assign("var2", "number", 2)
    p.connect("var1", 0, "var2", 0)
