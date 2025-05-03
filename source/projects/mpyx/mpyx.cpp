#include "c74_min.h"

#define MPY_INTERPRETER_IMPLEMENTATION
#include "mpy_interpreter.h"

using namespace c74::min;


class PythonExternal : public object<PythonExternal> {
public:
    MIN_DESCRIPTION {"Run Python code in Max/MSP."};
    MIN_TAGS        {"python"};
    MIN_AUTHOR      {"S. Alireza"};

    inlet<>  input  { this, "(anything) receives messages and python code" };
    outlet<> output { this, "(anything) output results of processing python code" };

    PythonExternal(const atoms& args = {})
    {
        this->py = std::make_unique<pyjs::PythonInterpreter>(this_class);

        if (args.size() > 0) {
            symbol name = args[0];
            this->py->set_name(name);
        }
    }

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

    message<> bang { this, "bang", "Test things here.",
        [this](const atoms& args, const int inlet) -> atoms {
            output.send("bang");
            return {};
        }
    };

    message<> instance_name { this, "name", "Get name of object.",
        [this](const atoms& args, const int inlet) -> atoms {
            if (args.size() == 0) {
                symbol _name = this->py->get_name();
                output.send(_name);
            } else {
                cout << "args.size: " << args.size() << endl;
                symbol _name = symbol(args[0]);
                this->py->set_name(_name);
            }
            return {};
        }
    };

    // post to max window == but only when the class is loaded the first time
    message<> maxclass_setup { this, "maxclass_setup",
        [this](const atoms& args, const int inlet) -> atoms {
            cout << "python3 external created" << endl;
            return {};
        }
    };

private:
    std::unique_ptr<pyjs::PythonInterpreter> py;

};


MIN_EXTERNAL(PythonExternal);
