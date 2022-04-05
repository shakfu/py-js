from sysconfig import get_config_var

AVAILABLE_VERSIONS = ["3.7.9", "3.8.9", "3.9.12", "3.10.4"]

def get_default_py_version(version: str = None):
    _short_version = None
    if version:
        parts = version.split(".")
        if len(parts) == 2:
            _short_version = version
        else:
            _short_version = ".".join(parts[:2])
    else:
        _short_version = get_config_var("py_version_short")

    if _short_version:
        for ver in AVAILABLE_VERSIONS:
            if ver.startswith(_short_version):
                return ver

    print('version not found, selecting latest available version')
    return AVAILABLE_VERSIONS[-1]


def test_no_input():
    assert get_default_py_version() == '3.9.12'

def test_with_input():
    assert get_default_py_version('3.7.11') == '3.7.9'

def test_with_short_input():
    assert get_default_py_version('3.8') == '3.8.9'

def test_with_bad_input():
    assert get_default_py_version('3.3') == '3.10.4'

def test_with_bad_input2():
    assert get_default_py_version('BAD_INPUT') == '3.10.4'


if __name__ == '__main__':
    test_no_input()
    test_with_input()
    test_with_short_input()
    test_with_bad_input()
    test_with_bad_input2()