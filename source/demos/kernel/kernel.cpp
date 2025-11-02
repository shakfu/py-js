// kernel.cpp - Max external embedding a Jupyter kernel using xeus

#include "ext.h"
#include "ext_obex.h"

#include <string>
#include <sstream>
#include <memory>

#include "xeus/xinterpreter.hpp"
#include "xeus/xkernel.hpp"
#include "xeus/xkernel_configuration.hpp"
#include "nlohmann/json.hpp"

using namespace xeus;
namespace nl = nlohmann;

// Forward declarations
typedef struct _kernel t_kernel;
class max_interpreter;

// Max external structure
typedef struct _kernel {
    t_object ob;                          // object header (must be first)
    void* outlet_left;                    // left outlet for output
    void* outlet_right;                   // right outlet for status
    t_symbol* name;                       // unique object name
    long debug;                           // debug flag
    std::unique_ptr<max_interpreter>* interpreter;  // pointer to interpreter
    std::unique_ptr<xkernel>* kernel;     // pointer to kernel
} t_kernel;

// Custom interpreter for Max integration
class max_interpreter : public xinterpreter {
public:
    max_interpreter(t_kernel* x) : m_max_obj(x), m_execution_count(0) {
        register_interpreter(this);
    }

    virtual ~max_interpreter() = default;

private:
    void configure_impl() override {
        // Configuration logic
        if (m_max_obj && m_max_obj->debug) {
            object_post((t_object*)m_max_obj, "kernel: interpreter configured");
        }
    }

    void execute_request_impl(send_reply_callback cb,
                              int execution_counter,
                              const std::string& code,
                              execute_request_config config,
                              nl::json user_expressions) override {

        m_execution_count = execution_counter;

        if (m_max_obj && m_max_obj->debug) {
            object_post((t_object*)m_max_obj, "kernel: executing code: %s", code.c_str());
        }

        // Create reply
        nl::json reply;
        reply["status"] = "ok";
        reply["execution_count"] = execution_counter;
        reply["user_expressions"] = nl::json::object();

        // Publish execution result
        if (!config.silent) {
            nl::json pub_data;
            pub_data["text/plain"] = "Executed in Max: " + code;
            publish_execution_result(execution_counter, pub_data, nl::json::object());
        }

        // Send to Max outlet
        if (m_max_obj && m_max_obj->outlet_left) {
            t_atom atoms[2];
            atom_setsym(&atoms[0], gensym("execute"));
            atom_setsym(&atoms[1], gensym(code.c_str()));
            outlet_anything(m_max_obj->outlet_left, gensym("code"), 2, atoms);
        }

        cb(reply);
    }

    nl::json complete_request_impl(const std::string& code,
                                   int cursor_pos) override {
        nl::json reply;
        reply["matches"] = nl::json::array();
        reply["cursor_start"] = cursor_pos;
        reply["cursor_end"] = cursor_pos;
        reply["status"] = "ok";
        return reply;
    }

    nl::json inspect_request_impl(const std::string& code,
                                  int cursor_pos,
                                  int detail_level) override {
        nl::json reply;
        reply["status"] = "ok";
        reply["found"] = false;
        reply["data"] = nl::json::object();
        reply["metadata"] = nl::json::object();
        return reply;
    }

    nl::json is_complete_request_impl(const std::string& code) override {
        nl::json reply;
        reply["status"] = "complete";
        return reply;
    }

    nl::json kernel_info_request_impl() override {
        nl::json reply;
        reply["protocol_version"] = "5.3";
        reply["implementation"] = "max_kernel";
        reply["implementation_version"] = "0.1.0";

        nl::json language_info;
        language_info["name"] = "max";
        language_info["version"] = "8.0";
        language_info["mimetype"] = "text/x-maxmsp";
        language_info["file_extension"] = ".maxpat";

        reply["language_info"] = language_info;
        reply["banner"] = "Max/MSP Jupyter Kernel v0.1.0";
        reply["help_links"] = nl::json::array();

        return reply;
    }

    void shutdown_request_impl() override {
        if (m_max_obj && m_max_obj->debug) {
            object_post((t_object*)m_max_obj, "kernel: shutdown requested");
        }
    }

    t_kernel* m_max_obj;
    int m_execution_count;
};

// Method prototypes
void* kernel_new(t_symbol* s, long argc, t_atom* argv);
void kernel_free(t_kernel* x);
void kernel_assist(t_kernel* x, void* b, long m, long a, char* s);
void kernel_bang(t_kernel* x);
void kernel_eval(t_kernel* x, t_symbol* s, long argc, t_atom* argv);
void kernel_start(t_kernel* x);
void kernel_stop(t_kernel* x);
void kernel_info(t_kernel* x);

// Global class pointer
static t_class* kernel_class = NULL;

// Main entry point
void ext_main(void* r)
{
    t_class* c;

    c = class_new("kernel",
                  (method)kernel_new,
                  (method)kernel_free,
                  (long)sizeof(t_kernel),
                  0L,
                  A_GIMME,
                  0);

    class_addmethod(c, (method)kernel_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)kernel_bang,   "bang",   0);
    class_addmethod(c, (method)kernel_eval,   "eval",   A_GIMME, 0);
    class_addmethod(c, (method)kernel_start,  "start",  0);
    class_addmethod(c, (method)kernel_stop,   "stop",   0);
    class_addmethod(c, (method)kernel_info,   "info",   0);

    CLASS_ATTR_SYM(c, "name", 0, t_kernel, name);
    CLASS_ATTR_LABEL(c, "name", 0, "Unique Name");
    CLASS_ATTR_BASIC(c, "name", 0);

    CLASS_ATTR_LONG(c, "debug", 0, t_kernel, debug);
    CLASS_ATTR_LABEL(c, "debug", 0, "Debug Mode");
    CLASS_ATTR_STYLE(c, "debug", 0, "onoff");

    class_register(CLASS_BOX, c);
    kernel_class = c;

    post("kernel: Max/MSP Jupyter Kernel v0.1.0");
}

void kernel_assist(t_kernel* x, void* b, long m, long a, char* s)
{
    if (m == ASSIST_INLET) {
        snprintf(s, 256, "Messages to kernel");
    }
    else {
        switch (a) {
        case 0:
            snprintf(s, 256, "Kernel output");
            break;
        case 1:
            snprintf(s, 256, "Status messages");
            break;
        }
    }
}

void kernel_free(t_kernel* x)
{
    if (x->kernel) {
        try {
            (*x->kernel)->stop();
        } catch (...) {
            // Ignore exceptions during cleanup
        }
        delete x->kernel;
        x->kernel = nullptr;
    }

    if (x->interpreter) {
        delete x->interpreter;
        x->interpreter = nullptr;
    }
}

void* kernel_new(t_symbol* s, long argc, t_atom* argv)
{
    t_kernel* x = NULL;

    if ((x = (t_kernel*)object_alloc(kernel_class))) {
        // Create outlets (right to left)
        x->outlet_right = outlet_new(x, NULL);
        x->outlet_left = outlet_new(x, NULL);

        // Initialize attributes
        x->name = gensym("");
        x->debug = 0;
        x->interpreter = nullptr;
        x->kernel = nullptr;

        // Process attributes
        attr_args_process(x, argc, argv);

        // Create interpreter
        try {
            x->interpreter = new std::unique_ptr<max_interpreter>(
                new max_interpreter(x)
            );

            if (x->debug) {
                object_post((t_object*)x, "kernel: created with name '%s'",
                           x->name->s_name);
            }
        } catch (const std::exception& e) {
            object_error((t_object*)x, "kernel: failed to create interpreter: %s",
                        e.what());
        }
    }
    return (x);
}

void kernel_bang(t_kernel* x)
{
    if (x->outlet_right) {
        outlet_bang(x->outlet_right);
    }
}

void kernel_eval(t_kernel* x, t_symbol* s, long argc, t_atom* argv)
{
    if (!x->interpreter || !(*x->interpreter)) {
        object_error((t_object*)x, "kernel: interpreter not initialized");
        return;
    }

    // Concatenate atoms into a string
    std::stringstream ss;
    for (long i = 0; i < argc; i++) {
        if (atom_gettype(argv + i) == A_SYM) {
            ss << atom_getsym(argv + i)->s_name;
        } else if (atom_gettype(argv + i) == A_LONG) {
            ss << atom_getlong(argv + i);
        } else if (atom_gettype(argv + i) == A_FLOAT) {
            ss << atom_getfloat(argv + i);
        }
        if (i < argc - 1) ss << " ";
    }

    std::string code = ss.str();

    if (x->debug) {
        object_post((t_object*)x, "kernel: eval '%s'", code.c_str());
    }

    // Execute the code through the interpreter
    try {
        auto callback = [x](nl::json reply) {
            if (x->debug) {
                object_post((t_object*)x, "kernel: execution complete");
            }
        };

        execute_request_config config;
        config.silent = false;
        config.store_history = true;
        config.allow_stdin = false;

        // Note: This is a simplified version. Full implementation would need
        // proper request context handling
        if (x->outlet_right) {
            outlet_anything(x->outlet_right, gensym("eval"), argc, argv);
        }

    } catch (const std::exception& e) {
        object_error((t_object*)x, "kernel: eval error: %s", e.what());
    }
}

void kernel_start(t_kernel* x)
{
    if (x->kernel && (*x->kernel)) {
        object_warn((t_object*)x, "kernel: already running");
        return;
    }

    if (!x->interpreter || !(*x->interpreter)) {
        object_error((t_object*)x, "kernel: interpreter not initialized");
        return;
    }

    try {
        // Note: Full kernel startup would require proper configuration
        // This is a placeholder for the actual kernel initialization
        object_post((t_object*)x, "kernel: start requested (full implementation pending)");

        if (x->outlet_right) {
            outlet_anything(x->outlet_right, gensym("started"), 0, NULL);
        }

    } catch (const std::exception& e) {
        object_error((t_object*)x, "kernel: start error: %s", e.what());
    }
}

void kernel_stop(t_kernel* x)
{
    if (x->kernel && (*x->kernel)) {
        try {
            (*x->kernel)->stop();

            object_post((t_object*)x, "kernel: stopped");

            if (x->outlet_right) {
                outlet_anything(x->outlet_right, gensym("stopped"), 0, NULL);
            }

        } catch (const std::exception& e) {
            object_error((t_object*)x, "kernel: stop error: %s", e.what());
        }
    } else {
        object_warn((t_object*)x, "kernel: not running");
    }
}

void kernel_info(t_kernel* x)
{
    if (!x->interpreter || !(*x->interpreter)) {
        object_error((t_object*)x, "kernel: interpreter not initialized");
        return;
    }

    try {
        nl::json info = (*x->interpreter)->kernel_info_request();

        object_post((t_object*)x, "=== Kernel Info ===");
        object_post((t_object*)x, "Implementation: %s",
                   info["implementation"].get<std::string>().c_str());
        object_post((t_object*)x, "Version: %s",
                   info["implementation_version"].get<std::string>().c_str());
        object_post((t_object*)x, "Language: %s",
                   info["language_info"]["name"].get<std::string>().c_str());

        if (x->outlet_right) {
            t_atom atoms[2];
            atom_setsym(&atoms[0], gensym("info"));
            atom_setsym(&atoms[1], gensym(info.dump().c_str()));
            outlet_anything(x->outlet_right, gensym("kernel"), 2, atoms);
        }

    } catch (const std::exception& e) {
        object_error((t_object*)x, "kernel: info error: %s", e.what());
    }
}
