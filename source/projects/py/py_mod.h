static char* PY_DEFAULT_MODULE =
"import os\n"
"# import subprocess\n"
"\n"
"EDITOR = 'Sublime Text'\n"
"\n"
"# disabled by default for safety\n"
"# \n"
"# def sysout(cmd):\n"
"#     result = None\n"
"#     try:\n"
"#         result = subprocess.check_output(\n"
"#             cmd.split(), encoding='utf8').strip()\n"
"#     except:\n"
"#         pass\n"
"#     return result\n"
"\n"
"\n"
"def edit(path):\n"
"    editor = os.getenv('EDITOR', EDITOR)\n"
"    os.system(f'open -a \"{editor}\" \"{path}\"')\n"
"\n"
"\n"
"\n"
"# ---------------------------------------------------------\n"
"# protected funcs \n"
"\n"
"def __py_maxmsp_out_dict(py_dict):\n"
"    res = []\n"
"    for k,v in py_dict.items():\n"
"        res.append(k)\n"
"        res.append(':')\n"
"        if type(v) in [list, set, tuple]:\n"
"            for i in v:\n"
"                res.append(i)\n"
"        else:\n"
"            res.append(v)\n"
"    return res\n"
"\n"
"def __py_maxmsp_pipe(arg):\n"
"    args = arg.split()\n"
"    val = eval(args[0], locals(), globals())\n"
"    funcs = [eval(f, locals(), globals()) for f in args[1:]]\n"
"    for f in funcs:\n"
"        val = f(val)\n"
"    return val\n"
"\n"
"\n";