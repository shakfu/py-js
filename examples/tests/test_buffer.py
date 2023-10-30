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


def resize(buffer_name, frames):
    buf = api.get_buffer(buffer_name)
    api.post(f"framecount: {buf.framecount}")
    buf.set_framecount(frames)
    return buf.framecount

def test_change_buffer_framecount():
    buf = api.get_buffer("drum")
    api.post(f"framecount: {buf.framecount}")
    buf.set_framecount(buf.framecount/2)
    return buf.framecount

def test_change_duration():
    buf = api.get_buffer("drum")
    api.post(f"duration (secs): {buf.duration}")
    buf.set_duration(buf.duration/2)
    return buf.duration

def test_change_duration_ms():
    buf = api.get_buffer("drum")
    api.post(f"duration (ms): {buf.duration_ms}")
    buf.set_duration_ms(buf.duration_ms/2)
    return buf.duration_ms

def test_change_samplerate():
    buf = api.get_buffer("drum")
    buf.set_samplerate(22500)
    return buf.samplerate

def test_send():
    buf = api.get_buffer("drum")
    buf.send("fill", "sin", 24)

def test_fill():
    buf = api.get_buffer("drum")
    buf.fill("sin", 12)

def test_change():
    buf = api.get_buffer("drum")
    buf.change("sizeinsamps", 20000)

