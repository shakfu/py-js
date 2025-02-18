import api


def test_pyexternal_init():
    ext = api.PyExternal()
    ext.out(f"{ext.name} external is initialized")


def test_pyexternal_box():
    ext = api.PyExternal()
    box = ext.get_box()
    ext.log_info(f"rec: {box.get_patching_rect()}")
