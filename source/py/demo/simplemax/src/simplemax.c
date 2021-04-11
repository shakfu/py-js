/**
	@file
	simplemax - a max object shell
	jeremy bernstein - jeremy@bootsquad.com

	@ingroup	examples
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <libgen.h>
#endif

////////////////////////// object struct
typedef struct _simplemax
{
	t_object					ob;			// the object itself (must be first)
} t_simplemax;

///////////////////////// function prototypes
//// standard set
void *simplemax_new(t_symbol *s, long argc, t_atom *argv);
void simplemax_free(t_simplemax *x);
void simplemax_assist(t_simplemax *x, void *b, long m, long a, char *s);
void simplemax_bang(t_simplemax* x);

//////////////////////// global class pointer variable
void *simplemax_class;


void ext_main(void *moduleRef)
{
	t_class *c;

	c = class_new("simplemax", (method)simplemax_new, (method)simplemax_free, (long)sizeof(t_simplemax),
				  0L /* leave NULL!! */, A_GIMME, 0);

	/* you CAN'T call this from the patcher */
	class_addmethod(c, (method)simplemax_assist,			"assist",		A_CANT, 0);
    class_addmethod(c, (method)simplemax_bang, "bang", 0);

    class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	simplemax_class = c;

#ifdef __APPLE__
    char appname[256];
    appname[0] = "";
    CFURLRef url = CFBundleCopyBundleURL(moduleRef);
    CFStringRef string = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	// gives path (still including escape chars)
	// CFStringRef string = CFURLCopyPath(url);

	// gives basename
	//	CFStringRef string = CFURLCopyLastPathComponent(url);
	CFStringGetCString(string, appname, 256, 0);
    post("external path: %s", appname);
    CFRelease(string);
#else
    char* appname_win;
    GetModuleFileName(moduleRef, (LPCH)appname, sizeof(appname));
    appname_win = strrchr(appname, '\\');

    post("external path: %s", appname);
    post("appname: %s", appname_win);

#endif

	post("I am the simplemax object");
}

void simplemax_assist(t_simplemax *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		sprintf(s, "I am inlet %ld", a);
	}
	else {	// outlet
		sprintf(s, "I am outlet %ld", a);
	}
}

void simplemax_free(t_simplemax *x)
{
	;
}


void *simplemax_new(t_symbol *s, long argc, t_atom *argv)
{
	t_simplemax *x = NULL;
	long i;

	if ((x = (t_simplemax *)object_alloc(simplemax_class))) {
		object_post((t_object *)x, "a new %s object was instantiated: %p", s->s_name, x);
		object_post((t_object *)x, "it has %ld arguments", argc);

		for (i = 0; i < argc; i++) {
			if ((argv + i)->a_type == A_LONG) {
				object_post((t_object *)x, "arg %ld: long (%ld)", i, atom_getlong(argv+i));
			} else if ((argv + i)->a_type == A_FLOAT) {
				object_post((t_object *)x, "arg %ld: float (%f)", i, atom_getfloat(argv+i));
			} else if ((argv + i)->a_type == A_SYM) {
				object_post((t_object *)x, "arg %ld: symbol (%s)", i, atom_getsym(argv+i)->s_name);
			} else {
				object_error((t_object *)x, "forbidden argument");
			}
		}
	}
	return (x);
}

void simplemax_bang(t_simplemax *x)
{
    object_post((t_object*)x, "hello max");

    // Check for Environment (Max or Standalone)
    {
        char appname[256];
        int isMax = 0;
        appname[0] = "";

#ifdef __APPLE__
        CFBundleRef bun = CFBundleGetMainBundle();
        CFURLRef url = CFBundleCopyBundleURL(bun);
        CFStringRef string = CFURLCopyLastPathComponent(url);
        CFStringGetCString(string, appname, 256, 0);

        object_post((t_object*)x, "appname: %s", appname);
        
		isMax = strcmp(appname, "Max.app");
        CFRelease(string);
#else
        char* appname_win;
        HMODULE hMod;
        hMod = GetModuleHandle(NULL);
        GetModuleFileName(hMod, (LPCH)appname, sizeof(appname));

		appname_win = strrchr(appname, '\');
		isMax =	strcmp(appname_win+1, "max.exe");	// Max 4 had a lower case first letter
		if(isMax)	// the above did not match
			isMax =	strcmp(appname_win+1, "Max.exe");	// Max 5 has an upper case first letter
														// post("TT IsMax: %i AppName: %s", isMax, appname_win+1);
#endif
        int is_standalone = (isMax != 0);
    }

    char filename[MAX_FILENAME_CHARS];
    short path;
    t_fourcc outtype;
    t_fourcc filetype;
        
	strncpy_zero(filename, "simplemax.mxo", MAX_FILENAME_CHARS);
	short result = locatefile_extended(filename, &path, &outtype,
										&filetype, 1);
	if (result == 0) {
        object_post((t_object*)x, "simplemax.mxo found");
    } else {
        object_post((t_object*)x, "simplemax.mxo not found");
    }
}