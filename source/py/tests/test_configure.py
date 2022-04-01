from pathlib import Path

DEFAULT_ENV_VARS = dict(
    MACOSX_DEPLOYMENT_TARGET='10.13',
)

def configure(*options, **kwargs):
    """generate ./configure instructions"""
    _kwargs = {}
    _options = [opt.replace('_', '-') for opt in options]
    _env = {}

    if DEFAULT_ENV_VARS:
        _env.update(DEFAULT_ENV_VARS)

    if '_env' in kwargs:
        _env = kwargs.pop('_env', {})

    if _env:
        prefix = " ".join(f'{k}={v}' for k,v in _env.items())
    else:
        prefix = ""

    for key in kwargs:
        _key = key.replace('_','-')
        _kwargs[_key] = kwargs[key]

    print (
        '{prefix} ./configure {options} {kwargs}'.format(
            prefix=prefix,
            options=" ".join(f'--{opt}' for opt in _options), 
            kwargs=" ".join(f"--{k}='{v}'" for k,v in _kwargs.items())
        )
    )


BUILD_LIB=Path('/build/lib')
MAC_DEP_TARGET='10.13'
PREFIX=Path('/prefix/')

print('-'*79)
print('VanillaPythonBuilder')

configure('without_doc_strings',
    enable_framework=repr(BUILD_LIB),
)

print('-'*79)
print('XzBuilder')
print(
    f"""MACOSX_DEPLOYMENT_TARGET={MAC_DEP_TARGET} \
    ./configure --disable-shared --enable-static --prefix='{PREFIX}'"""
)
print()
configure('disable_shared', 'enable_static', prefix=PREFIX)


print('-'*79)
print('FrameworkPythonBuilder')

print(
    f"""\
./configure MACOSX_DEPLOYMENT_TARGET='{MAC_DEP_TARGET}' \
    --enable-framework='{BUILD_LIB}' \
    --with-openssl='{BUILD_LIB}/openssl' \
    --without-doc-strings \
    --enable-ipv6 \
    --without-ensurepip \
    --with-lto \
    --enable-optimizations
"""
)

configure(
    'without_doc_strings',
    'enable_ipv6',
    'without_ensurepip',
    'with_lto',
    'enable_optimizations',
    enable_framework=BUILD_LIB,
    with_openssl=BUILD_LIB / 'openssl'
)


print('-'*79)
print('SharePythonBuilder')

print(
    f"""\
./configure MACOSX_DEPLOYMENT_TARGET={MAC_DEP_TARGET} \
    --prefix='{PREFIX}' \
    --enable-shared \
    --with-openssl='{BUILD_LIB / "openssl"}' \
    --without-doc-strings \
    --enable-ipv6 \
    --without-ensurepip \
    --with-lto \
    --enable-optimizations \
    ac_cv_lib_intl_textdomain=no
"""
)

configure(
    'enable_shared',
    'without_doc_strings',
    'enable_ipv6',
    'without_ensurepip',
    'with_lto',
    'enable_optimizations',
    prefix=PREFIX,
    enable_framework=BUILD_LIB,
    with_openssl=BUILD_LIB / 'openssl'
)

print('-'*79)
print('StaticPythonBuilder')

configure(
        f"""\
    ./configure MACOSX_DEPLOYMENT_TARGET={MAC_DEP_TARGET} \
        --prefix='{PREFIX}' \
        --with-openssl='{BUILD_LIB / "openssl"}' \
        --without-doc-strings \
        --enable-ipv6 \
        --without-ensurepip \
        --with-lto \
        --enable-optimizations
    """
    )

configure(
    'without_doc_strings',
    'enable_ipv6',
    'without_ensurepip',
    'with_lto',
    'enable_optimizations',
    prefix=PREFIX,
    enable_framework=BUILD_LIB,
    with_openssl=BUILD_LIB / 'openssl'
)
