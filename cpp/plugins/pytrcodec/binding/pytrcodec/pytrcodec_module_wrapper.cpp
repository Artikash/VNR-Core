
#include <sbkpython.h>
#include <shiboken.h>
#include <algorithm>
#include "pytrcodec_python.h"



// Extra includes

// Current module's type array.
PyTypeObject** SbkpytrcodecTypes;
// Current module's converter array.
SbkConverter** SbkpytrcodecTypeConverters;
// Global functions ------------------------------------------------------------

static PyMethodDef pytrcodec_methods[] = {
    {0} // Sentinel
};

// Classes initialization functions ------------------------------------------------------------
void init_TranslationCoder(PyObject* module);


// Module initialization ------------------------------------------------------------

// Primitive Type converters.

// C++ to Python conversion for type 'std::wstring'.
static PyObject* std_wstring_CppToPython_std_wstring(const void* cppIn) {
    ::std::wstring& cppInRef = *((::std::wstring*)cppIn);

          const int N = cppInRef.size();
          const wchar_t *str = cppInRef.c_str();
          PyObject *pyOut = PyUnicode_FromWideChar(str, N);
          return pyOut;

}
// Python to C++ conversions for type 'std::wstring'.
static void PyUnicode_PythonToCpp_std_wstring(PyObject* pyIn, void* cppOut) {

    Py_UNICODE *unicode = PyUnicode_AS_UNICODE(pyIn);
    *((::std::wstring*)cppOut) = unicode;

}
static PythonToCppFunc is_PyUnicode_PythonToCpp_std_wstring_Convertible(PyObject* pyIn) {
    if (PyUnicode_Check(pyIn))
        return PyUnicode_PythonToCpp_std_wstring;
    return 0;
}
static void PyString_PythonToCpp_std_wstring(PyObject* pyIn, void* cppOut) {

    const char *str = PyString_AS_STRING(pyIn);
    *((::std::wstring*)cppOut) = std::wstring(str, str + PyString_GET_SIZE(pyIn));

}
static PythonToCppFunc is_PyString_PythonToCpp_std_wstring_Convertible(PyObject* pyIn) {
    if (PyString_Check(pyIn))
        return PyString_PythonToCpp_std_wstring;
    return 0;
}
static void Py_None_PythonToCpp_std_wstring(PyObject* pyIn, void* cppOut) {

    *((::std::wstring*)cppOut) = std::wstring();

}
static PythonToCppFunc is_Py_None_PythonToCpp_std_wstring_Convertible(PyObject* pyIn) {
    if (pyIn == Py_None)
        return Py_None_PythonToCpp_std_wstring;
    return 0;
}


#if defined _WIN32 || defined __CYGWIN__
    #define SBK_EXPORT_MODULE __declspec(dllexport)
#elif __GNUC__ >= 4
    #define SBK_EXPORT_MODULE __attribute__ ((visibility("default")))
#else
    #define SBK_EXPORT_MODULE
#endif

#ifdef IS_PY3K
static struct PyModuleDef moduledef = {
    /* m_base     */ PyModuleDef_HEAD_INIT,
    /* m_name     */ "pytrcodec",
    /* m_doc      */ 0,
    /* m_size     */ -1,
    /* m_methods  */ pytrcodec_methods,
    /* m_reload   */ 0,
    /* m_traverse */ 0,
    /* m_clear    */ 0,
    /* m_free     */ 0
};

#endif
SBK_MODULE_INIT_FUNCTION_BEGIN(pytrcodec)
    // Create an array of wrapper types for the current module.
    static PyTypeObject* cppApi[SBK_pytrcodec_IDX_COUNT];
    SbkpytrcodecTypes = cppApi;

    // Create an array of primitive type converters for the current module.
    static SbkConverter* sbkConverters[SBK_pytrcodec_CONVERTERS_IDX_COUNT];
    SbkpytrcodecTypeConverters = sbkConverters;

#ifdef IS_PY3K
    PyObject* module = Shiboken::Module::create("pytrcodec", &moduledef);
#else
    PyObject* module = Shiboken::Module::create("pytrcodec", pytrcodec_methods);
#endif

    // Initialize classes in the type system
    init_TranslationCoder(module);

    // Register converter for type 'std::wstring'.
    SbkpytrcodecTypeConverters[SBK_STD_WSTRING_IDX] = Shiboken::Conversions::createConverter(&PyUnicode_Type, std_wstring_CppToPython_std_wstring);
    Shiboken::Conversions::registerConverterName(SbkpytrcodecTypeConverters[SBK_STD_WSTRING_IDX], "std::wstring");
    // Add user defined implicit conversions to type converter.
    Shiboken::Conversions::addPythonToCppValueConversion(SbkpytrcodecTypeConverters[SBK_STD_WSTRING_IDX],
        PyUnicode_PythonToCpp_std_wstring,
        is_PyUnicode_PythonToCpp_std_wstring_Convertible);
    Shiboken::Conversions::addPythonToCppValueConversion(SbkpytrcodecTypeConverters[SBK_STD_WSTRING_IDX],
        PyString_PythonToCpp_std_wstring,
        is_PyString_PythonToCpp_std_wstring_Convertible);
    Shiboken::Conversions::addPythonToCppValueConversion(SbkpytrcodecTypeConverters[SBK_STD_WSTRING_IDX],
        Py_None_PythonToCpp_std_wstring,
        is_Py_None_PythonToCpp_std_wstring_Convertible);

    // Register primitive types converters.

    Shiboken::Module::registerTypes(module, SbkpytrcodecTypes);
    Shiboken::Module::registerTypeConverters(module, SbkpytrcodecTypeConverters);

    if (PyErr_Occurred()) {
        PyErr_Print();
        Py_FatalError("can't initialize module pytrcodec");
    }
SBK_MODULE_INIT_FUNCTION_END
