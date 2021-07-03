# Challenging Bugs

## Cannot Reload External C-extensions without Restarting Max

references:

- [pep-3121](https://www.python.org/dev/peps/pep-3121)
- [post to cython list](https://groups.google.com/g/cython-users/c/SnVpCE7Sq8M/m/K89-W4ubAAAJ)
- [python-issue-34309](https://bugs.python.org/issue34309)

Exclusively in the case for c-based 3rd party python modules (such as the matrix library, Numpy), I have no issue importing them in the first patch that uses them, but if I close the patch (and cleanup my python objects), they are somehow not cleaned up properly and may cause unpredictable behavior: this could be returning nothing at import which is no biggie, but in the particular case of Numpy, it actually crashes Max (and defies any attempt to catch this any exceptions).

It turns out that this is actually a bug in python for embedded applications (which I wasn't aware of, and which is being worked on and may be fixed in future versions.

Therefore, the only thing that I can do right now is to stop users from 'reloading' c-based python extensions after a patch is closed which first uses them successfully, and ask them kindly to restart Max.
Therefore, is there any API method or function, which provides some meta information about whether Max has been freshly started or restarted or a count of patches which have been opened and closed, etc..

My basic idea is have some kind of flag_file which is available

```python
if not flag_file.exists:
    import normally
    touch flag_file

else if flag_file.exists:
    block imports
    error("cannot import c-extensions, restart max to use")
    remove flag_file
```

Related to this bug, I have just posted the following to [Issue 34309](https://bugs.python.org/issue34309) on the python bugtracker:

In my [project](https://github.com/shakfu/py-js), which provides an embedded python3 interpreter to Max/MSP in the form of an 'external' plugin, I have faced similar issues of being unable to reload extension modules, namely numpy, without reliably crashing the host application, in this case Max.

Being able to reload extension modules cleanly is absolutely critical especially in case when python is embedded. Since Numpy is one of the key reasons why people would want to use Python, such a constraint, in this embedded context, becomes a sufficient reason not to use Python at all.

For example, I have recently taken note of similar frustration with this exact [same issue](https://community.vcvrack.com/t/blowing-the-dust-off-python-in-prototype/12909) from the VCV project. I quote: "I should add that CPython and more notably numpy do not support nor advise a complete restart of the interpreter in embedded scenarios without restarting the host process which kind of defeats our purpose in Prototype.

At that point I think I can safely take a step back and turn to the dev community looking for suggestions. Should we throw away numpy, or even python, altogether?"

## Xcode Strangeness: Gettext Dependency

This is not really a bug but some strangeness in xcode.

Since the python3 externals have a dependency on libintl, if one `-lint`
normally in the LDFLAGS the external then links to the homebrew version of
libintl.dylib which is not desired since it creates a requirement for users to
`brew install gettext`

The solution is to link statically via including `libintl.a` in the xcode
project folder itself and then adding the static library as a `link to binary`
entry in `build phases`

The strangeness is that if `libintla.a` resides in director outside the xcode
project directory (let's one parent directory above) and the same method is
used, xcode will fail to compile and assume that it being linked dynamically
with a `-lintl`.

The solution is obviously to keep `libintl.a` in the xcode project folder.

