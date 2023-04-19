//
// 	unit/integration test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"
#include "z_dsp.h"

#define MAXSAMPLECOUNT 64


/** like snapshot~ but a little easier to get predictable results in the test context.
	for example, snapshot~ will return a result when banged even if dsp has never finished executing a single vector. 
 */
typedef struct _testsample {
	t_pxobject	x_ob;						///< header
	t_object	*x_patcher;					///< the patcher in which the object exists -- assumed to be a top-level patcher
	t_test		*x_test;					///< test object that is calling this assertion
	void		*x_outlet;					///< float/list for sampled values
	t_clock		*x_clock;					///< clock for pushing the data onto the scheduler from the audio thread
	long		x_offset;					///< attr: offset into the vector at which to grab the sample
	long		x_vectoroffset;				///< how many vectors to wait before grabbing a sample
	long		x_vectorcountdown;			///< the counter of how many vectors remain to pass by before grabbing a sample
	long		x_samplecount;				///< attr: number of samples to grab
	double		x_samples[MAXSAMPLECOUNT];	///< samples to return
	long		x_hasrun;					///< have we run our perform method and returned output yet?
	long		x_autorun;					///> run automatically? default is yes
} t_testsample;


// prototypes
void*	testsample_new(t_symbol *s, long argc, t_atom *argv);
void 	testsample_free(t_testsample *x);
void	testsample_assist(t_testsample *x, void *b, long m, long a, char *s);
void	testsample_bang(t_testsample *x);
void	testsample_tick(t_testsample *x);
void	testsample_dsp64(t_testsample *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);


// class variables
static t_class		*s_testsample_class = NULL;


/************************************************************************/

void testsample_classinit(void)
{
	t_class *c = class_new("test.sample~",
				  (method)testsample_new,
				  (method)testsample_free,
				  sizeof(t_testsample),
				  (method)NULL,
				  A_GIMME,
				  0L);
		
	class_addmethod(c, (method)testsample_bang,		"bang",			0);
	class_addmethod(c, (method)testsample_dsp64,	"dsp64",		A_CANT, 0);
	class_addmethod(c, (method)testsample_assist,	"assist",		A_CANT, 0);
	
	CLASS_ATTR_LONG(c, "offset", 0, t_testsample, x_offset);
	CLASS_ATTR_LONG(c, "vectoroffset", 0, t_testsample, x_vectoroffset);
	CLASS_ATTR_LONG(c, "count", 0, t_testsample, x_samplecount);
	CLASS_ATTR_LONG(c, "autorun", 0, t_testsample, x_autorun);
	
	class_dspinit(c);
	class_register(_sym_box, c);
	s_testsample_class = c;
}


/************************************************************************/


void* testsample_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testsample	*x = (t_testsample*)object_alloc(s_testsample_class);
	
	if (x) {
		dsp_setup((t_pxobject *)x, 1);
		x->x_clock = (t_clock*)clock_new(x, (method)testsample_tick);
		x->x_outlet = outlet_new(x, NULL);
		x->x_test = (t_test*)gensym("#T")->s_thing;
		x->x_samplecount = 1;
		x->x_autorun = true;
		attr_args_process(x, (short)argc, argv);
		x->x_vectorcountdown = x->x_vectoroffset;
		if (!x->x_autorun)
			x->x_hasrun = true; // if we aren't going to automatically fire, then pretend we already did it
	}
	autocolorbox((t_object*)x);
	return x;
}


void testsample_free(t_testsample *x)
{
	dsp_free((t_pxobject*)x);
	clock_free(x->x_clock);
}


#pragma mark -
/************************************************************************/

void testsample_assist(t_testsample *x, void *b, long m, long a, char *s)
{
	strcpy(s, "log messages to the test result, or to the max window");
}


void testsample_tick(t_testsample *x)
{
	if (x->x_samplecount == 1)
		outlet_float(x->x_outlet, x->x_samples[0]);
	else {
		t_atom	a[MAXSAMPLECOUNT];
		int		i;
		
		for (i=0; i < x->x_samplecount; i++)
			atom_setfloat(a+i, x->x_samples[i]);
		outlet_anything(x->x_outlet, _sym_list, (short)x->x_samplecount, a);
	}
}


void testsample_bang(t_testsample *x)
{
	x->x_vectorcountdown = x->x_vectoroffset;
	x->x_hasrun = false;
}


void testsample_perform64(t_testsample *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
	if (x->x_vectorcountdown) {
		x->x_vectorcountdown--;
		return;
	}
	if (!x->x_hasrun) {
		memcpy(x->x_samples, ins[0]+x->x_offset, sizeof(t_double) * x->x_samplecount);		
		clock_delay(x->x_clock, 0);
		x->x_hasrun = true;
	}
}


void testsample_dsp64(t_testsample *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
	dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)testsample_perform64, 0, NULL);
}

