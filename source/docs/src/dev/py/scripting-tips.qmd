# Scripting Do's & Dont's

Here are some tips when scripting max with python using the py external. This only applies to python

## Don't

- redirect stdout and try to read from the python script. The following will crash Max:

```python
# test stdout/stderr redirection
sys.stdout = io.StringIO()
print('foo')
print('hello max!')
api.post("from py: %s", sys.stdout.getvalue())
```

