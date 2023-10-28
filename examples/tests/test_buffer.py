import api

# also see test_buffer_np.py for use of numpy/scipy with buffer

def test_view_buffer():
    # this step is necessary to get a reference to the `py` external instance
    ext = api.PyExternal()
    b = ext.get_buffer("drum") # b is a buffer instance

    # test it
    api.post(f"framecount: {b.framecount})")
    api.send("drum_scope", "set", "drum")
    b.view()


def test_create_buffer():
    name = "drum1"
    sample_file = "jongly.aif"
    buf = api.create_buffer(name, sample_file)
    api.post(f"created buffer name: '{name}' sample_file: '{sample_file}'")


def test_get_buffer():
    buf = api.get_buffer("drum1")
    api.post(f"framecount: {buf.framecount}")
    api.send("drum_scope1", "set", "drum1")
    buf.view()
    return buf.samplerate


def resize(buffer_name, to_framecount):
    buf = api.get_buffer(buffer_name)
    api.post(f"framecount: {buf.framecount}")
    buf.resize(int(to_framecount))
    return buf.framecount


def test_resize_buffer():
    buf = api.get_buffer("drum")
    api.post(f"framecount: {buf.framecount}")
    buf.resize(int(buf.framecount/2))
    # api.send("drum_scope", "set", "drum")
    buf.view()
    return buf.framecount
