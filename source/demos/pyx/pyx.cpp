#include "maxcpp6.h"

#define PY_INTERPRETER_IMPLEMENTATION
#include "py_interpreter.h"

class PythonExternal : public MaxCpp6<PythonExternal> {

private:
	pyjs::PythonInterpreter* py;

public:
	PythonExternal(t_symbol * sym, long ac, t_atom * av)
	{ 
		setupIO(2, 2); // inlets / outlets
		this->py = new pyjs::PythonInterpreter(m_class);
	}
	~PythonExternal()
	{
		delete this->py;
	}	
	
	// methods:
	void bang(long inlet)
	{ 
		outlet_bang(m_outlets[0]);
	}

	void import(long inlet, t_symbol* v)
	{
		this->py->import(v);
	}

	void eval(long inlet, t_symbol* v)
	{
		this->py->eval(v, m_outlets[0]);
	}

	void exec(long inlet, t_symbol* v)
	{
		this->py->exec(v);
	}

	void execfile(long inlet, t_symbol* v)
	{
		this->py->execfile(v);
	}

	void call(long inlet, t_symbol* s, long ac, t_atom* av)
	{
	    this->py->call(s, ac, av, m_outlets[0]);
	}

	void assign(long inlet, t_symbol* s, long ac, t_atom* av)
	{
	    this->py->assign(s, ac, av);
	}

	void code(long inlet, t_symbol* s, long ac, t_atom* av)
	{
	    this->py->code(s, ac, av, m_outlets[0]);
	}

	void anything(long inlet, t_symbol* s, long ac, t_atom* av)
	{
	    this->py->anything(s, ac, av, m_outlets[0]);
	}

	void pipe(long inlet, t_symbol* s, long ac, t_atom* av)
	{
	    this->py->pipe(s, ac, av, m_outlets[0]);
	}

	void testfloat(long inlet, double v)
	{
		post("inlet %ld float %f", inlet, v);
		outlet_float(m_outlets[0], v);
	}
	
	void testint(long inlet, long v)
	{
		post("inlet %ld int %ld", inlet, v);
		outlet_int(m_outlets[0], v);
	}
	
	void test(long inlet, t_symbol * s, long ac, t_atom * av)
	{ 
		outlet_anything(m_outlets[1], gensym("test"), ac, av); 
	}

};

C74_EXPORT int main(void) {
	// create a class with the given name:
	PythonExternal::makeMaxClass("pyx");
	REGISTER_METHOD(PythonExternal, bang);
	REGISTER_METHOD_SYM(PythonExternal, import);
	REGISTER_METHOD_SYM(PythonExternal, eval);
	REGISTER_METHOD_SYM(PythonExternal, exec);
	REGISTER_METHOD_SYM(PythonExternal, execfile);

	REGISTER_METHOD_GIMME(PythonExternal, call);
	REGISTER_METHOD_GIMME(PythonExternal, assign);
	REGISTER_METHOD_GIMME(PythonExternal, code);
	REGISTER_METHOD_GIMME(PythonExternal, anything);
	REGISTER_METHOD_GIMME(PythonExternal, pipe);

	REGISTER_METHOD_FLOAT(PythonExternal, testfloat);
	REGISTER_METHOD_DEFLONG(PythonExternal, testint);
	REGISTER_METHOD_GIMME(PythonExternal, test);
	
	// these are for handling float/int messages directly (no method name in Max):
	REGISTER_INLET_FLOAT(PythonExternal, testfloat);
	REGISTER_INLET_LONG(PythonExternal, testint);
}