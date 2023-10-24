import api

import numpy as np
from scipy import signal


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
    xs = np.array(buf.get_samples())
    assert len(xs) == n_samples
    api.post(f"get {n_samples} samples from buffer {name}")


def test_buffer_set_samples():
    name = "drum1"
    duration_ms = 500
    buf = api.create_empty_buffer(name, duration_ms)
    api.post("buffer.set_sample example with numpy and scipy.signal")
    api.post(f"created buffer name: '{name}' duration(ms): '{duration_ms}'")
    api.post(f"framecount: {buf.framecount}")
    n_samples = buf.n_samples
    t = np.linspace(0, 1, n_samples, endpoint=False, dtype=np.float64)
    xs = signal.sawtooth(2 * np.pi * 5 * t)
    buf.set_samples(xs)
    api.post(f"set {n_samples} samples to buffer {name}")
