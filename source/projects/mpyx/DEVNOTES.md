# min-api notes


## Sequencing problem

In attributes is specified as follows, there are a few thing to note:

- `setter` and `getter` need to be qualified with `c74::min` because of `Python.h`

- setting a member in the member `py` instance causes a crash



```c++

attribute<symbol> instance_name { this, "name", "myname",
    description {
        "Name of python external instance."
        "This unique name will be used to id the instance and can be used to send messages to it."
    },
    c74::min::setter { [this](const atoms& args, const int inlet) -> atoms {
        symbol _name = args[0];
        this->py->set_name(_name); // <<-- causes CRASH
        return { _name };
    }},
    c74::min::getter { [this]() -> atoms {
        return { instance_name.get() };
    }}
};


attribute<symbol> pythonpath { this, "pythonpath", "",
    description {
        "Python pythonpath."
        "Add a custom path to add to the python interpreter's sys.path." 
    },
    c74::min::setter { [this](const atoms& args, const int inlet) -> atoms {
        symbol _pypath = args[0];
        this->py->set_pythonpath(_pypath); // <<-- causes CRASH
        return { _pypath } ;
    }},
    c74::min::getter { [this]() -> atoms {
        atoms result;
        result.push_back(this->py->get_pythonpath());
        return result;
    }}
};
```