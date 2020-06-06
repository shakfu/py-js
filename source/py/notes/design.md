# Design Notes

There's a tradeoff between

1. having a `py_<operation>` method for each type of operation -> many methods

2. having a single heuristic `py_anything` which aproximately tries to figure out what to do from input.

both ways have merit so (1) will be done first to capture the semantics (2) will be done last once semantics are stable.


## py_call algorithm

```python

def py_call(s: str, argc: int , argv: list) -> None:

	is_eval=1

	text = atom_gettext(argc, argv)

	try:
		codeobj = Py_CompileString(text, as_type='eval_input')

	except SyntaxError:
		codeobj = Py_CompileString(text, as_type='single_input')


	pval = PyEval_EvalCode(codeobj);

	if is_eval:
		process_output(pval)
	else:
		pass

```

## py_anything algorithm

```python

def py_anything(s: str, argc: int , argv: list) -> None:

    something = PyRun_String(s, as_type='eval_input')

    if not(something):
    	return

    if not(callable(something)):
    	# process as successfully evaluated eval output
    	process_output(something)

    argslist = []
    for i in argv:
    	if i.type == 'float':
    		argslist.append(float(i))
    	elif i.type == 'int':
    		argslist.append(int(i))
    	else:
    		argslist.append(str(i))

    ## treat it as a callable

    try:
	    pval = something(*argslist)
	    process_output(pval)
	except TypError:
		pass

	try:
		pval = something(argslist)
		process_output(pval)
	except:
		return
```

## combined algorithm

```python

def py_combined(s: str, argc: int , argv: list) -> None:

	is_eval=1

	text = atom_gettext(argc, argv)

	try:
		codeobj = Py_CompileString(text, as_type='eval_input')

	except SyntaxError:
		codeobj = Py_CompileString(text, as_type='single_input')


	pval = PyEval_EvalCode(codeobj);

	if is_eval:
		process_output(pval)
	else:
		pass

```

