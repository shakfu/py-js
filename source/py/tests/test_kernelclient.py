"""
from: https://stackoverflow.com/questions/33731744/executing-code-in-ipython-kernel-with-the-kernelclient-api

get connection file from from

    jupyter --runtime-dir

"""
import os

from jupyter_client.blocking import BlockingKernelClient

RUNTIME_DIR = os.path.join(os.environ['HOME'], 'Library/Jupyter/runtime')
CONNECTION_FILE = os.path.join(RUNTIME_DIR, 'kernel-29857.json')
kc = BlockingKernelClient(connection_file=CONNECTION_FILE)
kc.load_connection_file()
kc.start_channels()
msgid = kc.execute('a = 10')
reply = kc.get_shell_msg(timeout=5)
print(reply)
