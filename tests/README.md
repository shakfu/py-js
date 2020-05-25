# Testing


## Tools

- Valgrind



## Issues

# see: https://stackoverflow.com/questions/27672572/embedding-python-in-c-linking-fails-with-undefined-reference-to-py-initialize


- new/main: remove Py_DECREF(x->p_globals)
- new/py_eval: remove Py_XDECREF(locals)
- py/py_eval: change
            Py_DECREF(ptype);      	->      Py_XDECREF(ptype);
            Py_DECREF(pvalue);		->		Py_XDECREF(pvalue);
            Py_DECREF(ptraceback);	-> 		Py_XDECREF(ptraceback);


