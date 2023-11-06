import api


def test_binbuf_init():
    buf = api.Binbuf()


def test_binbuf_append():
    buf = api.Binbuf()
    buf.append("#N", "buffer~", "buf", "cello-f2.aif")
    buf.eval()

def test_binbuf_insert():
    buf = api.Binbuf()
    buf.insert("#N", "buffer~", "buf", "vibes-a1.aif")
    buf.eval()

def test_binbuf_add_text():
    buf = api.Binbuf()
    buf.add_text("#N buffer~ buf drumLoop.aif")
    buf.eval()

def test_binbuf_to_text():
    buf = api.Binbuf()
    buf.add_text("#N buffer~ buf jongly.aif")
    api.post(buf.to_text())
