#include "c74_min.h"

#define PY_INTERPRETER_IMPLEMENTATION
#include "mpy_interpreter.h"

using namespace c74::min;


class PythonExternal : public object<PythonExternal> {
public:
    MIN_DESCRIPTION {"Post to the Max Console."};
    MIN_TAGS        {"utilities"};
    MIN_AUTHOR      {"Cycling '74"};
    MIN_RELATED     {"print, jit.print, dict.print"};

    inlet<>  input  { this, "(bang) post greeting to the max console" };
    outlet<> output { this, "(anything) output the message which is posted to the max console" };

    pyjs::PythonInterpreter* py;

    PythonExternal() 
    {
        this->py = new pyjs::PythonInterpreter(this_class);
    }

    // define an optional argument for setting the message
    argument<symbol> greeting_arg { this, "greeting", "Initial value for the greeting attribute.",
        [this](const atom& arg) {
            greeting = arg;
        }
    };


    // the actual attribute for the message
    attribute<symbol> greeting { this, "greeting", "hello world",
        description {
            "Greeting to be posted. "
            "The greeting will be posted to the Max console when a bang is received."
        }
    };

    message<> import { this, "import", "Import Python module.",
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->import(args);
            return {};
        }
    };

    message<> eval { this, "eval", "Evaluate python expression.",
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->eval(args, &output);
            return {};
        }
    };

    message<> exec { this, "exec", "Execute python code.",
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->exec(args);
            return {};
        }
    };

    message<> execfile { this, "execfile", "Execute python code from file.",
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->execfile(args);
            return {};
        }
    };

    message<> call { this, "call", "Call python function with atom arguments.",
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->call(args, &output);
            return {};
        }
    };

    message<> assign { this, "assign", "Assign python variable to atom expression.",
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->assign(args);
            return {};
        }
    };

    message<> code { this, "code", "Process arbitrary python code.",
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->code(args, &output);
            return {};
        }
    };    

    message<> anything { this, "anything", "Process arbitrary python code.",
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->anything(args, &output);
            return {};
        }
    };

    // respond to the bang message to do something
    message<> bang { this, "bang", "Post the greeting.",
        [this](const atoms& args, const int inlet) -> atoms {
            symbol the_greeting = greeting;  // fetch the symbol itself from the attribute named greeting
            cout << the_greeting << endl;    // post to the max console
            output.send(the_greeting);       // send out our outlet
            return {};
        }
    };


    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        // MIN_FUNCTION {
        [this](const atoms& args, const int inlet) -> atoms {
            this->py->import("sys");
            this->py->eval("sys.version", &output);
            // cout << "hello world" << endl;
            return {};
        }
    };

};


MIN_EXTERNAL(PythonExternal);
