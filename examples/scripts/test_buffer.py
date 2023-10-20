import api


def test_buffer():
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

