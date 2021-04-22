# Notes on Implementing Thread Safety

## py-js very basic current approach

Very basic wrapping of python c-api code as follows:

```c
// func start
PyGILState_STATE gstate;
gstate = PyGILState_Ensure();
// ...
// python c-api code
// ...
PyGILState_Release(gstate);
// func end
```

When there is a blocking non-python code:

```c
Py_BEGIN_ALLOW_THREADS
//... Do some blocking I/O operation ...
Py_END_ALLOW_THREADS
```

Seems to work ok, not sure what else to do here.

Some relevant articles:

- [Python Docs -- Thread State and the Global Interpreter Lock](https://docs.python.org/3/c-api/init.html#thread-state-and-the-global-interpreter-lock)

- [Concurrency with embedded Python in a multi-threaded C++ application](https://www.codevate.com/blog/concurrency-with-embedded-python-in-a-multi-threaded-c-application)

- [Blocking vs Non-Blocking](https://www.modernescpp.com/index.php/blocking-and-non-blocking)

- [Python Threads do not run in C++ Application Embedded Interpreter](https://stackoverflow.com/questions/1775612/python-threads-do-not-run-in-c-application-embedded-interpreter)

## grrrr's py/pyext way

`py/pyext` has the most sophisticated threading implementation.

Threading is implemented in `pybase.c` and `pybase.h`.

## pybase.h threading part

```cpp
    // ----thread stuff ------------

    virtual void m_stop(int argc,const t_atom *argv);

    bool respond;
#ifdef FLEXT_THREADS
    int thrcount;
    bool shouldexit;
    int stoptick;
    Timer stoptmr;

    void tick(void *);
#endif

    int detach;
    bool pymsg;

    bool gencall(PyObject *fun,PyObject *args);

    bool docall(PyObject *fun,PyObject *args)
    {
        callpy(fun,args);
        if(PyErr_Occurred()) { 
            exchandle(); 
            return false; 
        }
        else 
            return true;
    }

    virtual void callpy(PyObject *fun,PyObject *args) = 0;

    void exchandle();

    static bool collect();

protected:

#ifdef FLEXT_THREADS
    static void thrworker(thr_params *data); 

    bool qucall(PyObject *fun,PyObject *args)
    {
        FifoEl *el = qufifo.New();
        el->Set(this,fun,args);
        qufifo.Put(el);
        qucond.Signal();
        return true;
    }

    static void quworker(thr_params *);
    static void pyworker(thr_params *);
    void erasethreads();

    static PyFifo qufifo;
    static ThrCond qucond;
    
#ifndef PY_USE_GIL
    static ThrState pythrsys;
#endif
#endif

    static const t_symbol *sym_fint; // float or int symbol, depending on native number message type
    static const t_symbol *sym_response;

    static const t_symbol *getone(t_atom &at,PyObject *arg);
    static const t_symbol *getlist(t_atom *lst,PyObject *seq,int cnt,int offs = 0);

public:

    static void AddToPath(const char *dir);

#ifdef FLEXT_THREADS
    // this is especially needed when one py/pyext object calls another one
    // we don't want the message to be queued, but otoh we have to avoid deadlock
    // (recursive calls can only happen in the system thread)
    static int lockcount;

#ifdef PY_USE_GIL
    static inline ThrState FindThreadState() { return ThrState(); }

    static inline ThrState PyLock(ThrState = ThrState()) { return PyGILState_Ensure(); }
    static inline ThrState PyLockSys() { return PyLock(); }
    static inline void PyUnlock(ThrState st) { PyGILState_Release(st); }
#else // PY_USE_GIL
    static ThrState FindThreadState();
    static void FreeThreadState();

    static ThrState PyLock(ThrState st = FindThreadState()) 
    { 
        if(st != pythrsys || !lockcount++) PyEval_AcquireLock();
        return PyThreadState_Swap(st);
    }

#if 1
    static inline ThrState PyLockSys() { return PyLock(); }
#else
    static ThrState PyLockSys() 
    { 
        if(!lockcount++) PyEval_AcquireLock();
        return PyThreadState_Swap(pythrsys);
    }
#endif

    static void PyUnlock(ThrState st) 
    {
        ThrState old = PyThreadState_Swap(st);
        if(old != pythrsys || !--lockcount) PyEval_ReleaseLock();
    }
#endif // PY_USE_GIL
    
#else // FLEXT_THREADS
    static inline ThrState PyLock(ThrState = NULL) { return NULL; }
    static inline ThrState PyLockSys() { return NULL; }
    static inline void PyUnlock(ThrState st) {}
#endif

    class ThrLock
    {
    public:
        ThrLock(): state(PyLock()) {}
        ThrLock(const ThrState &st): state(PyLock(st)) {}
        ThrLock(const ThrLock &t): state(PyLock(t.state)) {}
        ~ThrLock() { PyUnlock(state); }
        ThrState state;
    };

    class ThrLockSys
    {
    public:
        ThrLockSys(): state(PyLockSys()) {}
        ~ThrLockSys() { PyUnlock(state); }
        ThrState state;
    };
```
