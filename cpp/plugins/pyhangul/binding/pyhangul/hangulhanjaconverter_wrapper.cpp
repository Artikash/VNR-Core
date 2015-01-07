
// default includes
#include <shiboken.h>
#include <pysidesignal.h>
#include <pysideproperty.h>
#include <pyside.h>
#include <destroylistener.h>
#include <typeresolver.h>
#include <typeinfo>
#include "pyhangul_python.h"

#include "hangulhanjaconverter_wrapper.h"

// Extra includes
#include <QList>
#include <QPair>
#include <hangulconv.h>



// Target ---------------------------------------------------------

extern "C" {
static int
Sbk_HangulHanjaConverter_Init(PyObject* self, PyObject* args, PyObject* kwds)
{
    SbkObject* sbkSelf = reinterpret_cast<SbkObject*>(self);
    if (Shiboken::Object::isUserType(self) && !Shiboken::ObjectType::canCallConstructor(self->ob_type, Shiboken::SbkType< ::HangulHanjaConverter >()))
        return -1;

    ::HangulHanjaConverter* cptr = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // HangulHanjaConverter()
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cptr = new ::HangulHanjaConverter();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred() || !Shiboken::Object::setCppPointer(sbkSelf, Shiboken::SbkType< ::HangulHanjaConverter >(), cptr)) {
        delete cptr;
        return -1;
    }
    Shiboken::Object::setValidCpp(sbkSelf, true);
    Shiboken::BindingManager::instance().registerWrapper(sbkSelf, cptr);


    return 1;
}

static PyObject* Sbk_HangulHanjaConverterFunc_clear(PyObject* self)
{
    ::HangulHanjaConverter* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::HangulHanjaConverter*)Shiboken::Conversions::cppPointer(SbkpyhangulTypes[SBK_HANGULHANJACONVERTER_IDX], (SbkObject*)self));

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // clear()
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cppSelf->clear();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred()) {
        return 0;
    }
    Py_RETURN_NONE;
}

static PyObject* Sbk_HangulHanjaConverterFunc_convert(PyObject* self, PyObject* pyArg)
{
    ::HangulHanjaConverter* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::HangulHanjaConverter*)Shiboken::Conversions::cppPointer(SbkpyhangulTypes[SBK_HANGULHANJACONVERTER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: convert(QString)const
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArg)))) {
        overloadId = 0; // convert(QString)const
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_HangulHanjaConverterFunc_convert_TypeError;

    // Call function/method
    {
        ::QString cppArg0 = ::QString();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // convert(QString)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            QString cppResult = const_cast<const ::HangulHanjaConverter*>(cppSelf)->convert(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_HangulHanjaConverterFunc_convert_TypeError:
        const char* overloads[] = {"unicode", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pyhangul.HangulHanjaConverter.convert", overloads);
        return 0;
}

static PyObject* Sbk_HangulHanjaConverterFunc_isEmpty(PyObject* self)
{
    ::HangulHanjaConverter* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::HangulHanjaConverter*)Shiboken::Conversions::cppPointer(SbkpyhangulTypes[SBK_HANGULHANJACONVERTER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // isEmpty()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::HangulHanjaConverter*>(cppSelf)->isEmpty();
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

static PyObject* Sbk_HangulHanjaConverterFunc_loadFile(PyObject* self, PyObject* pyArg)
{
    ::HangulHanjaConverter* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::HangulHanjaConverter*)Shiboken::Conversions::cppPointer(SbkpyhangulTypes[SBK_HANGULHANJACONVERTER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: loadFile(QString)
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArg)))) {
        overloadId = 0; // loadFile(QString)
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_HangulHanjaConverterFunc_loadFile_TypeError;

    // Call function/method
    {
        ::QString cppArg0 = ::QString();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // loadFile(QString)
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = cppSelf->loadFile(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_HangulHanjaConverterFunc_loadFile_TypeError:
        const char* overloads[] = {"unicode", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pyhangul.HangulHanjaConverter.loadFile", overloads);
        return 0;
}

static PyObject* Sbk_HangulHanjaConverterFunc_parse(PyObject* self, PyObject* pyArg)
{
    ::HangulHanjaConverter* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::HangulHanjaConverter*)Shiboken::Conversions::cppPointer(SbkpyhangulTypes[SBK_HANGULHANJACONVERTER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: parse(QString)const
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArg)))) {
        overloadId = 0; // parse(QString)const
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_HangulHanjaConverterFunc_parse_TypeError;

    // Call function/method
    {
        ::QString cppArg0 = ::QString();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // parse(QString)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            QList<QList<QPair<QString, QString > > > cppResult = const_cast<const ::HangulHanjaConverter*>(cppSelf)->parse(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkpyhangulTypeConverters[SBK_PYHANGUL_QLIST_QLIST_QPAIR_QSTRING_QSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_HangulHanjaConverterFunc_parse_TypeError:
        const char* overloads[] = {"unicode", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pyhangul.HangulHanjaConverter.parse", overloads);
        return 0;
}

static PyObject* Sbk_HangulHanjaConverterFunc_size(PyObject* self)
{
    ::HangulHanjaConverter* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::HangulHanjaConverter*)Shiboken::Conversions::cppPointer(SbkpyhangulTypes[SBK_HANGULHANJACONVERTER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // size()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            int cppResult = const_cast<const ::HangulHanjaConverter*>(cppSelf)->size();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<int>(), &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;
}

static PyMethodDef Sbk_HangulHanjaConverter_methods[] = {
    {"clear", (PyCFunction)Sbk_HangulHanjaConverterFunc_clear, METH_NOARGS},
    {"convert", (PyCFunction)Sbk_HangulHanjaConverterFunc_convert, METH_O},
    {"isEmpty", (PyCFunction)Sbk_HangulHanjaConverterFunc_isEmpty, METH_NOARGS},
    {"loadFile", (PyCFunction)Sbk_HangulHanjaConverterFunc_loadFile, METH_O},
    {"parse", (PyCFunction)Sbk_HangulHanjaConverterFunc_parse, METH_O},
    {"size", (PyCFunction)Sbk_HangulHanjaConverterFunc_size, METH_NOARGS},

    {0} // Sentinel
};

} // extern "C"

static int Sbk_HangulHanjaConverter_traverse(PyObject* self, visitproc visit, void* arg)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_traverse(self, visit, arg);
}
static int Sbk_HangulHanjaConverter_clear(PyObject* self)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_clear(self);
}
// Class Definition -----------------------------------------------
extern "C" {
static SbkObjectType Sbk_HangulHanjaConverter_Type = { { {
    PyVarObject_HEAD_INIT(&SbkObjectType_Type, 0)
    /*tp_name*/             "pyhangul.HangulHanjaConverter",
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
    /*tp_traverse*/         Sbk_HangulHanjaConverter_traverse,
    /*tp_clear*/            Sbk_HangulHanjaConverter_clear,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          Sbk_HangulHanjaConverter_methods,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             reinterpret_cast<PyTypeObject*>(&SbkObject_Type),
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             Sbk_HangulHanjaConverter_Init,
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
static void HangulHanjaConverter_PythonToCpp_HangulHanjaConverter_PTR(PyObject* pyIn, void* cppOut) {
    Shiboken::Conversions::pythonToCppPointer(&Sbk_HangulHanjaConverter_Type, pyIn, cppOut);
}
static PythonToCppFunc is_HangulHanjaConverter_PythonToCpp_HangulHanjaConverter_PTR_Convertible(PyObject* pyIn) {
    if (pyIn == Py_None)
        return Shiboken::Conversions::nonePythonToCppNullPtr;
    if (PyObject_TypeCheck(pyIn, (PyTypeObject*)&Sbk_HangulHanjaConverter_Type))
        return HangulHanjaConverter_PythonToCpp_HangulHanjaConverter_PTR;
    return 0;
}

// C++ to Python pointer conversion - tries to find the Python wrapper for the C++ object (keeps object identity).
static PyObject* HangulHanjaConverter_PTR_CppToPython_HangulHanjaConverter(const void* cppIn) {
    PyObject* pyOut = (PyObject*)Shiboken::BindingManager::instance().retrieveWrapper(cppIn);
    if (pyOut) {
        Py_INCREF(pyOut);
        return pyOut;
    }
    const char* typeName = typeid(*((::HangulHanjaConverter*)cppIn)).name();
    return Shiboken::Object::newObject(&Sbk_HangulHanjaConverter_Type, const_cast<void*>(cppIn), false, false, typeName);
}

void init_HangulHanjaConverter(PyObject* module)
{
    SbkpyhangulTypes[SBK_HANGULHANJACONVERTER_IDX] = reinterpret_cast<PyTypeObject*>(&Sbk_HangulHanjaConverter_Type);

    if (!Shiboken::ObjectType::introduceWrapperType(module, "HangulHanjaConverter", "HangulHanjaConverter*",
        &Sbk_HangulHanjaConverter_Type, &Shiboken::callCppDestructor< ::HangulHanjaConverter >)) {
        return;
    }

    // Register Converter
    SbkConverter* converter = Shiboken::Conversions::createConverter(&Sbk_HangulHanjaConverter_Type,
        HangulHanjaConverter_PythonToCpp_HangulHanjaConverter_PTR,
        is_HangulHanjaConverter_PythonToCpp_HangulHanjaConverter_PTR_Convertible,
        HangulHanjaConverter_PTR_CppToPython_HangulHanjaConverter);

    Shiboken::Conversions::registerConverterName(converter, "HangulHanjaConverter");
    Shiboken::Conversions::registerConverterName(converter, "HangulHanjaConverter*");
    Shiboken::Conversions::registerConverterName(converter, "HangulHanjaConverter&");
    Shiboken::Conversions::registerConverterName(converter, typeid(::HangulHanjaConverter).name());



}
