//
// 	unit/integration test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"
#include "z_dsp.h"
#ifdef WIN_VERSION
#include <float.h>
#endif
#include <algorithm>

/** like == but can compare floating-point numbers while tolerating the floating-point (im)precision.
 */
typedef struct _testequals {
	t_pxobject	x_ob;				///< header
	void		*x_outlet;			///< float/list for sampled values
	void		*x_inlet;			///< for setting the operand
	double		x_operand;			///< the number against which to test input
	long		x_tolerance;		///< number of floating-point representations around the specified operand to consider as "equal"
	long		x_single_precision;	///< operate on 32-bit floats rather than 64-bit doubles
} t_testequals;


// prototypes
void*	testequals_new(t_symbol *s, long argc, t_atom *argv);
void 	testequals_free(t_testequals *x);
void	testequals_assist(t_testequals *x, void *b, long m, long a, char *s);
void	testequals_float(t_testequals *x, double f);


// class variables
static t_class		*s_testequals_class = NULL;


/************************************************************************/

void testequals_classinit(void)
{
	t_class *c = class_new("test.equals", (method)testequals_new, (method)testequals_free, sizeof(t_testequals), (method)NULL, A_GIMME, 0);
	
	class_addmethod(c, (method)testequals_float,	"float",	A_FLOAT, 0);
	class_addmethod(c, (method)testequals_assist,	"assist",	A_CANT, 0);
	
	CLASS_ATTR_LONG(c, "tolerance", 0, t_testequals, x_tolerance);
	CLASS_ATTR_LONG(c, "single_precision", 0, t_testequals, x_single_precision);
	
	class_register(_sym_box, c);
	s_testequals_class = c;
}


/************************************************************************/


void* testequals_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testequals	*x = (t_testequals*)object_alloc(s_testequals_class);
	long			attrstart = attr_args_offset((short)argc, argv);

	if (attrstart)
		x->x_operand = atom_getfloat(argv);
	
	x->x_outlet = outlet_new(x, NULL);
	x->x_inlet = proxy_new(x, 1, NULL);
	x->x_tolerance = 2;
#ifdef C74_X64
	x->x_single_precision = false;
#else
	x->x_single_precision = true;
#endif
	
	attr_args_process(x, (short)argc, argv);
	autocolorbox((t_object*)x);
	return x;
}


void testequals_free(t_testequals *x)
{
	object_free(x->x_inlet);
}


#pragma mark -
/************************************************************************/

void testequals_assist(t_testequals *x, void *b, long m, long a, char *s)
{
	strcpy(s, "log messages to the test result, or to the max window");
}


// see http://realtimecollisiondetection.net/blog/?p=89 regarding the comparison

static const double k_epsilon64 = DBL_EPSILON;
static const float k_epsilon32 = FLT_EPSILON;

t_bool testequals_equivalent(double a, double b, long tolerance, long single_precision)
{
	if (single_precision) {
		float tol = tolerance * k_epsilon32;
		float maxab = (std::max)(fabsf((float)a), fabsf((float)b));
		
		if ( fabsf((float)a - (float)b) <= tol * (std::max)(1.0f, maxab) )
			return true;
	}
	else {
		double tol = tolerance * k_epsilon64;
		double maxab = (std::max)(fabs(a), fabs(b));
		
		if ( fabs(a - b) <= tol * (std::max)(1.0, maxab) )
			return true;
	}
	return false;
}


void testequals_float(t_testequals *x, double f)
{
	if (proxy_getinlet((t_object*)x) == 1)
		x->x_operand = f;
	else
		outlet_int(x->x_outlet, testequals_equivalent(x->x_operand, f, x->x_tolerance, x->x_single_precision));
}

