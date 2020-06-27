# Challenging Bugs


## Numpy

I've been chipping away at my first max external project, (https://github.com/shakfu/py), which provides two python3 externals, a more featureful regular max object (py) which provide a two-way interface between python and max; and another (pyjs),  a jsextension providing a minimal  interface to python via the js object.

So far so good, it's on its way, but I have a showstopper which is as follows:

Exclusively in the case for c-based 3-rd party python modules (such as the matrix library, Numpy), I have no issue importing them in the first patch that uses them, but if I close the patch (and cleanup my python objects), they are somehow not cleaned up properly and may cause unpredictable behavior: this could be returning nothing at import which is no biggie, but in the particular case of Numpy, it actually crashes Max (and defies any attempt to catch this any exceptions).

It turns out that this is actually a bug in python for embedded applications (which I wasn't aware of, and which is being worked on and may be fixed in future versions.

Therefore, the only thing that I can do right now is to stop users from 'reloading' c-based python extensions after a patch is closed which first uses them successfully, and ask them kindly to restart Max.
Therefore, is there any API method or function, which provides some meta information about whether Max has been freshly started or restarted or a 
count of patches which have been opened and closed, etc..

My basic idea is have some kind of flag_file which is available

```python

if not flag_file.exists:
	import normally
	touch flag_file

else if flag_file.exists:
	block imports
	error("cannot import c-extensions, restart max to use")
	remove flag_file


