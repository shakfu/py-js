/*
shell Copyright (c) 2013-2019 Jeremy Bernstein and Bill Orcutt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "ext.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <mutex>
#include <atomic>

#define MAX_MESSAGELEN	4096

#ifdef MAC_VERSION
#include <unistd.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <mach-o/getsect.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#else
#ifndef Boolean
#define Boolean bool
#endif
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef MAC_VERSION
typedef int t_fildes;
typedef int t_procid;
#define t_fildes	int
#define WRITE(f,s)	write(f, s, strlen(s))
#define READ		read
#define READ_HANDLE(x)	x->fd
#define WRITE_HANDLE(x) x->fd
typedef pid_t (*t_forkfn)(void);
#else
typedef HANDLE t_fildes;
typedef HANDLE t_procid;
#define WRITE			WriteToPipe
#define READ			ReadFromPipe
#define READ_HANDLE(e)	x->fd_r
#define WRITE_HANDLE(e) x->fd_w
#define CLEAN_CLOSEHANDLE(h) if (h) { CloseHandle(h); h = 0; }
#define kill windows_kill

BEGIN_USING_C_LINKAGE
extern BOOL
APIENTRY
MyCreatePipeEx(
			   OUT LPHANDLE lpReadPipe,
			   OUT LPHANDLE lpWritePipe,
			   IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
			   IN DWORD nSize,
			   DWORD dwReadMode,
			   DWORD dwWriteMode,
			   DWORD dwPipeMode
			   );
int WriteToPipe(HANDLE fh, char *str);
int ReadFromPipe(HANDLE fh, char *str, DWORD slen);
END_USING_C_LINKAGE
#endif

struct t_shell_threadinfo
{
	std::string cmdstring;
	std::vector<std::string> cmdargs;
};

struct t_shell_threading
{
	std::thread thr;
	std::mutex tcmut;
	std::condition_variable tcvar;
	std::unique_ptr<t_shell_threadinfo> info;
	std::atomic<int> run;

	t_shell_threading() : info{nullptr}, run{false} {}
};

typedef struct _shell
{
    t_object		ob;
	void			*textout;
	void			*bangout;
	char			cmdbuf[MAX_MESSAGELEN]; //command
#ifdef MAC_VERSION
	t_fildes		fd;
#else
	t_fildes		fd_r;
	t_fildes		fd_w;
#endif
	long			forkmode; // disabled on Windows
	t_procid		pid;
	char			merge_stderr;
	t_symbol		*wd;
	t_symbol		*shell;
#ifdef WIN_VERSION
	char			unicode;
#endif
	char			symout;
	t_shell_threading *threading;
} t_shell;

t_class *shell_class;

void doReport();
void shell_bang(t_shell *x);
void shell_anything(t_shell *x, t_symbol *s, long ac, t_atom *av);
void shell_do(t_shell *x, t_symbol *s, long ac, t_atom *av);
void shell_write(t_shell *x, t_symbol *s, long ac, t_atom *av);
void shell_dowrite(t_shell *x, t_symbol *s, long ac, t_atom *av);
void shell_stop(t_shell *x);
void shell_kill(t_shell *x);
void shell_qfn(t_shell *x);
void shell_assist(t_shell *x, void *b, long m, long a, char *s);
void shell_free(t_shell *x);
void *shell_new(t_symbol *s, long ac, t_atom *av);
void shell_output(t_shell *x, t_symbol *s, long ac, t_atom *av);
Boolean shell_readline(t_shell *x);
void shell_atoms2string(long ac, t_atom *av, std::string &str);

void shell_threadfn(t_shell *x);
void shell_terminated(t_shell *x);

t_max_err shell_attr_wd_set(t_shell *x, void *attr, long ac, t_atom *av);
t_max_err shell_attr_wd_get(t_shell *x, void *attr, long *ac, t_atom **av);
t_max_err shell_attr_shell_set(t_shell *x, void *attr, long ac, t_atom *av);
t_max_err shell_attr_shell_get(t_shell *x, void *attr, long *ac, t_atom **av);

int shell_pipe_open(t_shell *x, t_fildes *masterfd_r, t_fildes *masterfd_w, char *cmd, char *argv[], t_procid *ppid, int merge_stderr);
int shell_pipe_close(t_shell *x, t_fildes *masterfd_r, t_fildes *masterfd_w, t_procid pid, int *result);

static t_symbol *ps_default, *ps_nothing;

int C74_EXPORT main(void)
{
	shell_class = class_new("shell", (method)shell_new, (method)shell_free, sizeof(t_shell), 0L, A_GIMME, 0);
	
	class_addmethod(shell_class, (method)shell_bang,		"bang",					0);
	class_addmethod(shell_class, (method)shell_kill,		"pkill",				0);
	class_addmethod(shell_class, (method)shell_write,		"pwrite",	A_GIMME,	0);
	class_addmethod(shell_class, (method)shell_write,		"penter",	A_GIMME,	0);
	class_addmethod(shell_class, (method)shell_anything,	"anything",	A_GIMME,	0);
	class_addmethod(shell_class, (method)shell_assist,		"assist",	A_CANT,		0);
	class_addmethod(shell_class, (method)doReport,			"dblclick",	A_CANT,		0);
	
	CLASS_ATTR_CHAR(shell_class, "symout", 0, t_shell, symout);
	CLASS_ATTR_DEFAULT_SAVE(shell_class, "symout", 0, "0");
	CLASS_ATTR_STYLE_LABEL(shell_class, "symout", 0, "onoff", "Output Line as One Symbol");

	CLASS_ATTR_CHAR(shell_class, "stderr", 0, t_shell, merge_stderr);
	CLASS_ATTR_DEFAULT_SAVE(shell_class, "stderr", 0, "0");
	CLASS_ATTR_STYLE_LABEL(shell_class, "stderr", 0, "onoff", "Merge STDERR With STDOUT");

	CLASS_ATTR_SYM(shell_class, "wd", 0, t_shell, wd);
	CLASS_ATTR_ACCESSORS(shell_class, "wd", (method)shell_attr_wd_get, (method)shell_attr_wd_set);
	CLASS_ATTR_DEFAULT_SAVE(shell_class, "wd", 0, "(default)");
	CLASS_ATTR_STYLE_LABEL(shell_class, "wd", 0, "filefolder", "Working directory");

	CLASS_ATTR_SYM(shell_class, "shell", 0, t_shell, shell);
	CLASS_ATTR_ACCESSORS(shell_class, "shell", (method)shell_attr_shell_get, (method)shell_attr_shell_set);
	CLASS_ATTR_DEFAULT_SAVE(shell_class, "shell", 0, "");
	CLASS_ATTR_STYLE_LABEL(shell_class, "shell", 0, "file", "Shell");

	CLASS_ATTR_LONG(shell_class, "forkmode", 0, t_shell, forkmode);
	CLASS_ATTR_ENUMINDEX2(shell_class, "forkmode", 0, "fork", "vfork");
	CLASS_ATTR_DEFAULT_SAVE(shell_class, "forkmode", 0, "0");
	CLASS_ATTR_STYLE_LABEL(shell_class, "forkmode", 0, "enumindex", "Fork Method (Expert)");
#ifdef WIN_VERSION
	CLASS_ATTR_INVISIBLE(shell_class, "forkmode", 0);
#endif

	class_register(CLASS_BOX, shell_class);

	ps_default = gensym("(default)");
	ps_nothing = gensym("");

	object_post(NULL, "shell: compiled %s", __TIMESTAMP__);
	return 0;
}

t_max_err shell_attr_wd_set(t_shell *x, void *attr, long ac, t_atom *av)
{
	char fname[MAX_FILENAME_CHARS];
	short fvol;

	if (ac && av) {
		t_symbol *pathsym = atom_getsym(av);
		if (pathsym && pathsym != ps_nothing && pathsym != ps_default
			&& !path_frompathname(atom_getsym(av)->s_name, &fvol, fname)
			&& fvol && !*fname) 
		{
			char conform[MAX_PATH_CHARS];

			if (!path_nameconform(pathsym->s_name, conform, PATH_STYLE_MAX, maxversion() >= 0x733 ? 10 : 9)) { // PATH_TYPE_MAXDB changed Max 7.3.3, oops
				x->wd = gensym(conform);
			}
			else {
				x->wd = pathsym; // unlikely
			}
		}
		else if (pathsym == ps_nothing || pathsym == ps_default)
		{
			x->wd = ps_nothing;
		}
	}
	else {
		x->wd = ps_nothing;
	}
	return MAX_ERR_NONE;
}

t_max_err shell_attr_wd_get(t_shell *x, void *attr, long *ac, t_atom **av)
{
	if (ac && av) {
		char alloc;
		if (atom_alloc(ac, av, &alloc) == MAX_ERR_NONE) {
			atom_setsym(*av, x->wd == ps_nothing ? ps_default : x->wd);
			return MAX_ERR_NONE;
		}
		return MAX_ERR_OUT_OF_MEM;
	}
	return MAX_ERR_GENERIC;
}

t_max_err shell_attr_shell_set(t_shell *x, void *attr, long ac, t_atom *av)
{
	char fname[MAX_FILENAME_CHARS];
	short fvol;

	if (ac && av) {
		t_symbol *pathsym = atom_getsym(av);
		if (pathsym && pathsym != ps_nothing && pathsym != ps_default 
			&& !path_frompathname(atom_getsym(av)->s_name, &fvol, fname)
			&& fvol && *fname) 
		{
			char conform[MAX_PATH_CHARS];
			if (!path_nameconform(pathsym->s_name, conform, PATH_STYLE_MAX, maxversion() >= 0x733 ? 10 : 9)) { // PATH_TYPE_MAXDB changed Max 7.3.3, oops
				x->shell = gensym(conform);
			}
			else {
				x->shell = pathsym; // unlikely
			}
		}
		else if (pathsym == ps_nothing || pathsym == ps_default) {
			x->shell = ps_nothing;
		}
	}
	else {
		x->shell = ps_nothing;
	}
	return MAX_ERR_NONE;
}

t_max_err shell_attr_shell_get(t_shell *x, void *attr, long *ac, t_atom **av)
{
	if (ac && av) {
		char alloc;
		if (atom_alloc(ac, av, &alloc) == MAX_ERR_NONE) {
			atom_setsym(*av, x->shell == ps_nothing ? ps_default : x->shell);
			return MAX_ERR_NONE;
		}
		return MAX_ERR_OUT_OF_MEM;
	}
	return MAX_ERR_GENERIC;
}


void shell_anything(t_shell *x, t_symbol *s, long ac, t_atom *av)
{
	if (!x->pid) {
		defer_medium(x, (method)shell_do, s, (short)ac, av);
	}
}

void shell_write(t_shell *x, t_symbol *s, long ac, t_atom *av)
{
	if (x->pid) {
		defer_medium(x, (method)shell_dowrite, s, (short)ac, av);
	}
}
void shell_atoms2string(long ac, t_atom *av, std::string &str)
{
	char tmp[MAX_MESSAGELEN];
	int i;

	for (i = 0; i < ac; i++) {
		switch(atom_gettype(av + i)) {
			case A_LONG:
				snprintf_zero(tmp, MAX_MESSAGELEN, "%" ATOM_LONG_FMT_MODIFIER "d", atom_getlong(av + i));
				break;
			case A_FLOAT:
				snprintf_zero(tmp, MAX_MESSAGELEN, "%f", atom_getfloat(av + i));
				break;
			case A_SYM:
			{
				const char *formatstr = "%s";
				const char *symstr = atom_getsym(av + i)->s_name;
				if (strchr(symstr, ' ') && *symstr != '\\' && *(symstr+1) != '\"') {
					formatstr = "\"%s\"";
				}
				snprintf_zero(tmp, MAX_MESSAGELEN, formatstr, atom_getsym(av + i)->s_name);
			}
				break;
			default:
				continue;
		}
		if (i > 0) {
			str += " ";
		}
		str += tmp;
	}
}

void shell_dowrite(t_shell *x, t_symbol *s, long ac, t_atom *av)
{
	if (x->pid && WRITE_HANDLE(x)) {
		std::string cmd;

		shell_atoms2string(ac, av, cmd);

		if (s == gensym("penter")) {
			cmd += "\n";
		}
		if (cmd.length()) {
			WRITE(WRITE_HANDLE(x), const_cast<char *>(cmd.c_str()));
		}
	}
}

static inline std::string quote(std::string &s)
{
	if (s.find(' ') != std::string::npos) {
		std::string quoted("\"");
		quoted += s;
		quoted += "\"";
		return quoted;
	}
	return s;
}

void shell_do(t_shell *x, t_symbol *s, long ac, t_atom *av)	
{
	std::string cmd("");
	char shellcmd[MAX_PATH_CHARS] = "sh";
	std::vector<std::string> args;

	if (s) {
		char cmdtemp[MAX_PATH_CHARS] = "";

		if (path_getseparator(s->s_name)) {
			short cmdvol;
			char cmdfile[MAX_PATH_CHARS];
			if (!path_frompathname(s->s_name, &cmdvol, cmdfile)) { // path exists
				if (path_nameconform(s->s_name, cmdtemp, PATH_STYLE_NATIVE, PATH_TYPE_BOOT)) {
					*cmdtemp = '\0';
				}
			}
		}
		if (!*cmdtemp) {
			strncpy(cmdtemp, s->s_name, MAX_PATH_CHARS);
		}

		cmd = cmdtemp;
		cmd = quote(cmd);

		// process args
		if (ac && av) {
			cmd += " ";
			shell_atoms2string(ac, av, cmd);
		}
	}
	else {
		cmd = x->cmdbuf;
	}

	if (x->shell != ps_nothing) {
		strncpy(shellcmd, x->shell->s_name, MAX_PATH_CHARS);
		// brute force and potentially wrong, assuming that any other shell will use -c to read input from the string
	}

	if (cmd.length()) {
#ifdef MAC_VERSION
		args.push_back(shellcmd);
		args.push_back("-c");
		args.push_back(cmd);
#else
		const char *shellarg = "/U /C"; // for CMD.exe, /U = unicode, /C = returning after executing string arg

		x->unicode = true;
		if (x->shell != ps_nothing) {
			strncpy(shellcmd, x->shell->s_name, MAX_PATH_CHARS);
			// brute force and potentially wrong, assuming that any other shell will use -c to read input from the string
			shellarg = "-c";
			x->unicode = false;
		}
		else {
			size_t nSize = _countof(shellcmd);
			getenv_s(&nSize, shellcmd, MAX_PATH_CHARS, "COMSPEC");
		}

		args.push_back(shellcmd);
		args.push_back(shellarg);
		args.push_back(cmd);
#endif

		shell_stop(x); // kill previous command, if any

		std::unique_ptr<t_shell_threadinfo> ti(new t_shell_threadinfo);
		ti->cmdstring = shellcmd;
		ti->cmdargs = args;
		x->threading->info = std::move(ti);
		x->threading->run = true;
		x->threading->tcvar.notify_all();
	}
}

void shell_terminated(t_shell *x)
{
	x->pid = 0;
	outlet_bang(x->bangout);
}

void shell_threadfn(t_shell *x)
{
	t_shell_threading *threading = x->threading;
	while (1) {
		std::unique_ptr<t_shell_threadinfo> info;

		{
			std::unique_lock<std::mutex> lk(threading->tcmut);
			threading->tcvar.wait(lk, [threading] { return (threading->run ? true : false); });
			threading->run = false;
			info = std::move(threading->info);
		}
		if (!info) break;

		int size = info->cmdargs.size() + 1;
		char **args = new char*[size];
		char *cmdstr = const_cast<char *>(info->cmdstring.c_str());
		char **a = args;

		for (auto it = info->cmdargs.begin(); it < info->cmdargs.end(); it++) {
			*a++ = const_cast<char *>((*it).c_str());
		}
		*a = nullptr;

		if ((shell_pipe_open(x, &(READ_HANDLE(x)), &(WRITE_HANDLE(x)),
							 cmdstr, args,
							 &x->pid, (int)x->merge_stderr)))
		{
#ifdef MAC_VERSION // read and write are the same, don't need to do this twice
			int flags = fcntl(WRITE_HANDLE(x), F_GETFL, 0) | O_NONBLOCK;
			fcntl(WRITE_HANDLE(x), F_SETFL, flags);
#endif
			if (size > 2) {
				strncpy(x->cmdbuf, args[2], MAX_MESSAGELEN);
			}
			else x->cmdbuf[0] = '\0';

			// ready to read
			while (x->pid) {
				int rv;
				while (shell_readline(x))
					;
				// check if the process has terminated
#ifdef MAC_VERSION
				if (waitpid(x->pid, &rv, WNOHANG))
#else
				if (WaitForSingleObject(x->pid, 0) == WAIT_OBJECT_0)
#endif
				{
					shell_pipe_close(x, &READ_HANDLE(x), &WRITE_HANDLE(x), x->pid, &rv);
					defer_low(x, (method)shell_terminated, NULL, 0, NULL);
					break;
				}
				// otherwise, requeue
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
		delete[] args;
	}
	// std::cout << "exiting threadfn" << std::endl;
}

void shell_bang(t_shell *x)
{
	if (!x->pid) {
		defer_medium(x, (method)shell_do, 0, 0, 0);
	}
}

void shell_stop(t_shell *x)	
{
	if (x->pid) {
		int rv;
#ifdef MAC_VERSION
		kill(x->pid, SIGKILL); // pipe_close_3 will do this on windows
#endif
		shell_pipe_close(x, &READ_HANDLE(x), &WRITE_HANDLE(x), x->pid, &rv);
		shell_terminated(x);
	}
}

void shell_kill(t_shell *x) 
{	
	if (x->pid) {
		defer_medium(x, (method)shell_stop, NULL, 0, NULL);
	}
}

void shell_output(t_shell *x, t_symbol *s, long ac, t_atom *av)
{
	if (ac && av && atomisstring(av)) {
		long argc = 0;
		t_atom *argv = NULL;
		t_string *str = (t_string *)atom_getobj(av);

		if (x->symout) {
			outlet_anything(x->textout, gensym(str->s_text), 0, NULL);
		}
		else {
			if (atom_setparse(&argc, &argv, str->s_text) == MAX_ERR_NONE) {
				if (atom_gettype(argv) == A_SYM) {
					outlet_anything(x->textout, atom_getsym(argv), short(argc - 1), argv + 1);
				}
				else {
					outlet_list(x->textout, NULL, short(argc), argv);
				}
				sysmem_freeptr(argv);
			}
		}
		object_free(str);
	}
}

#ifdef WIN_VERSION
WCHAR *shell_char_to_wchar(int cp, const char *text, int nbytes, int *wlen)
{
	WCHAR *wbuf = NULL;
	int wbytes = 0;

	wbytes = MultiByteToWideChar(cp, 0, (char *)text, nbytes, NULL, 0);
	if (wbytes) {
		wbuf = (WCHAR *)sysmem_newptr(wbytes * sizeof(WCHAR));
		wbytes = MultiByteToWideChar(cp, 0, (char *)text, nbytes, wbuf, wbytes);
		if (!wbytes) {
			C74_ASSERT(0);
		}
	}
	else {
		C74_ASSERT(0);
	}
	*wlen = wbytes;
	return wbuf;
}

char *shell_utf8_to_native(const char *ubuf, int ubytes, int *len)
{
	char *buf = NULL;
	int bytes = 0;
	int wbytes;
	WCHAR *wbuf = shell_char_to_wchar(CP_UTF8, ubuf, ubytes, &wbytes);
	if (wbuf) {
		bytes = WideCharToMultiByte(CP_ACP, 0, wbuf, wbytes, NULL, 0, NULL, NULL);
		if (bytes) {
			buf = (char *)sysmem_newptr(bytes);
			bytes = WideCharToMultiByte(CP_ACP, 0, wbuf, wbytes, buf, bytes, NULL, NULL);
			if (!bytes) {
				C74_ASSERT(0);
			}
		}
		else {
			C74_ASSERT(0);
		}
		sysmem_freeptr(wbuf);
	}
	*len = bytes;
	return buf;
}

#endif

char *shell_escape_backslashes(const char *str)
{
	// count the backslashes
	t_ptr_size ct = 0;
	const char *c = str;
	char *newstr = NULL;
	char *n;
	t_bool hasslash = false;

	while (c && *c) {
		if (*c++ == '\\') {
			ct++;
			hasslash = true;
		}
		ct++;
	}
	if (!hasslash) return NULL;
	newstr = (char *)sysmem_newptr(ct + 1);
	c = str;
	n = newstr;
	while (c && *c) {
		if (*c == '\\') {
			*n++ = '\\';
		}
		*n++ = *c++;
	}
	*n = '\0';
	return newstr;
}

Boolean shell_readline(t_shell *x)
{
	char stream[MAX_MESSAGELEN];
	char line[MAX_MESSAGELEN];
	char *lp1, *lp2;
	long bytes;
	long offset = 0;
	t_atom a;
	char *readstream = stream;
	int charsize = 1;
	char *escape;
	t_string *str;

#ifdef WIN_VERSION
	WCHAR *unicodestream = NULL;

	if (x->unicode) {
		unicodestream = (WCHAR *)sysmem_newptr(MAX_MESSAGELEN * sizeof(WCHAR));
		readstream = (char *)unicodestream;
		charsize = sizeof(WCHAR);
	}
#endif

	while ((bytes = READ(READ_HANDLE(x), readstream + offset, ((MAX_MESSAGELEN-1) * charsize) - offset)) > 0) {
		readstream[bytes + offset] = '\0'; // 0-terminate.

#ifdef WIN_VERSION
		// the problem with Unicode mode is that the output might come back as Unicode or not
		// depending on which tool(s) were used. Built-in commands like DIR return Unicode,
		// but "git --version" returns ANSI. This complicates the logic of dealing with
		// incomplete lines considerably.
		if (x->unicode) {
			if (IsTextUnicode(unicodestream, bytes, NULL)) {
				int sizeinchars = bytes / sizeof(WCHAR);
				WideCharToMultiByte(CP_UTF8, 0, unicodestream, sizeinchars, stream, MAX_MESSAGELEN, NULL, NULL);
				stream[sizeinchars] = '\0';
			}
			else {
				int widelen = 0;
				WCHAR *widebuf = shell_char_to_wchar(CP_ACP, (const char *)unicodestream, bytes, &widelen);
				if (widebuf) {
					int sizeinchars = WideCharToMultiByte(CP_UTF8, 0, widebuf, widelen, stream, MAX_MESSAGELEN, NULL, NULL);
					stream[sizeinchars] = '\0';
					sysmem_freeptr(widebuf);
				}
				charsize = 1;
			}
		}
#endif
		lp2 = stream;
		while ((lp1 = strchr(lp2, '\n'))) { // for each complete line...
			size_t cbytes = lp1 - lp2;
			if (lp1 != stream && *(lp1 - 1) == '\r') {
				cbytes -= 1;
			}
			sysmem_copyptr(lp2, line, (long)cbytes);
			line[cbytes] = '\0';
			lp2 = lp1 + 1;

			escape = shell_escape_backslashes(line);
			str = string_new(escape ? escape : line);
			if (escape) {
				sysmem_freeptr(escape);
				escape = NULL;
			}
			atom_setobj(&a, str);
			defer(x, (method)shell_output, NULL, 1, &a);
		}
		if (lp2 && *lp2) { // there's an incomplete line, rewrite it to the front of the
						   // read buffer and set the offset.
			offset = (long)strlen(lp2) * charsize;
#ifdef WIN_VERSION
			strncpy(line, lp2, MAX_MESSAGELEN); // temp copy
			if (x->unicode) {
				if (charsize == sizeof(WCHAR)) { // it's really unicode
					MultiByteToWideChar(CP_UTF8, 0, lp2, -1, unicodestream, MAX_MESSAGELEN);
				}
				else {
					int blen;
					char *buf = shell_utf8_to_native(lp2, -1, &blen);
					// convert back to the native code page
					if (buf) {
						strncpy((char *)unicodestream, (const char *)buf, MAX_MESSAGELEN);
						sysmem_freeptr(buf);
					}
				}
			}
			else
#endif
			{
				strncpy(stream, line, MAX_MESSAGELEN);
			}
		} else {
			offset = 0;
		}
	}
	if (offset) {
		escape = shell_escape_backslashes(line);
		str = string_new(escape ? escape : line);
		if (escape) {
			sysmem_freeptr(escape);
			escape = NULL;
		}
		atom_setobj(&a, str);
		defer(x, (method)shell_output, NULL, 1, &a);
	}
#ifdef WIN_VERSION
	if (unicodestream) {
		sysmem_freeptr(unicodestream);
	}
#endif
	return FALSE;
}

void doReport()
{
	post("shell  _  bill orcutt (user@publicbeta.cx) / jeremy bernstein (jeremy@cycling74.com)  _  %s", __DATE__);
}

void shell_assist(t_shell *x, void *b, long m, long a, char *s)
{
	if (m==1)
		sprintf(s,"anything: shell command to exec");
	else if (m==2)
		switch (a) {

			case 0:
				strcpy(s,"stdout as list");
				break;

			case 1:
				strcpy(s, "bang when done");
				break;

		} 
}

void shell_free(t_shell *x)	
{
	shell_stop(x);

	x->threading->info = nullptr;
	x->threading->run = true;
	x->threading->tcvar.notify_all();
	x->threading->thr.join();

	delete x->threading;
}

void *shell_new(t_symbol *s, long ac, t_atom *av)
{
	t_shell *x;

	x = (t_shell *)object_alloc(shell_class);
	if (x) {
		x->bangout = bangout(x);
		x->textout = outlet_new(x,NULL);
#ifdef MAC_VERSION
		x->fd = 0;
#else
		x->fd_r = NULL;
		x->fd_w = NULL;
		x->unicode = false;
#endif
		x->pid = 0;
		x->cmdbuf[0] = '\0';
		x->wd = ps_nothing;
		x->shell = ps_nothing;

		attr_args_process(x, (short)ac, av);

		x->threading = new t_shell_threading;
		x->threading->thr = std::thread(shell_threadfn, x);
	}
	return(x);
}

/////// PIPE CODE
// http://rachid.koucha.free.fr/tech_corner/pty_pdip.html
// using posix_openpt()

#ifndef NSIG
# define NSIG 32
#endif

int shell_pipe_open(t_shell *x, t_fildes *masterfd_r, t_fildes *masterfd_w, char *cmd, char *argv[], t_procid *ppid, int merge_stderr)
{
#ifdef MAC_VERSION
	int masterfd = posix_openpt(O_RDWR |O_NOCTTY);
	int slavefd;
	char *slavedevice;
	int rc;
	char workingdir[MAX_PATH_CHARS] = "";
	sigset_t oldmask, newmask;
	struct sigaction sig_action;

	*ppid = 0;
	
	if (masterfd == -1
		|| grantpt(masterfd) == -1
		|| unlockpt(masterfd) == -1
		|| (slavedevice = ptsname(masterfd)) == NULL) {
		//cpost("Unable to open pty.\n");
		return 0;
	}
	
	slavefd = open(slavedevice, O_RDWR | O_NOCTTY);
	if (slavefd < 0) {
		//cpost("Unable to open slave end.\n");
		close(masterfd);
		return 0;
	}

	if (x->wd != ps_nothing) { // custom wd
		path_nameconform(x->wd->s_name, workingdir, PATH_STYLE_NATIVE, PATH_TYPE_BOOT);
	}
	else {
		char *homedir = getenv("HOME");
		if (homedir && *homedir) {
			strncpy(workingdir, homedir, MAX_PATH_CHARS);
		}
	}

	sigfillset(&newmask);
	if (pthread_sigmask(SIG_SETMASK, &newmask, &oldmask) < 0) {
		object_error((t_object *)x, "error setting sigmask: %s", strerror(errno));
		return 0;
	}

	// SIGTTOU or SIGTTIN won't be sent in processes created using vfork, which breaks SSH
	t_forkfn forkfn = (t_forkfn)(x->forkmode ? vfork : fork);
	*ppid = forkfn();
	if (*ppid < 0) {
		close(masterfd);
		close(slavefd);
		return 0; // error
	}

	if (*ppid == 0) { // child
		struct termios orig_termios, new_termios;

		umask(0);

		close(masterfd); // close the master
		
		// Save the default parameters of the slave side of the PTY
		rc = tcgetattr(slavefd, &orig_termios);
		
		// Set raw mode on the slave side of the PTY
		new_termios = orig_termios;
		cfmakeraw (&new_termios);
		tcsetattr (slavefd, TCSANOW, &new_termios);
		
		dup2(slavefd, STDIN_FILENO); // PTY becomes standard input (0)
		dup2(slavefd, STDOUT_FILENO); // PTY becomes standard output (1)
		if (merge_stderr)
			dup2(slavefd, STDERR_FILENO); // PTY becomes standard error (2)
		
		close(slavefd); // is now unnecessary
		
		setsid(); // Make the current process a new session leader
		// As the child is a session leader, set the controlling terminal to be the slave side of the PTY
		// (Mandatory for programs like the shell to make them manage correctly their outputs)
		ioctl(0, TIOCSCTTY, 1);

		// reset signals
		sig_action.sa_handler = SIG_DFL;
		sig_action.sa_flags = 0;
		sigemptyset(&sig_action.sa_mask);

		for (int i = 0 ; i < NSIG ; i++) {
			// Only possible errors are EFAULT or EINVAL
			// The former wont happen, the latter we
			// expect, so no need to check return value
			sigaction(i, &sig_action, NULL);
		}

		// Unmask all signals in child, since we've no idea
		// what the caller's done with their signal mask
		if (pthread_sigmask(SIG_SETMASK, &oldmask, NULL) < 0) {
			object_error((t_object *)x, "error resetting sigmask (child): %s", strerror(errno));
			return 0;
		}

		if (*workingdir) {
			chdir(workingdir);
		}
		setenv("LC_ALL", "en_US.UTF-8", 1);
		execvp(cmd, argv);
	} else { // parent
		close(slavefd); // close the slave
		*masterfd_r = *masterfd_w = masterfd;

		if (pthread_sigmask(SIG_SETMASK, &oldmask, NULL) < 0) {
			object_error((t_object *)x, "error resetting sigmask: %s", strerror(errno));
			return masterfd;
		}
	}
	return masterfd;
#else
	SECURITY_ATTRIBUTES saAttr;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOW siStartInfo;
	HANDLE stdin_read = 0, stdin_write_tmp = 0, stdin_write = 0;
	HANDLE stdout_write = 0, stdout_read_tmp = 0, stdout_read = 0; 
	HANDLE stderr_write = 0;
	
	*masterfd_r = 0;
	*masterfd_w = 0;
	*ppid = 0;
	
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	
	// STDOUT
	if (!MyCreatePipeEx(&stdout_read_tmp, &stdout_write, &saAttr, 0, 0, 0, PIPE_NOWAIT)) {
		return 0;
	}
	// Dupe STDOUT to STDERR in case the child process closes STDERR for some reason
	if (!DuplicateHandle(GetCurrentProcess(), stdout_write, 
						 GetCurrentProcess(), &stderr_write, 
						 0, TRUE, DUPLICATE_SAME_ACCESS)) 
	{
		goto abkack;
	}
	// STDIN
	if (!MyCreatePipeEx(&stdin_read, &stdin_write_tmp, &saAttr, 0, 0, 0, PIPE_WAIT)) {
		goto abkack;
	}
	// Remove inheritance on the parent handles
	if (!DuplicateHandle(GetCurrentProcess(), stdout_read_tmp, 
						 GetCurrentProcess(), &stdout_read, 
						 0, FALSE, DUPLICATE_SAME_ACCESS)) 
	{
		goto abkack;
	}
	if (!DuplicateHandle(GetCurrentProcess(), stdin_write_tmp, 
						 GetCurrentProcess(), &stdin_write, 
						 0, FALSE, DUPLICATE_SAME_ACCESS)) 
	{
		goto abkack;
	}
	// Close the temp handles
	CLEAN_CLOSEHANDLE(stdout_read_tmp);
	CLEAN_CLOSEHANDLE(stdin_write_tmp);
	// Prep the process creation
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFOW));
	siStartInfo.cb = sizeof(STARTUPINFOW);
	siStartInfo.hStdError = stderr_write;
	siStartInfo.hStdOutput = stdout_write;
	siStartInfo.hStdInput = stdin_read;
	siStartInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	siStartInfo.wShowWindow = SW_HIDE;
	
	if (1) {
		char newcmd[MAX_MESSAGELEN];
		char workingdir[MAX_PATH];
		char cmdtemp[MAX_PATH_CHARS];
		char wdtemp[MAX_PATH_CHARS];
		WCHAR wnewcmd[MAX_MESSAGELEN];
		WCHAR wworkingdir[MAX_PATH];
		char *w;
		char **arg = argv;

		if (*arg) {
			char quotes = false;

			// 1st arg: shell
			strncpy(cmdtemp, *arg, MAX_PATH_CHARS);
			path_nameconform(cmdtemp, newcmd, PATH_STYLE_NATIVE, PATH_TYPE_ABSOLUTE);

			// 2nd arg: flag to shell
			++arg;
			if (*arg) {
				strncat(newcmd, " ", MAX_MESSAGELEN);
				strncat(newcmd, *arg, MAX_MESSAGELEN);
			}

			// addl args: cmd to execute
			while (++arg && *arg) {
				strncat(newcmd, " ", MAX_MESSAGELEN);
				// don't quote in CMD.exe (I will note that quoting in CreateProcess is a catastrophe)
				if (!quotes && !x->unicode) {
					strncat(newcmd, "\'", MAX_MESSAGELEN);
					quotes = true;
				}
				strncat(newcmd, *arg, MAX_MESSAGELEN);
			}
			if (quotes) {
				strncat(newcmd, "\'", MAX_MESSAGELEN);
			}
		}
		MultiByteToWideChar(CP_UTF8, 0, newcmd, (int)(strlen(newcmd) + 1), wnewcmd, MAX_MESSAGELEN);
		
		// WCHAR working dir
		if (x->wd != ps_nothing) {
			strncpy(wdtemp, x->wd->s_name, MAX_PATH_CHARS);
			path_nameconform(wdtemp, workingdir, PATH_STYLE_NATIVE, PATH_TYPE_ABSOLUTE);
			MultiByteToWideChar(CP_UTF8, 0, workingdir, (int)(strlen(workingdir) + 1), wworkingdir, MAX_PATH);
		}
		else { // use $HOME as the default WD
			size_t nSize = _countof(wworkingdir);
			if (!_wgetenv_s(&nSize, wworkingdir, (size_t)MAX_PATH, L"HOMEPATH")) {
				strncpy(cmdtemp, cmd, MAX_PATH_CHARS);
				path_nameconform(cmdtemp, workingdir, PATH_STYLE_NATIVE, PATH_TYPE_ABSOLUTE);
				if (w = strrchr(workingdir, '\\')) {
					*w = '\0';
				}
				MultiByteToWideChar(CP_UTF8, 0, workingdir, (int)(strlen(workingdir) + 1), wworkingdir, MAX_PATH);
			}
		}

		if (!CreateProcessW(NULL, wnewcmd, NULL, NULL, TRUE, 
			/*DETACHED_PROCESS | */ CREATE_NO_WINDOW | CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP,
						   NULL, wworkingdir, &siStartInfo, &piProcInfo)) 
		{
			goto abkack;
		}
	}
	
	// Close all the dead handles
	CLEAN_CLOSEHANDLE(stdout_write);
	CLEAN_CLOSEHANDLE(stdin_read);
	CLEAN_CLOSEHANDLE(stderr_write);
	
	CLEAN_CLOSEHANDLE(piProcInfo.hThread);
	
	// we did it!
	*masterfd_r = stdout_read;
	*masterfd_w = stdin_write;
	
	*ppid = piProcInfo.hProcess;
	
	return 1;
abkack:
	CLEAN_CLOSEHANDLE(stdout_write);
	CLEAN_CLOSEHANDLE(stdout_read_tmp);
	CLEAN_CLOSEHANDLE(stdout_read);
	
	CLEAN_CLOSEHANDLE(stdin_read);
	CLEAN_CLOSEHANDLE(stdin_write_tmp);
	CLEAN_CLOSEHANDLE(stdin_write);
	
	CLEAN_CLOSEHANDLE(stderr_write);
	return 0;
#endif
}

int shell_pipe_close(t_shell *x, t_fildes *masterfd_r, t_fildes *masterfd_w, t_procid pid, int *result)
{
#ifdef MAC_VERSION
    int status;
	
    if (result) *result=255;
	if (masterfd_r && *masterfd_r) {
		close(*masterfd_r);
		*masterfd_r = 0;
	}
	if (masterfd_w && *masterfd_w) {
		close(*masterfd_w);
		*masterfd_w = 0;
	}
	
	if (!pid) return 0;
    
	while (waitpid((pid_t)pid, &status, 0/*WNOHANG | WUNTRACED*/) < 0) {
		if (EINTR!=errno) 
			return 0;
	}
    if (result && WIFEXITED(status)) {
		*result=WEXITSTATUS(status);
	}
#else
	if (masterfd_r) {
		CLEAN_CLOSEHANDLE(*masterfd_r);
	}
	if (masterfd_w) {
		CLEAN_CLOSEHANDLE(*masterfd_w);
	}
	if (pid) {
		TerminateProcess(pid, 0);
		WaitForSingleObject(pid, INFINITE); // we could do this, but it's probably not necessary
		CLEAN_CLOSEHANDLE(pid);
	}
#endif
	return 0;
}
