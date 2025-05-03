/*
TODO:
    - eval needs try catch

*/

#include "c74_min.h"
#include <pybind11/embed.h> // everything needed for embedding

using namespace c74::min;

namespace py = pybind11;
using namespace py::literals;


// Global Constants
// ------------------------------------------------------------------
static int GLOBAL_OBJ_COUNT = 0; // when 0 then free interpreter


// Functions
// ------------------------------------------------------------------


// Classes
// ------------------------------------------------------------------

class pymx : public object<pymx> {
private:
    // object attributes
    symbol p_name;          // unique object name

    // python-related
    symbol p_pythonpath;    // path to python directory
    bool p_debug;           // bool to switch per-object debug state

	// python interpreter related
    py::scoped_interpreter guard;
    py::dict scope;


public:
    MIN_DESCRIPTION	{"eval python code."};
    MIN_TAGS		{"languages"};
    MIN_AUTHOR		{"shakfu"};
    MIN_RELATED		{"pymx"};

    inlet<>  input	   {this, "(bang) main input." };
    outlet<> output	   {this, "(anything) main output." };
	outlet<> success_outlet {this, "(bang) bangs with success."};
	outlet<> error_outlet   {this, "(bang) bangs with error."};


    // constructor
    pymx(const atoms& args = {}) {
        if (args.size() > 0)
            p_name = args[0];
        if (args.size() > 1)
            p_pythonpath = args[1];

		// initiate the the python interpreter
		// create a globals namespace scope object
		scope = py::module_::import("__main__").attr("__dict__");
    }

    // destructor
    ~pymx() {

	}

    // utility methods
    void bang_success(void) { success_outlet.send("bang"); };
    void bang_failure(void) { error_outlet.send("bang"); };


	// the actual attribute for the object
	attribute<symbol> greeting {this, "greeting", "hello py",
		description {"Greeting to be posted. "
					 "The greeting will be posted to the Max console when a bang is received."}};


	// define an optional argument for setting the message
    argument<symbol> greeting_arg {this, "greeting", "Initial value for the greeting attribute.",
        MIN_ARGUMENT_FUNCTION {
            greeting = arg;
        }
    };

    message<> import {this, "import", "import a python module.",
        MIN_FUNCTION {
            try {
                std::string s = (std::string)args[0];
                const char *name = s.c_str();
				py::module_ py_module = py::module_::import(name);
				scope[name] = py_module;
				cout << s << " module imported." << endl;
				bang_success();
			}
            catch (std::exception&) {
				cerr << "could not import: " << args[0] << endl;
				bang_failure();
			}
			return {};
        }
    };

    message<> exec {this, "exec", "exec python code.",
        MIN_FUNCTION {
			try {
                py::exec((std::string)args[0], scope);
                bang_success();

            }
			catch (std::exception&) {
				cerr << "could not exec: " << args[0] << endl;
                bang_failure();
			}
			return {};
        }
	};

	template<typename T>
	void py_object_to_atom_out(const py::object x) {
        try {
            T special_result = x.cast<T>();
            output.send((atom)special_result);
            bang_success();
        }
        catch (py::cast_error) {
            cerr << "py object could not be cast" << endl;
            bang_failure();
        }
	};

	message<> eval {this, "eval", "eval python code.",
		MIN_FUNCTION {
            if (args[0].a_type == c74::max::A_SYM) {
                try {
                    py::object result = py::eval((std::string)args[0], scope);
                    if (py::isinstance<py::int_>(result)) {
                        py_object_to_atom_out<int>(result);
                    } 
                    else if (py::isinstance<py::float_>(result)) {
                        py_object_to_atom_out<float>(result);
                    }
                    else if (py::isinstance<py::str>(result)) {
                        py_object_to_atom_out<string>(result);
                    }
                    // TODO: more type conversion to implement
                    // list, tuple, set, dict, etc..
                    else {
                        cout << "unable to convert py object to atom" << endl;
                    }
                }
                catch (std::exception&) {
                    cerr << "could not eval: " << args[0] << endl;
                    bang_failure();
                }
            }
			return {};
        }
    };

    message<> number {this, "number", "Send any kind of number.",
        MIN_FUNCTION {
            cout << args[0] << endl;
            output.send(args);
			return {};
		}
    };

	// respond to the bang message to do something
    message<> bang {this, "bang", "Post the greeting.",
        MIN_FUNCTION {

            // print name
            cout << "p_name: " << p_name << endl;

            //py::scoped_interpreter guard{};

            auto locals = py::dict("name"_a="pybind11", "number"_a=42);
            py::exec(R"(
                message = "Hello, {name}! The answer is {number}".format(**locals())
            )", py::globals(), locals);

            auto message = locals["message"].cast<std::string>();

            cout << message << endl;    // post to the max console

            symbol the_greeting = greeting;    // fetch the symbol itself from the attribute named greeting

            cout << the_greeting << endl;    // post to the max console
            output.send(the_greeting);       // send out our outlet
            bang_success();
            return {};
        }
    };


    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup {this, "maxclass_setup",
        MIN_FUNCTION {
            cout << "first time" << endl;
            return {};
        }
    };
};


MIN_EXTERNAL(pymx);
