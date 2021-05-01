"""
from: https://stackoverflow.com/questions/33731744/executing-code-in-ipython-kernel-with-the-kernelclient-api

get connection file from

    jupyter --runtime-dir

"""
import os
from pprint import pprint

from jupyter_client.consoleapp import JupyterConsoleApp

RUNTIME_DIR = os.path.join(os.environ['HOME'], 'Library/Jupyter/runtime')
CONNECTION_FILE = 'kernel-29857.json'


class MyKernelApp(JupyterConsoleApp):
    def __init__(self, connection_file, runtime_dir):
        self._dispatching = False
        self.existing = connection_file
        self.runtime_dir = runtime_dir
        self.initialize()

app = MyKernelApp(CONNECTION_FILE, RUNTIME_DIR)
kc = app.kernel_client
kc.execute("print 'hello'")
msg = kc.iopub_channel.get_msg(block=True, timeout=1)
pprint(msg)
