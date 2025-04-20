# pyx: example use of cobra with maxcpp

This is a proof-of-concept of using `cobra`, the single-header c++ python3 library for max externals with Graham Wakefield's [maxcpp](https://github.com/grrrwaaa/maxcpp) (C++ templates for Max/MSP objects)

## Usage

This can be built as part of the `demos` subgroup:

```sh
make demos
```

or individually:

```sh
make sh
```

## Possible Future Directions

- [ ] Use `nanobind` or `pybind11`

- [ ] Include option to enable `api` module

- [ ] integrate `maxcpp` and `cobra` such that to enable something like the following example where the inherited `PythonExternal` class already include python methods.

```c++
#include "maxcpp_py_interpreter.h"

class Demo : public PythonExternal<Demo> {

public:
	Demo(t_symbol * sym, long ac, t_atom * av)
	{ 
		setupIO(2, 2); // inlets / outlets
	}
	~Demo() {}	
	
	// methods:
	void bang(long inlet)
	{ 
		outlet_bang(m_outlets[0]);
	}

	void testfloat(long inlet, double v)
	{
		post("inlet %ld float %f", inlet, v);
		outlet_float(m_outlets[0], v);
	}
	
	void testint(long inlet, long v)
	{
		post("inlet %ld int %ld", inlet, v);
		outlet_int(m_outlets[0], v);
	}
	
	void test(long inlet, t_symbol * s, long ac, t_atom * av)
	{ 
		outlet_anything(m_outlets[1], gensym("test"), ac, av); 
	}

};

C74_EXPORT int main(void) {
	// create a class with the given name:
	Demo::makePythonExternal("demo");
	REGISTER_METHOD(Demo, bang);

	REGISTER_METHOD_FLOAT(Demo, testfloat);
	REGISTER_METHOD_DEFLONG(Demo, testint);
	REGISTER_METHOD_GIMME(Demo, test);
	
	// these are for handling float/int messages directly (no method name in Max):
	REGISTER_INLET_FLOAT(Demo, testfloat);
	REGISTER_INLET_LONG(Demo, testint);
}

- [ ] similar to the above but use `min.api` instead of `maxcpp`


```

