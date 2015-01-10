
// default includes
#include <shiboken.h>
#include <typeresolver.h>
#include <typeinfo>
#include "pytrscript_python.h"

#include "translationscriptmanager_wrapper.h"

// Extra includes
#include <trscript.h>



// Target ---------------------------------------------------------

extern "C" {
static int
Sbk_TranslationScriptManager_Init(PyObject* self, PyObject* args, PyObject* kwds)
{
    SbkObject* sbkSelf = reinterpret_cast<SbkObject*>(self);
    if (Shiboken::Object::isUserType(self) && !Shiboken::ObjectType::canCallConstructor(self->ob_type, Shiboken::SbkType< ::TranslationScriptManager >()))
        return -1;

    ::TranslationScriptManager* cptr = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // TranslationScriptManager()
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cptr = new ::TranslationScriptManager();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred() || !Shiboken::Object::setCppPointer(sbkSelf, Shiboken::SbkType< ::TranslationScriptManager >(), cptr)) {
        delete cptr;
        return -1;
    }
    Shiboken::Object::setValidCpp(sbkSelf, true);
    Shiboken::BindingManager::instance().registerWrapper(sbkSelf, cptr);


    return 1;
}

static PyObject* Sbk_TranslationScriptManagerFunc_clear(PyObject* self)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));

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

static PyObject* Sbk_TranslationScriptManagerFunc_isEmpty(PyObject* self)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // isEmpty()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::TranslationScriptManager*>(cppSelf)->isEmpty();
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

static PyObject* Sbk_TranslationScriptManagerFunc_isLinkEnabled(PyObject* self)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // isLinkEnabled()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::TranslationScriptManager*>(cppSelf)->isLinkEnabled();
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

static PyObject* Sbk_TranslationScriptManagerFunc_linkStyle(PyObject* self)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // linkStyle()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            std::wstring cppResult = const_cast<const ::TranslationScriptManager*>(cppSelf)->linkStyle();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkpytrscriptTypeConverters[SBK_STD_WSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;
}

static PyObject* Sbk_TranslationScriptManagerFunc_loadFile(PyObject* self, PyObject* pyArg)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: loadFile(std::wstring)
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkpytrscriptTypeConverters[SBK_STD_WSTRING_IDX], (pyArg)))) {
        overloadId = 0; // loadFile(std::wstring)
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_TranslationScriptManagerFunc_loadFile_TypeError;

    // Call function/method
    {
        ::std::wstring cppArg0 = ::std::wstring();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // loadFile(std::wstring)
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

    Sbk_TranslationScriptManagerFunc_loadFile_TypeError:
        const char* overloads[] = {"std::wstring", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pytrscript.TranslationScriptManager.loadFile", overloads);
        return 0;
}

static PyObject* Sbk_TranslationScriptManagerFunc_setLinkEnabled(PyObject* self, PyObject* pyArg)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: setLinkEnabled(bool)
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), (pyArg)))) {
        overloadId = 0; // setLinkEnabled(bool)
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_TranslationScriptManagerFunc_setLinkEnabled_TypeError;

    // Call function/method
    {
        bool cppArg0;
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // setLinkEnabled(bool)
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cppSelf->setLinkEnabled(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred()) {
        return 0;
    }
    Py_RETURN_NONE;

    Sbk_TranslationScriptManagerFunc_setLinkEnabled_TypeError:
        const char* overloads[] = {"bool", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pytrscript.TranslationScriptManager.setLinkEnabled", overloads);
        return 0;
}

static PyObject* Sbk_TranslationScriptManagerFunc_setLinkStyle(PyObject* self, PyObject* pyArg)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: setLinkStyle(std::wstring)
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkpytrscriptTypeConverters[SBK_STD_WSTRING_IDX], (pyArg)))) {
        overloadId = 0; // setLinkStyle(std::wstring)
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_TranslationScriptManagerFunc_setLinkStyle_TypeError;

    // Call function/method
    {
        ::std::wstring cppArg0 = ::std::wstring();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // setLinkStyle(std::wstring)
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cppSelf->setLinkStyle(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred()) {
        return 0;
    }
    Py_RETURN_NONE;

    Sbk_TranslationScriptManagerFunc_setLinkStyle_TypeError:
        const char* overloads[] = {"std::wstring", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pytrscript.TranslationScriptManager.setLinkStyle", overloads);
        return 0;
}

static PyObject* Sbk_TranslationScriptManagerFunc_size(PyObject* self)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // size()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            int cppResult = const_cast<const ::TranslationScriptManager*>(cppSelf)->size();
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

static PyObject* Sbk_TranslationScriptManagerFunc_translate(PyObject* self, PyObject* pyArg)
{
    ::TranslationScriptManager* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::TranslationScriptManager*)Shiboken::Conversions::cppPointer(SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: translate(std::wstring)const
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkpytrscriptTypeConverters[SBK_STD_WSTRING_IDX], (pyArg)))) {
        overloadId = 0; // translate(std::wstring)const
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_TranslationScriptManagerFunc_translate_TypeError;

    // Call function/method
    {
        ::std::wstring cppArg0 = ::std::wstring();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // translate(std::wstring)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            std::wstring cppResult = const_cast<const ::TranslationScriptManager*>(cppSelf)->translate(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkpytrscriptTypeConverters[SBK_STD_WSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_TranslationScriptManagerFunc_translate_TypeError:
        const char* overloads[] = {"std::wstring", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pytrscript.TranslationScriptManager.translate", overloads);
        return 0;
}

static PyMethodDef Sbk_TranslationScriptManager_methods[] = {
    {"clear", (PyCFunction)Sbk_TranslationScriptManagerFunc_clear, METH_NOARGS},
    {"isEmpty", (PyCFunction)Sbk_TranslationScriptManagerFunc_isEmpty, METH_NOARGS},
    {"isLinkEnabled", (PyCFunction)Sbk_TranslationScriptManagerFunc_isLinkEnabled, METH_NOARGS},
    {"linkStyle", (PyCFunction)Sbk_TranslationScriptManagerFunc_linkStyle, METH_NOARGS},
    {"loadFile", (PyCFunction)Sbk_TranslationScriptManagerFunc_loadFile, METH_O},
    {"setLinkEnabled", (PyCFunction)Sbk_TranslationScriptManagerFunc_setLinkEnabled, METH_O},
    {"setLinkStyle", (PyCFunction)Sbk_TranslationScriptManagerFunc_setLinkStyle, METH_O},
    {"size", (PyCFunction)Sbk_TranslationScriptManagerFunc_size, METH_NOARGS},
    {"translate", (PyCFunction)Sbk_TranslationScriptManagerFunc_translate, METH_O},

    {0} // Sentinel
};

} // extern "C"

static int Sbk_TranslationScriptManager_traverse(PyObject* self, visitproc visit, void* arg)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_traverse(self, visit, arg);
}
static int Sbk_TranslationScriptManager_clear(PyObject* self)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_clear(self);
}
// Class Definition -----------------------------------------------
extern "C" {
static SbkObjectType Sbk_TranslationScriptManager_Type = { { {
    PyVarObject_HEAD_INIT(&SbkObjectType_Type, 0)
    /*tp_name*/             "pytrscript.TranslationScriptManager",
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
    /*tp_traverse*/         Sbk_TranslationScriptManager_traverse,
    /*tp_clear*/            Sbk_TranslationScriptManager_clear,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          Sbk_TranslationScriptManager_methods,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             reinterpret_cast<PyTypeObject*>(&SbkObject_Type),
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             Sbk_TranslationScriptManager_Init,
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
static void TranslationScriptManager_PythonToCpp_TranslationScriptManager_PTR(PyObject* pyIn, void* cppOut) {
    Shiboken::Conversions::pythonToCppPointer(&Sbk_TranslationScriptManager_Type, pyIn, cppOut);
}
static PythonToCppFunc is_TranslationScriptManager_PythonToCpp_TranslationScriptManager_PTR_Convertible(PyObject* pyIn) {
    if (pyIn == Py_None)
        return Shiboken::Conversions::nonePythonToCppNullPtr;
    if (PyObject_TypeCheck(pyIn, (PyTypeObject*)&Sbk_TranslationScriptManager_Type))
        return TranslationScriptManager_PythonToCpp_TranslationScriptManager_PTR;
    return 0;
}

// C++ to Python pointer conversion - tries to find the Python wrapper for the C++ object (keeps object identity).
static PyObject* TranslationScriptManager_PTR_CppToPython_TranslationScriptManager(const void* cppIn) {
    PyObject* pyOut = (PyObject*)Shiboken::BindingManager::instance().retrieveWrapper(cppIn);
    if (pyOut) {
        Py_INCREF(pyOut);
        return pyOut;
    }
    const char* typeName = typeid(*((::TranslationScriptManager*)cppIn)).name();
    return Shiboken::Object::newObject(&Sbk_TranslationScriptManager_Type, const_cast<void*>(cppIn), false, false, typeName);
}

void init_TranslationScriptManager(PyObject* module)
{
    SbkpytrscriptTypes[SBK_TRANSLATIONSCRIPTMANAGER_IDX] = reinterpret_cast<PyTypeObject*>(&Sbk_TranslationScriptManager_Type);

    if (!Shiboken::ObjectType::introduceWrapperType(module, "TranslationScriptManager", "TranslationScriptManager*",
        &Sbk_TranslationScriptManager_Type, &Shiboken::callCppDestructor< ::TranslationScriptManager >)) {
        return;
    }

    // Register Converter
    SbkConverter* converter = Shiboken::Conversions::createConverter(&Sbk_TranslationScriptManager_Type,
        TranslationScriptManager_PythonToCpp_TranslationScriptManager_PTR,
        is_TranslationScriptManager_PythonToCpp_TranslationScriptManager_PTR_Convertible,
        TranslationScriptManager_PTR_CppToPython_TranslationScriptManager);

    Shiboken::Conversions::registerConverterName(converter, "TranslationScriptManager");
    Shiboken::Conversions::registerConverterName(converter, "TranslationScriptManager*");
    Shiboken::Conversions::registerConverterName(converter, "TranslationScriptManager&");
    Shiboken::Conversions::registerConverterName(converter, typeid(::TranslationScriptManager).name());



}
