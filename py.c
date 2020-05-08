/**
    @file
    py.c - experiment in max object for calling python code

    this object has 2 inlets and one outlet
    it responds to ints in its inlets and the 'bang' message in the left inlet
    it responds to the 'assistance' message sent by Max when the mouse is positioned over an inlet or outlet
        (including an assistance method is optional, but strongly sugggested)
    it adds its input values together and outputs their sum

    @ingroup    examples
*/

#include "ext.h"            // you must include this - it contains the external object's link to available Max functions
#include "ext_obex.h"       // this is required for all objects using the newer style for writing objects.

/* python */
#define PY_SSIZE_T_CLEAN
#include <Python.h>


typedef struct _py {    // defines our object's internal variables for each instance in a patch
    t_object p_ob;          // object header - ALL objects MUST begin with this...
    long p_value0;          // int value - received from the left inlet and stored internally for each object instance
    long p_value1;          // int value - received from the right inlet and stored internally for each object instance
    void *p_outlet;         // outlet creation - inlets are automatic, but objects must "own" their own outlets
} t_py;


// these are prototypes for the methods that are defined below
void py_bang(t_py *x);
void py_int(t_py *x, long n);
void py_in1(t_py *x, long n);
void py_assist(t_py *x, void *b, long m, long a, char *s);
void *py_new(long n);


t_class *py_class;      // global pointer to the object class - so max can reference the object


//--------------------------------------------------------------------------

void ext_main(void *r)
{
    t_class *c;

    c = class_new("py", (method)py_new, (method)NULL, sizeof(t_py), 0L, A_DEFLONG, 0);
    // class_new() loads our external's class into Max's memory so it can be used in a patch
    // py_new = object creation method defined below

    class_addmethod(c, (method)py_bang,     "bang",     0);         // the method it uses when it gets a bang in the left inlet
    class_addmethod(c, (method)py_int,      "int",      A_LONG, 0); // the method for an int in the left inlet (inlet 0)
    class_addmethod(c, (method)py_in1,      "in1",      A_LONG, 0); // the method for an int in the right inlet (inlet 1)
    // "ft1" is the special message for floats
    class_addmethod(c, (method)py_assist,   "assist",   A_CANT, 0); // (optional) assistance method needs to be declared like this

    class_register(CLASS_BOX, c);
    py_class = c;

    post("py object loaded...",0);  // post any important info to the max window when our class is loaded
}


//--------------------------------------------------------------------------

void *py_new(long n)        // n = int argument typed into object box (A_DEFLONG) -- defaults to 0 if no args are typed
{
    t_py *x;                // local variable (pointer to a t_py data structure)

    x = (t_py *)object_alloc(py_class); // create a new instance of this object

    intin(x,1);                 // create a second int inlet (leftmost inlet is automatic - all objects have one inlet by default)
    x->p_outlet = intout(x);    // create an int outlet and assign it to our outlet variable in the instance's data structure

    x->p_value0 = 0;            // set initial (default) left operand value in the instance's data structure
    x->p_value1 = n;            // set initial (default) right operand value (n = variable passed to py_new)

    post(" new py object instance added to patch...", 0); // post important info to the max window when new instance is created

    return(x);                  // return a reference to the object instance
}


//--------------------------------------------------------------------------

void py_assist(t_py *x, void *b, long m, long a, char *s) // 4 final arguments are always the same for the assistance method
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"Sum of Left and Right Inlets");
    else {
        switch (a) {
        case 0:
            sprintf(s,"Inlet %ld: Left Operand (Causes Output)", a);
            break;
        case 1:
            sprintf(s,"Inlet %ld: Right Operand (Added to Left)", a);
            break;
        }
    }
}


void py_bang(t_py *x)           // x = reference to this instance of the object
{
    long sum;                           // local variable for this method

    sum = x->p_value0+x->p_value1;      // add left and right operands
    outlet_int(x->p_outlet, sum);       // send out the sum on bang
}


void py_int(t_py *x, long n)    // x = the instance of the object; n = the int received in the left inlet
{
    x->p_value0 = n;                    // store left operand value in instance's data structure
    py_bang(x);                     // ... call the bang method to sum and send out our outlet
}


void py_in1(t_py *x, long n)    // x = the instance of the object, n = the int received in the right inlet
{
    x->p_value1 = n;                    // just store right operand value in instance's data structure and do nothing else
}


