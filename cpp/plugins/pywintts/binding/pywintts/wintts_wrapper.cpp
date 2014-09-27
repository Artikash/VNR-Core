
// default includes
#include <shiboken.h>
#include <typeresolver.h>
#include <typeinfo>
#include "pywintts_python.h"

#include "wintts_wrapper.h"

// Extra includes
#include <pywintts.h>



// Target ---------------------------------------------------------

extern "C" {
static int
Sbk_WinTts_Init(PyObject* self, PyObject* args, PyObject* kwds)
{
    SbkObject* sbkSelf = reinterpret_cast<SbkObject*>(self);
    if (Shiboken::Object::isUserType(self) && !Shiboken::ObjectType::canCallConstructor(self->ob_type, Shiboken::SbkType< ::WinTts >()))
        return -1;

    ::WinTts* cptr = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // WinTts()
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cptr = new ::WinTts();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred() || !Shiboken::Object::setCppPointer(sbkSelf, Shiboken::SbkType< ::WinTts >(), cptr)) {
        delete cptr;
        return -1;
    }
    Shiboken::Object::setValidCpp(sbkSelf, true);
    Shiboken::BindingManager::instance().registerWrapper(sbkSelf, cptr);


    return 1;
}

static PyObject* Sbk_WinTtsFunc_hasVoice(PyObject* self)
{
    ::WinTts* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::WinTts*)Shiboken::Conversions::cppPointer(SbkpywinttsTypes[SBK_WINTTS_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // hasVoice()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::WinTts*>(cppSelf)->hasVoice();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;
}

static PyObject* Sbk_WinTtsFunc_isValid(PyObject* self)
{
    ::WinTts* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::WinTts*)Shiboken::Conversions::cppPointer(SbkpywinttsTypes[SBK_WINTTS_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // isValid()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::WinTts*>(cppSelf)->isValid();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;
}

static PyObject* Sbk_WinTtsFunc_purge(PyObject* self, PyObject* args, PyObject* kwds)
{
    ::WinTts* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::WinTts*)Shiboken::Conversions::cppPointer(SbkpywinttsTypes[SBK_WINTTS_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp[] = { 0 };
    SBK_UNUSED(pythonToCpp)
    int numNamedArgs = (kwds ? PyDict_Size(kwds) : 0);
    int numArgs = PyTuple_GET_SIZE(args);
    PyObject* pyArgs[] = {0};

    // invalid argument lengths
    if (numArgs + numNamedArgs > 1) {
        PyErr_SetString(PyExc_TypeError, "pywintts.WinTts.purge(): too many arguments");
        return 0;
    }

    if (!PyArg_ParseTuple(args, "|O:purge", &(pyArgs[0])))
        return 0;


    // Overloaded function decisor
    // 0: purge(bool)const
    if (numArgs == 0) {
        overloadId = 0; // purge(bool)const
    } else if ((pythonToCpp[0] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), (pyArgs[0])))) {
        overloadId = 0; // purge(bool)const
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_WinTtsFunc_purge_TypeError;

    // Call function/method
    {
        if (kwds) {
            PyObject* value = PyDict_GetItemString(kwds, "async");
            if (value && pyArgs[0]) {
                PyErr_SetString(PyExc_TypeError, "pywintts.WinTts.purge(): got multiple values for keyword argument 'async'.");
                return 0;
            } else if (value) {
                pyArgs[0] = value;
                if (!(pythonToCpp[0] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), (pyArgs[0]))))
                    goto Sbk_WinTtsFunc_purge_TypeError;
            }
        }
        bool cppArg0 = false;
        if (pythonToCpp[0]) pythonToCpp[0](pyArgs[0], &cppArg0);

        if (!PyErr_Occurred()) {
            // purge(bool)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::WinTts*>(cppSelf)->purge(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_WinTtsFunc_purge_TypeError:
        const char* overloads[] = {"bool = false", 0};
        Shiboken::setErrorAboutWrongArguments(args, "pywintts.WinTts.purge", overloads);
        return 0;
}

static PyObject* Sbk_WinTtsFunc_setVoice(PyObject* self, PyObject* pyArg)
{
    ::WinTts* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::WinTts*)Shiboken::Conversions::cppPointer(SbkpywinttsTypes[SBK_WINTTS_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: setVoice(std::wstring)
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkpywinttsTypeConverters[SBK_STD_WSTRING_IDX], (pyArg)))) {
        overloadId = 0; // setVoice(std::wstring)
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_WinTtsFunc_setVoice_TypeError;

    // Call function/method
    {
        ::std::wstring cppArg0 = ::std::wstring();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // setVoice(std::wstring)
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = cppSelf->setVoice(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_WinTtsFunc_setVoice_TypeError:
        const char* overloads[] = {"std::wstring", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pywintts.WinTts.setVoice", overloads);
        return 0;
}

static PyObject* Sbk_WinTtsFunc_speak(PyObject* self, PyObject* args, PyObject* kwds)
{
    ::WinTts* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::WinTts*)Shiboken::Conversions::cppPointer(SbkpywinttsTypes[SBK_WINTTS_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp[] = { 0, 0 };
    SBK_UNUSED(pythonToCpp)
    int numNamedArgs = (kwds ? PyDict_Size(kwds) : 0);
    int numArgs = PyTuple_GET_SIZE(args);
    PyObject* pyArgs[] = {0, 0};

    // invalid argument lengths
    if (numArgs + numNamedArgs > 2) {
        PyErr_SetString(PyExc_TypeError, "pywintts.WinTts.speak(): too many arguments");
        return 0;
    } else if (numArgs < 1) {
        PyErr_SetString(PyExc_TypeError, "pywintts.WinTts.speak(): not enough arguments");
        return 0;
    }

    if (!PyArg_ParseTuple(args, "|OO:speak", &(pyArgs[0]), &(pyArgs[1])))
        return 0;


    // Overloaded function decisor
    // 0: speak(std::wstring,bool)const
    if ((pythonToCpp[0] = Shiboken::Conversions::isPythonToCppConvertible(SbkpywinttsTypeConverters[SBK_STD_WSTRING_IDX], (pyArgs[0])))) {
        if (numArgs == 1) {
            overloadId = 0; // speak(std::wstring,bool)const
        } else if ((pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), (pyArgs[1])))) {
            overloadId = 0; // speak(std::wstring,bool)const
        }
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_WinTtsFunc_speak_TypeError;

    // Call function/method
    {
        if (kwds) {
            PyObject* value = PyDict_GetItemString(kwds, "async");
            if (value && pyArgs[1]) {
                PyErr_SetString(PyExc_TypeError, "pywintts.WinTts.speak(): got multiple values for keyword argument 'async'.");
                return 0;
            } else if (value) {
                pyArgs[1] = value;
                if (!(pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), (pyArgs[1]))))
                    goto Sbk_WinTtsFunc_speak_TypeError;
            }
        }
        ::std::wstring cppArg0 = ::std::wstring();
        pythonToCpp[0](pyArgs[0], &cppArg0);
        bool cppArg1 = false;
        if (pythonToCpp[1]) pythonToCpp[1](pyArgs[1], &cppArg1);

        if (!PyErr_Occurred()) {
            // speak(std::wstring,bool)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::WinTts*>(cppSelf)->speak(cppArg0, cppArg1);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_WinTtsFunc_speak_TypeError:
        const char* overloads[] = {"std::wstring, bool = false", 0};
        Shiboken::setErrorAboutWrongArguments(args, "pywintts.WinTts.speak", overloads);
        return 0;
}

static PyObject* Sbk_WinTtsFunc_voice(PyObject* self)
{
    ::WinTts* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::WinTts*)Shiboken::Conversions::cppPointer(SbkpywinttsTypes[SBK_WINTTS_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // voice()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            std::wstring cppResult = const_cast<const ::WinTts*>(cppSelf)->voice();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkpywinttsTypeConverters[SBK_STD_WSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;
}

static PyMethodDef Sbk_WinTts_methods[] = {
    {"hasVoice", (PyCFunction)Sbk_WinTtsFunc_hasVoice, METH_NOARGS},
    {"isValid", (PyCFunction)Sbk_WinTtsFunc_isValid, METH_NOARGS},
    {"purge", (PyCFunction)Sbk_WinTtsFunc_purge, METH_VARARGS|METH_KEYWORDS},
    {"setVoice", (PyCFunction)Sbk_WinTtsFunc_setVoice, METH_O},
    {"speak", (PyCFunction)Sbk_WinTtsFunc_speak, METH_VARARGS|METH_KEYWORDS},
    {"voice", (PyCFunction)Sbk_WinTtsFunc_voice, METH_NOARGS},

    {0} // Sentinel
};

} // extern "C"

static int Sbk_WinTts_traverse(PyObject* self, visitproc visit, void* arg)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_traverse(self, visit, arg);
}
static int Sbk_WinTts_clear(PyObject* self)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_clear(self);
}
// Class Definition -----------------------------------------------
extern "C" {
static SbkObjectType Sbk_WinTts_Type = { { {
    PyVarObject_HEAD_INIT(&SbkObjectType_Type, 0)
    /*tp_name*/             "pywintts.WinTts",
    /*tp_basicsize*/        sizeof(SbkObject),
    /*tp_itemsize*/         0,
    /*tp_dealloc*/          &SbkDeallocWrapper,
    /*tp_print*/            0,
    /*tp_getattr*/          0,
    /*tp_setattr*/          0,
    /*tp_compare*/          0,
    /*tp_repr*/             0,
    /*tp_as_number*/        0,
    /*tp_as_sequence*/      0,
    /*tp_as_mapping*/       0,
    /*tp_hash*/             0,
    /*tp_call*/             0,
    /*tp_str*/              0,
    /*tp_getattro*/         0,
    /*tp_setattro*/         0,
    /*tp_as_buffer*/        0,
    /*tp_flags*/            Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE|Py_TPFLAGS_CHECKTYPES|Py_TPFLAGS_HAVE_GC,
    /*tp_doc*/              0,
    /*tp_traverse*/         Sbk_WinTts_traverse,
    /*tp_clear*/            Sbk_WinTts_clear,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          Sbk_WinTts_methods,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             reinterpret_cast<PyTypeObject*>(&SbkObject_Type),
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             Sbk_WinTts_Init,
    /*tp_alloc*/            0,
    /*tp_new*/              SbkObjectTpNew,
    /*tp_free*/             0,
    /*tp_is_gc*/            0,
    /*tp_bases*/            0,
    /*tp_mro*/              0,
    /*tp_cache*/            0,
    /*tp_subclasses*/       0,
    /*tp_weaklist*/         0
}, },
    /*priv_data*/           0
};
} //extern


// Type conversion functions.

// Python to C++ pointer conversion - returns the C++ object of the Python wrapper (keeps object identity).
static void WinTts_PythonToCpp_WinTts_PTR(PyObject* pyIn, void* cppOut) {
    Shiboken::Conversions::pythonToCppPointer(&Sbk_WinTts_Type, pyIn, cppOut);
}
static PythonToCppFunc is_WinTts_PythonToCpp_WinTts_PTR_Convertible(PyObject* pyIn) {
    if (pyIn == Py_None)
        return Shiboken::Conversions::nonePythonToCppNullPtr;
    if (PyObject_TypeCheck(pyIn, (PyTypeObject*)&Sbk_WinTts_Type))
        return WinTts_PythonToCpp_WinTts_PTR;
    return 0;
}

// C++ to Python pointer conversion - tries to find the Python wrapper for the C++ object (keeps object identity).
static PyObject* WinTts_PTR_CppToPython_WinTts(const void* cppIn) {
    PyObject* pyOut = (PyObject*)Shiboken::BindingManager::instance().retrieveWrapper(cppIn);
    if (pyOut) {
        Py_INCREF(pyOut);
        return pyOut;
    }
    const char* typeName = typeid(*((::WinTts*)cppIn)).name();
    return Shiboken::Object::newObject(&Sbk_WinTts_Type, const_cast<void*>(cppIn), false, false, typeName);
}

void init_WinTts(PyObject* module)
{
    SbkpywinttsTypes[SBK_WINTTS_IDX] = reinterpret_cast<PyTypeObject*>(&Sbk_WinTts_Type);

    if (!Shiboken::ObjectType::introduceWrapperType(module, "WinTts", "WinTts*",
        &Sbk_WinTts_Type, &Shiboken::callCppDestructor< ::WinTts >)) {
        return;
    }

    // Register Converter
    SbkConverter* converter = Shiboken::Conversions::createConverter(&Sbk_WinTts_Type,
        WinTts_PythonToCpp_WinTts_PTR,
        is_WinTts_PythonToCpp_WinTts_PTR_Convertible,
        WinTts_PTR_CppToPython_WinTts);

    Shiboken::Conversions::registerConverterName(converter, "WinTts");
    Shiboken::Conversions::registerConverterName(converter, "WinTts*");
    Shiboken::Conversions::registerConverterName(converter, "WinTts&");
    Shiboken::Conversions::registerConverterName(converter, typeid(::WinTts).name());



}
