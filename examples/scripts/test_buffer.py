import api


def test_buffer():
    # this step is necessary to get a reference to the `py` external instance
    ext = api.PyExternal()
    b = ext.get_buffer("drum") # b is a buffer instance

    # test it
    api.post(f"framecound: {b.framecount})")
    b.view()
