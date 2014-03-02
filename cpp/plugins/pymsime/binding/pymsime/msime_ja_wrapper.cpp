
// default includes
#include <shiboken.h>
#include <pysidesignal.h>
#include <pysideproperty.h>
#include <pyside.h>
#include <destroylistener.h>
#include <typeresolver.h>
#include <typeinfo>
#include "pymsime_python.h"

#include "msime_ja_wrapper.h"

// Extra includes
#include <QList>
#include <QPair>
#include <msime_ja.h>



// Target ---------------------------------------------------------

extern "C" {
static int
Sbk_Msime_ja_Init(PyObject* self, PyObject* args, PyObject* kwds)
{
    SbkObject* sbkSelf = reinterpret_cast<SbkObject*>(self);
    if (Shiboken::Object::isUserType(self) && !Shiboken::ObjectType::canCallConstructor(self->ob_type, Shiboken::SbkType< ::Msime_ja >()))
        return -1;

    ::Msime_ja* cptr = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // Msime_ja()
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cptr = new ::Msime_ja();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred() || !Shiboken::Object::setCppPointer(sbkSelf, Shiboken::SbkType< ::Msime_ja >(), cptr)) {
        delete cptr;
        return -1;
    }
    Shiboken::Object::setValidCpp(sbkSelf, true);
    Shiboken::BindingManager::instance().registerWrapper(sbkSelf, cptr);


    return 1;
}

static PyObject* Sbk_Msime_jaFunc_isValid(PyObject* self)
{
    ::Msime_ja* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::Msime_ja*)Shiboken::Conversions::cppPointer(SbkpymsimeTypes[SBK_MSIME_JA_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // isValid()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::Msime_ja*>(cppSelf)->isValid();
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

static PyObject* Sbk_Msime_jaFunc_toFurigana(PyObject* self, PyObject* args, PyObject* kwds)
{
    ::Msime_ja* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::Msime_ja*)Shiboken::Conversions::cppPointer(SbkpymsimeTypes[SBK_MSIME_JA_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp[] = { 0, 0 };
    SBK_UNUSED(pythonToCpp)
    int numNamedArgs = (kwds ? PyDict_Size(kwds) : 0);
    int numArgs = PyTuple_GET_SIZE(args);
    PyObject* pyArgs[] = {0, 0};

    // invalid argument lengths
    if (numArgs + numNamedArgs > 2) {
        PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toFurigana(): too many arguments");
        return 0;
    } else if (numArgs < 1) {
        PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toFurigana(): not enough arguments");
        return 0;
    }

    if (!PyArg_ParseTuple(args, "|OO:toFurigana", &(pyArgs[0]), &(pyArgs[1])))
        return 0;


    // Overloaded function decisor
    // 0: toFurigana(QString,int)const
    if ((pythonToCpp[0] = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArgs[0])))) {
        if (numArgs == 1) {
            overloadId = 0; // toFurigana(QString,int)const
        } else if ((pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArgs[1])))) {
            overloadId = 0; // toFurigana(QString,int)const
        }
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_Msime_jaFunc_toFurigana_TypeError;

    // Call function/method
    {
        if (kwds) {
            PyObject* value = PyDict_GetItemString(kwds, "type");
            if (value && pyArgs[1]) {
                PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toFurigana(): got multiple values for keyword argument 'type'.");
                return 0;
            } else if (value) {
                pyArgs[1] = value;
                if (!(pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArgs[1]))))
                    goto Sbk_Msime_jaFunc_toFurigana_TypeError;
            }
        }
        ::QString cppArg0 = ::QString();
        pythonToCpp[0](pyArgs[0], &cppArg0);
        int cppArg1 = Msime_ja::AnyType;
        if (pythonToCpp[1]) pythonToCpp[1](pyArgs[1], &cppArg1);

        if (!PyErr_Occurred()) {
            // toFurigana(QString,int)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            QList<QPair<QString, QString > > cppResult = const_cast<const ::Msime_ja*>(cppSelf)->toFurigana(cppArg0, cppArg1);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkpymsimeTypeConverters[SBK_PYMSIME_QLIST_QPAIR_QSTRING_QSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_Msime_jaFunc_toFurigana_TypeError:
        const char* overloads[] = {"unicode, int = AnyType", 0};
        Shiboken::setErrorAboutWrongArguments(args, "pymsime.Msime_ja.toFurigana", overloads);
        return 0;
}

static PyObject* Sbk_Msime_jaFunc_toKanji(PyObject* self, PyObject* args, PyObject* kwds)
{
    ::Msime_ja* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::Msime_ja*)Shiboken::Conversions::cppPointer(SbkpymsimeTypes[SBK_MSIME_JA_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp[] = { 0, 0 };
    SBK_UNUSED(pythonToCpp)
    int numNamedArgs = (kwds ? PyDict_Size(kwds) : 0);
    int numArgs = PyTuple_GET_SIZE(args);
    PyObject* pyArgs[] = {0, 0};

    // invalid argument lengths
    if (numArgs + numNamedArgs > 2) {
        PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toKanji(): too many arguments");
        return 0;
    } else if (numArgs < 1) {
        PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toKanji(): not enough arguments");
        return 0;
    }

    if (!PyArg_ParseTuple(args, "|OO:toKanji", &(pyArgs[0]), &(pyArgs[1])))
        return 0;


    // Overloaded function decisor
    // 0: toKanji(QString,ulong)const
    if ((pythonToCpp[0] = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArgs[0])))) {
        if (numArgs == 1) {
            overloadId = 0; // toKanji(QString,ulong)const
        } else if ((pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<ulong>(), (pyArgs[1])))) {
            overloadId = 0; // toKanji(QString,ulong)const
        }
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_Msime_jaFunc_toKanji_TypeError;

    // Call function/method
    {
        if (kwds) {
            PyObject* value = PyDict_GetItemString(kwds, "flags");
            if (value && pyArgs[1]) {
                PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toKanji(): got multiple values for keyword argument 'flags'.");
                return 0;
            } else if (value) {
                pyArgs[1] = value;
                if (!(pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<ulong>(), (pyArgs[1]))))
                    goto Sbk_Msime_jaFunc_toKanji_TypeError;
            }
        }
        ::QString cppArg0 = ::QString();
        pythonToCpp[0](pyArgs[0], &cppArg0);
        ulong cppArg1 = 0;
        if (pythonToCpp[1]) pythonToCpp[1](pyArgs[1], &cppArg1);

        if (!PyErr_Occurred()) {
            // toKanji(QString,ulong)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            QString cppResult = const_cast<const ::Msime_ja*>(cppSelf)->toKanji(cppArg0, cppArg1);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_Msime_jaFunc_toKanji_TypeError:
        const char* overloads[] = {"unicode, unsigned long = 0", 0};
        Shiboken::setErrorAboutWrongArguments(args, "pymsime.Msime_ja.toKanji", overloads);
        return 0;
}

static PyObject* Sbk_Msime_jaFunc_toYomigana(PyObject* self, PyObject* args, PyObject* kwds)
{
    ::Msime_ja* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::Msime_ja*)Shiboken::Conversions::cppPointer(SbkpymsimeTypes[SBK_MSIME_JA_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp[] = { 0, 0 };
    SBK_UNUSED(pythonToCpp)
    int numNamedArgs = (kwds ? PyDict_Size(kwds) : 0);
    int numArgs = PyTuple_GET_SIZE(args);
    PyObject* pyArgs[] = {0, 0};

    // invalid argument lengths
    if (numArgs + numNamedArgs > 2) {
        PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toYomigana(): too many arguments");
        return 0;
    } else if (numArgs < 1) {
        PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toYomigana(): not enough arguments");
        return 0;
    }

    if (!PyArg_ParseTuple(args, "|OO:toYomigana", &(pyArgs[0]), &(pyArgs[1])))
        return 0;


    // Overloaded function decisor
    // 0: toYomigana(QString,int)const
    if ((pythonToCpp[0] = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArgs[0])))) {
        if (numArgs == 1) {
            overloadId = 0; // toYomigana(QString,int)const
        } else if ((pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArgs[1])))) {
            overloadId = 0; // toYomigana(QString,int)const
        }
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_Msime_jaFunc_toYomigana_TypeError;

    // Call function/method
    {
        if (kwds) {
            PyObject* value = PyDict_GetItemString(kwds, "type");
            if (value && pyArgs[1]) {
                PyErr_SetString(PyExc_TypeError, "pymsime.Msime_ja.toYomigana(): got multiple values for keyword argument 'type'.");
                return 0;
            } else if (value) {
                pyArgs[1] = value;
                if (!(pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArgs[1]))))
                    goto Sbk_Msime_jaFunc_toYomigana_TypeError;
            }
        }
        ::QString cppArg0 = ::QString();
        pythonToCpp[0](pyArgs[0], &cppArg0);
        int cppArg1 = Msime_ja::AnyType;
        if (pythonToCpp[1]) pythonToCpp[1](pyArgs[1], &cppArg1);

        if (!PyErr_Occurred()) {
            // toYomigana(QString,int)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            QString cppResult = const_cast<const ::Msime_ja*>(cppSelf)->toYomigana(cppArg0, cppArg1);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_Msime_jaFunc_toYomigana_TypeError:
        const char* overloads[] = {"unicode, int = AnyType", 0};
        Shiboken::setErrorAboutWrongArguments(args, "pymsime.Msime_ja.toYomigana", overloads);
        return 0;
}

static PyMethodDef Sbk_Msime_ja_methods[] = {
    {"isValid", (PyCFunction)Sbk_Msime_jaFunc_isValid, METH_NOARGS},
    {"toFurigana", (PyCFunction)Sbk_Msime_jaFunc_toFurigana, METH_VARARGS|METH_KEYWORDS},
    {"toKanji", (PyCFunction)Sbk_Msime_jaFunc_toKanji, METH_VARARGS|METH_KEYWORDS},
    {"toYomigana", (PyCFunction)Sbk_Msime_jaFunc_toYomigana, METH_VARARGS|METH_KEYWORDS},

    {0} // Sentinel
};

} // extern "C"

static int Sbk_Msime_ja_traverse(PyObject* self, visitproc visit, void* arg)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_traverse(self, visit, arg);
}
static int Sbk_Msime_ja_clear(PyObject* self)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_clear(self);
}
// Class Definition -----------------------------------------------
extern "C" {
static SbkObjectType Sbk_Msime_ja_Type = { { {
    PyVarObject_HEAD_INIT(&SbkObjectType_Type, 0)
    /*tp_name*/             "pymsime.Msime_ja",
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
    /*tp_traverse*/         Sbk_Msime_ja_traverse,
    /*tp_clear*/            Sbk_Msime_ja_clear,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          Sbk_Msime_ja_methods,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             reinterpret_cast<PyTypeObject*>(&SbkObject_Type),
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             Sbk_Msime_ja_Init,
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

// Python to C++ enum conversion.
static void Msime_ja_Type_PythonToCpp_Msime_ja_Type(PyObject* pyIn, void* cppOut) {
    *((::Msime_ja::Type*)cppOut) = (::Msime_ja::Type) Shiboken::Enum::getValue(pyIn);

}
static PythonToCppFunc is_Msime_ja_Type_PythonToCpp_Msime_ja_Type_Convertible(PyObject* pyIn) {
    if (PyObject_TypeCheck(pyIn, SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX]))
        return Msime_ja_Type_PythonToCpp_Msime_ja_Type;
    return 0;
}
static PyObject* Msime_ja_Type_CppToPython_Msime_ja_Type(const void* cppIn) {
    int castCppIn = *((::Msime_ja::Type*)cppIn);
    return Shiboken::Enum::newItem(SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX], castCppIn);

}

static void Msime_ja_Flag_PythonToCpp_Msime_ja_Flag(PyObject* pyIn, void* cppOut) {
    *((::Msime_ja::Flag*)cppOut) = (::Msime_ja::Flag) Shiboken::Enum::getValue(pyIn);

}
static PythonToCppFunc is_Msime_ja_Flag_PythonToCpp_Msime_ja_Flag_Convertible(PyObject* pyIn) {
    if (PyObject_TypeCheck(pyIn, SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX]))
        return Msime_ja_Flag_PythonToCpp_Msime_ja_Flag;
    return 0;
}
static PyObject* Msime_ja_Flag_CppToPython_Msime_ja_Flag(const void* cppIn) {
    int castCppIn = *((::Msime_ja::Flag*)cppIn);
    return Shiboken::Enum::newItem(SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX], castCppIn);

}

// Python to C++ pointer conversion - returns the C++ object of the Python wrapper (keeps object identity).
static void Msime_ja_PythonToCpp_Msime_ja_PTR(PyObject* pyIn, void* cppOut) {
    Shiboken::Conversions::pythonToCppPointer(&Sbk_Msime_ja_Type, pyIn, cppOut);
}
static PythonToCppFunc is_Msime_ja_PythonToCpp_Msime_ja_PTR_Convertible(PyObject* pyIn) {
    if (pyIn == Py_None)
        return Shiboken::Conversions::nonePythonToCppNullPtr;
    if (PyObject_TypeCheck(pyIn, (PyTypeObject*)&Sbk_Msime_ja_Type))
        return Msime_ja_PythonToCpp_Msime_ja_PTR;
    return 0;
}

// C++ to Python pointer conversion - tries to find the Python wrapper for the C++ object (keeps object identity).
static PyObject* Msime_ja_PTR_CppToPython_Msime_ja(const void* cppIn) {
    PyObject* pyOut = (PyObject*)Shiboken::BindingManager::instance().retrieveWrapper(cppIn);
    if (pyOut) {
        Py_INCREF(pyOut);
        return pyOut;
    }
    const char* typeName = typeid(*((::Msime_ja*)cppIn)).name();
    return Shiboken::Object::newObject(&Sbk_Msime_ja_Type, const_cast<void*>(cppIn), false, false, typeName);
}

void init_Msime_ja(PyObject* module)
{
    SbkpymsimeTypes[SBK_MSIME_JA_IDX] = reinterpret_cast<PyTypeObject*>(&Sbk_Msime_ja_Type);

    if (!Shiboken::ObjectType::introduceWrapperType(module, "Msime_ja", "Msime_ja*",
        &Sbk_Msime_ja_Type, &Shiboken::callCppDestructor< ::Msime_ja >)) {
        return;
    }

    // Register Converter
    SbkConverter* converter = Shiboken::Conversions::createConverter(&Sbk_Msime_ja_Type,
        Msime_ja_PythonToCpp_Msime_ja_PTR,
        is_Msime_ja_PythonToCpp_Msime_ja_PTR_Convertible,
        Msime_ja_PTR_CppToPython_Msime_ja);

    Shiboken::Conversions::registerConverterName(converter, "Msime_ja");
    Shiboken::Conversions::registerConverterName(converter, "Msime_ja*");
    Shiboken::Conversions::registerConverterName(converter, "Msime_ja&");
    Shiboken::Conversions::registerConverterName(converter, typeid(::Msime_ja).name());


    // Initialization of enums.

    // Initialization of enum 'Type'.
    SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX] = Shiboken::Enum::createScopedEnum(&Sbk_Msime_ja_Type,
        "Type",
        "pymsime.Msime_ja.Type",
        "Msime_ja::Type");
    if (!SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX])
        return ;

    if (!Shiboken::Enum::createScopedEnumItem(SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX],
        &Sbk_Msime_ja_Type, "AnyType", (long) Msime_ja::AnyType))
        return ;
    if (!Shiboken::Enum::createScopedEnumItem(SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX],
        &Sbk_Msime_ja_Type, "Hiragana", (long) Msime_ja::Hiragana))
        return ;
    if (!Shiboken::Enum::createScopedEnumItem(SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX],
        &Sbk_Msime_ja_Type, "Katagana", (long) Msime_ja::Katagana))
        return ;
    // Register converter for enum 'Msime_ja::Type'.
    {
        SbkConverter* converter = Shiboken::Conversions::createConverter(SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX],
            Msime_ja_Type_CppToPython_Msime_ja_Type);
        Shiboken::Conversions::addPythonToCppValueConversion(converter,
            Msime_ja_Type_PythonToCpp_Msime_ja_Type,
            is_Msime_ja_Type_PythonToCpp_Msime_ja_Type_Convertible);
        Shiboken::Enum::setTypeConverter(SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX], converter);
        Shiboken::Enum::setTypeConverter(SbkpymsimeTypes[SBK_MSIME_JA_TYPE_IDX], converter);
        Shiboken::Conversions::registerConverterName(converter, "Msime_ja::Type");
        Shiboken::Conversions::registerConverterName(converter, "Type");
    }
    // End of 'Type' enum.

    // Initialization of enum 'Flag'.
    SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX] = Shiboken::Enum::createScopedEnum(&Sbk_Msime_ja_Type,
        "Flag",
        "pymsime.Msime_ja.Flag",
        "Msime_ja::Flag");
    if (!SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX])
        return ;

    if (!Shiboken::Enum::createScopedEnumItem(SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX],
        &Sbk_Msime_ja_Type, "DefaultFlag", (long) Msime_ja::DefaultFlag))
        return ;
    if (!Shiboken::Enum::createScopedEnumItem(SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX],
        &Sbk_Msime_ja_Type, "Autocorrect", (long) Msime_ja::Autocorrect))
        return ;
    // Register converter for enum 'Msime_ja::Flag'.
    {
        SbkConverter* converter = Shiboken::Conversions::createConverter(SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX],
            Msime_ja_Flag_CppToPython_Msime_ja_Flag);
        Shiboken::Conversions::addPythonToCppValueConversion(converter,
            Msime_ja_Flag_PythonToCpp_Msime_ja_Flag,
            is_Msime_ja_Flag_PythonToCpp_Msime_ja_Flag_Convertible);
        Shiboken::Enum::setTypeConverter(SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX], converter);
        Shiboken::Enum::setTypeConverter(SbkpymsimeTypes[SBK_MSIME_JA_FLAG_IDX], converter);
        Shiboken::Conversions::registerConverterName(converter, "Msime_ja::Flag");
        Shiboken::Conversions::registerConverterName(converter, "Flag");
    }
    // End of 'Flag' enum.


    qRegisterMetaType< ::Msime_ja::Type >("Msime_ja::Type");
    qRegisterMetaType< ::Msime_ja::Flag >("Msime_ja::Flag");
}
