from subprocess import Popen, PIPE
import pty
import os
from select import select
import sys
import tty

master, slave = pty.openpty()
p = Popen(['python'], stdin=slave, stdout=PIPE, stderr=PIPE)
pin = os.fdopen(master, 'w')
tty.setcbreak(sys.stdin)

msg = ''
errmsg = ''

while True:
    rs, ws, es = select([sys.stdin, p.stdout, p.stderr], [], [])
    for r in rs:
        if r is sys.stdin:
            c = r.read(1)
            if c == '':
                msg = msg[:-1]
            elif c == '\n':
                pin.write(msg+'\n')
                print('\r>>> %s' % msg)
                msg = ''
            else:
                msg += c
                print('\r    '+ ' '*(len(msg)+1), end=' ')
                print('\r>>> %s' % msg, end=' ')
                sys.stdout.flush()
        elif r is p.stdout:
            print(p.stdout.readline(), end=' ')
            sys.stderr.flush()
        elif r is p.stderr:
            errmsg += p.stderr.read(1)
            if errmsg.endswith('>>> '):
                errmsg = errmsg[:-4]
            if errmsg.endswith('\n'):
                print('ERR~%s' % (errmsg,), end=' ')
                errmsg = ''

