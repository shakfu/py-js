import math
from array import array

import api

def test_create_empty_buffer():
    name = "drum"
    duration_ms = 500
    buf = api.create_empty_buffer(name, duration_ms)
    api.post(f"created buffer name: '{name}' duration(ms): '{duration_ms}'")
    api.post(f"framecount: {buf.framecount}")


def test_buffer_get_samples():
    name = "drum1"
    sample_file = "jongly.aif"
    buf = api.create_buffer(name, sample_file)
    api.post(f"created buffer name: '{name}' sample_file: '{sample_file}'")
    n_samples = buf.n_samples
    xs = array('d', buf.get_samples())
    assert len(xs) == n_samples
    api.post(f"get {n_samples} samples from buffer {name}")


def linspace(start, stop, num=50, endpoint=True):
    """pure python version of np.linspace
    from: https://gist.github.com/pmav99/d124872c879f3e9fa51e
    """
    num = int(num)
    start = start * 1.
    stop = stop * 1.

    if num == 1:
        yield stop
        return
    if endpoint:
        step = (stop - start) / (num - 1)
    else:
        step = (stop - start) / num

    for i in range(num):
        yield start + step * i


def test_buffer_set_samples():
    name = "drum1"
    duration_ms = 500
    buf = api.create_empty_buffer(name, duration_ms)
    api.post("buffer.set_sample example with numpy and scipy.signal")
    api.post(f"created buffer name: '{name}' duration(ms): '{duration_ms}'")
    api.post(f"framecount: {buf.framecount}")
    n_samples = buf.n_samples
    t = linspace(0, 1, n_samples, endpoint=False)
    xs = array("d", [math.sin(i * 2 * math.pi * 5) for i in t])
    # xs = array("d", [math.sin(i) for i in t])
    buf.set_samples(xs)
    api.post(f"set {n_samples} samples to buffer {name}")
