
// default includes
#include <shiboken.h>
#include <pysidesignal.h>
#include <pysideproperty.h>
#include <pyside.h>
#include <destroylistener.h>
#include <typeresolver.h>
#include <typeinfo>
#include "pyrich_python.h"

#include "richrubyparser_wrapper.h"

// Extra includes
#include <qfontmetrics.h>
#include <richrubyparser.h>



// Target ---------------------------------------------------------

extern "C" {
static int
Sbk_RichRubyParser_Init(PyObject* self, PyObject* args, PyObject* kwds)
{
    SbkObject* sbkSelf = reinterpret_cast<SbkObject*>(self);
    if (Shiboken::Object::isUserType(self) && !Shiboken::ObjectType::canCallConstructor(self->ob_type, Shiboken::SbkType< ::RichRubyParser >()))
        return -1;

    ::RichRubyParser* cptr = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // RichRubyParser()
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cptr = new ::RichRubyParser();
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred() || !Shiboken::Object::setCppPointer(sbkSelf, Shiboken::SbkType< ::RichRubyParser >(), cptr)) {
        delete cptr;
        return -1;
    }
    Shiboken::Object::setValidCpp(sbkSelf, true);
    Shiboken::BindingManager::instance().registerWrapper(sbkSelf, cptr);


    return 1;
}

static PyObject* Sbk_RichRubyParserFunc_closeChar(PyObject* self)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // closeChar()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            int cppResult = const_cast<const ::RichRubyParser*>(cppSelf)->closeChar();
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

static PyObject* Sbk_RichRubyParserFunc_containsRuby(PyObject* self, PyObject* pyArg)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: containsRuby(QString)const
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArg)))) {
        overloadId = 0; // containsRuby(QString)const
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_RichRubyParserFunc_containsRuby_TypeError;

    // Call function/method
    {
        ::QString cppArg0 = ::QString();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // containsRuby(QString)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            bool cppResult = const_cast<const ::RichRubyParser*>(cppSelf)->containsRuby(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_RichRubyParserFunc_containsRuby_TypeError:
        const char* overloads[] = {"unicode", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pyrich.RichRubyParser.containsRuby", overloads);
        return 0;
}

static PyObject* Sbk_RichRubyParserFunc_createRuby(PyObject* self, PyObject* args)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp[] = { 0, 0 };
    SBK_UNUSED(pythonToCpp)
    int numArgs = PyTuple_GET_SIZE(args);
    PyObject* pyArgs[] = {0, 0};

    // invalid argument lengths


    if (!PyArg_UnpackTuple(args, "createRuby", 2, 2, &(pyArgs[0]), &(pyArgs[1])))
        return 0;


    // Overloaded function decisor
    // 0: createRuby(QString,QString)const
    if (numArgs == 2
        && (pythonToCpp[0] = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArgs[0])))
        && (pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArgs[1])))) {
        overloadId = 0; // createRuby(QString,QString)const
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_RichRubyParserFunc_createRuby_TypeError;

    // Call function/method
    {
        ::QString cppArg0 = ::QString();
        pythonToCpp[0](pyArgs[0], &cppArg0);
        ::QString cppArg1 = ::QString();
        pythonToCpp[1](pyArgs[1], &cppArg1);

        if (!PyErr_Occurred()) {
            // createRuby(QString,QString)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            QString cppResult = const_cast<const ::RichRubyParser*>(cppSelf)->createRuby(cppArg0, cppArg1);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_RichRubyParserFunc_createRuby_TypeError:
        const char* overloads[] = {"unicode, unicode", 0};
        Shiboken::setErrorAboutWrongArguments(args, "pyrich.RichRubyParser.createRuby", overloads);
        return 0;
}

static PyObject* Sbk_RichRubyParserFunc_openChar(PyObject* self)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // openChar()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            int cppResult = const_cast<const ::RichRubyParser*>(cppSelf)->openChar();
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

static PyObject* Sbk_RichRubyParserFunc_removeRuby(PyObject* self, PyObject* pyArg)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: removeRuby(QString)const
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArg)))) {
        overloadId = 0; // removeRuby(QString)const
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_RichRubyParserFunc_removeRuby_TypeError;

    // Call function/method
    {
        ::QString cppArg0 = ::QString();
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // removeRuby(QString)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            QString cppResult = const_cast<const ::RichRubyParser*>(cppSelf)->removeRuby(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_RichRubyParserFunc_removeRuby_TypeError:
        const char* overloads[] = {"unicode", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pyrich.RichRubyParser.removeRuby", overloads);
        return 0;
}

static PyObject* Sbk_RichRubyParserFunc_renderTable(PyObject* self, PyObject* args, PyObject* kwds)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;
    int overloadId = -1;
    PythonToCppFunc pythonToCpp[] = { 0, 0, 0, 0, 0, 0 };
    SBK_UNUSED(pythonToCpp)
    int numNamedArgs = (kwds ? PyDict_Size(kwds) : 0);
    int numArgs = PyTuple_GET_SIZE(args);
    PyObject* pyArgs[] = {0, 0, 0, 0, 0, 0};

    // invalid argument lengths
    if (numArgs + numNamedArgs > 6) {
        PyErr_SetString(PyExc_TypeError, "pyrich.RichRubyParser.renderTable(): too many arguments");
        return 0;
    } else if (numArgs < 4) {
        PyErr_SetString(PyExc_TypeError, "pyrich.RichRubyParser.renderTable(): not enough arguments");
        return 0;
    }

    if (!PyArg_ParseTuple(args, "|OOOOOO:renderTable", &(pyArgs[0]), &(pyArgs[1]), &(pyArgs[2]), &(pyArgs[3]), &(pyArgs[4]), &(pyArgs[5])))
        return 0;


    // Overloaded function decisor
    // 0: renderTable(QString,int,QFontMetrics,QFontMetrics,int,bool)const
    if (numArgs >= 4
        && (pythonToCpp[0] = Shiboken::Conversions::isPythonToCppConvertible(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], (pyArgs[0])))
        && (pythonToCpp[1] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArgs[1])))
        && (pythonToCpp[2] = Shiboken::Conversions::isPythonToCppReferenceConvertible((SbkObjectType*)SbkPySide_QtGuiTypes[SBK_QFONTMETRICS_IDX], (pyArgs[2])))
        && (pythonToCpp[3] = Shiboken::Conversions::isPythonToCppReferenceConvertible((SbkObjectType*)SbkPySide_QtGuiTypes[SBK_QFONTMETRICS_IDX], (pyArgs[3])))) {
        if (numArgs == 4) {
            overloadId = 0; // renderTable(QString,int,QFontMetrics,QFontMetrics,int,bool)const
        } else if ((pythonToCpp[4] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArgs[4])))) {
            if (numArgs == 5) {
                overloadId = 0; // renderTable(QString,int,QFontMetrics,QFontMetrics,int,bool)const
            } else if ((pythonToCpp[5] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), (pyArgs[5])))) {
                overloadId = 0; // renderTable(QString,int,QFontMetrics,QFontMetrics,int,bool)const
            }
        }
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_RichRubyParserFunc_renderTable_TypeError;

    // Call function/method
    {
        if (kwds) {
            PyObject* value = PyDict_GetItemString(kwds, "cellSpace");
            if (value && pyArgs[4]) {
                PyErr_SetString(PyExc_TypeError, "pyrich.RichRubyParser.renderTable(): got multiple values for keyword argument 'cellSpace'.");
                return 0;
            } else if (value) {
                pyArgs[4] = value;
                if (!(pythonToCpp[4] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArgs[4]))))
                    goto Sbk_RichRubyParserFunc_renderTable_TypeError;
            }
            value = PyDict_GetItemString(kwds, "wordWrap");
            if (value && pyArgs[5]) {
                PyErr_SetString(PyExc_TypeError, "pyrich.RichRubyParser.renderTable(): got multiple values for keyword argument 'wordWrap'.");
                return 0;
            } else if (value) {
                pyArgs[5] = value;
                if (!(pythonToCpp[5] = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<bool>(), (pyArgs[5]))))
                    goto Sbk_RichRubyParserFunc_renderTable_TypeError;
            }
        }
        ::QString cppArg0 = ::QString();
        pythonToCpp[0](pyArgs[0], &cppArg0);
        int cppArg1;
        pythonToCpp[1](pyArgs[1], &cppArg1);
        if (!Shiboken::Object::isValid(pyArgs[2]))
            return 0;
        ::QFontMetrics cppArg2_local = ::QFontMetrics(::QFont());
        ::QFontMetrics* cppArg2 = &cppArg2_local;
        if (Shiboken::Conversions::isImplicitConversion((SbkObjectType*)SbkPySide_QtGuiTypes[SBK_QFONTMETRICS_IDX], pythonToCpp[2]))
            pythonToCpp[2](pyArgs[2], &cppArg2_local);
        else
            pythonToCpp[2](pyArgs[2], &cppArg2);

        if (!Shiboken::Object::isValid(pyArgs[3]))
            return 0;
        ::QFontMetrics cppArg3_local = ::QFontMetrics(::QFont());
        ::QFontMetrics* cppArg3 = &cppArg3_local;
        if (Shiboken::Conversions::isImplicitConversion((SbkObjectType*)SbkPySide_QtGuiTypes[SBK_QFONTMETRICS_IDX], pythonToCpp[3]))
            pythonToCpp[3](pyArgs[3], &cppArg3_local);
        else
            pythonToCpp[3](pyArgs[3], &cppArg3);

        int cppArg4 = 1;
        if (pythonToCpp[4]) pythonToCpp[4](pyArgs[4], &cppArg4);
        bool cppArg5 = true;
        if (pythonToCpp[5]) pythonToCpp[5](pyArgs[5], &cppArg5);

        if (!PyErr_Occurred()) {
            // renderTable(QString,int,QFontMetrics,QFontMetrics,int,bool)const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            QString cppResult = const_cast<const ::RichRubyParser*>(cppSelf)->renderTable(cppArg0, cppArg1, *cppArg2, *cppArg3, cppArg4, cppArg5);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
            pyResult = Shiboken::Conversions::copyToPython(SbkPySide_QtCoreTypeConverters[SBK_QSTRING_IDX], &cppResult);
        }
    }

    if (PyErr_Occurred() || !pyResult) {
        Py_XDECREF(pyResult);
        return 0;
    }
    return pyResult;

    Sbk_RichRubyParserFunc_renderTable_TypeError:
        const char* overloads[] = {"unicode, int, PySide.QtGui.QFontMetrics, PySide.QtGui.QFontMetrics, int = 1, bool = true", 0};
        Shiboken::setErrorAboutWrongArguments(args, "pyrich.RichRubyParser.renderTable", overloads);
        return 0;
}

static PyObject* Sbk_RichRubyParserFunc_setCloseChar(PyObject* self, PyObject* pyArg)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: setCloseChar(int)
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArg)))) {
        overloadId = 0; // setCloseChar(int)
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_RichRubyParserFunc_setCloseChar_TypeError;

    // Call function/method
    {
        int cppArg0;
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // setCloseChar(int)
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cppSelf->setCloseChar(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred()) {
        return 0;
    }
    Py_RETURN_NONE;

    Sbk_RichRubyParserFunc_setCloseChar_TypeError:
        const char* overloads[] = {"int", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pyrich.RichRubyParser.setCloseChar", overloads);
        return 0;
}

static PyObject* Sbk_RichRubyParserFunc_setOpenChar(PyObject* self, PyObject* pyArg)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: setOpenChar(int)
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArg)))) {
        overloadId = 0; // setOpenChar(int)
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_RichRubyParserFunc_setOpenChar_TypeError;

    // Call function/method
    {
        int cppArg0;
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // setOpenChar(int)
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cppSelf->setOpenChar(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred()) {
        return 0;
    }
    Py_RETURN_NONE;

    Sbk_RichRubyParserFunc_setOpenChar_TypeError:
        const char* overloads[] = {"int", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pyrich.RichRubyParser.setOpenChar", overloads);
        return 0;
}

static PyObject* Sbk_RichRubyParserFunc_setSplitChar(PyObject* self, PyObject* pyArg)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    int overloadId = -1;
    PythonToCppFunc pythonToCpp;
    SBK_UNUSED(pythonToCpp)

    // Overloaded function decisor
    // 0: setSplitChar(int)
    if ((pythonToCpp = Shiboken::Conversions::isPythonToCppConvertible(Shiboken::Conversions::PrimitiveTypeConverter<int>(), (pyArg)))) {
        overloadId = 0; // setSplitChar(int)
    }

    // Function signature not found.
    if (overloadId == -1) goto Sbk_RichRubyParserFunc_setSplitChar_TypeError;

    // Call function/method
    {
        int cppArg0;
        pythonToCpp(pyArg, &cppArg0);

        if (!PyErr_Occurred()) {
            // setSplitChar(int)
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            cppSelf->setSplitChar(cppArg0);
            PyEval_RestoreThread(_save); // Py_END_ALLOW_THREADS
        }
    }

    if (PyErr_Occurred()) {
        return 0;
    }
    Py_RETURN_NONE;

    Sbk_RichRubyParserFunc_setSplitChar_TypeError:
        const char* overloads[] = {"int", 0};
        Shiboken::setErrorAboutWrongArguments(pyArg, "pyrich.RichRubyParser.setSplitChar", overloads);
        return 0;
}

static PyObject* Sbk_RichRubyParserFunc_splitChar(PyObject* self)
{
    ::RichRubyParser* cppSelf = 0;
    SBK_UNUSED(cppSelf)
    if (!Shiboken::Object::isValid(self))
        return 0;
    cppSelf = ((::RichRubyParser*)Shiboken::Conversions::cppPointer(SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX], (SbkObject*)self));
    PyObject* pyResult = 0;

    // Call function/method
    {

        if (!PyErr_Occurred()) {
            // splitChar()const
            PyThreadState* _save = PyEval_SaveThread(); // Py_BEGIN_ALLOW_THREADS
            int cppResult = const_cast<const ::RichRubyParser*>(cppSelf)->splitChar();
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

static PyMethodDef Sbk_RichRubyParser_methods[] = {
    {"closeChar", (PyCFunction)Sbk_RichRubyParserFunc_closeChar, METH_NOARGS},
    {"containsRuby", (PyCFunction)Sbk_RichRubyParserFunc_containsRuby, METH_O},
    {"createRuby", (PyCFunction)Sbk_RichRubyParserFunc_createRuby, METH_VARARGS},
    {"openChar", (PyCFunction)Sbk_RichRubyParserFunc_openChar, METH_NOARGS},
    {"removeRuby", (PyCFunction)Sbk_RichRubyParserFunc_removeRuby, METH_O},
    {"renderTable", (PyCFunction)Sbk_RichRubyParserFunc_renderTable, METH_VARARGS|METH_KEYWORDS},
    {"setCloseChar", (PyCFunction)Sbk_RichRubyParserFunc_setCloseChar, METH_O},
    {"setOpenChar", (PyCFunction)Sbk_RichRubyParserFunc_setOpenChar, METH_O},
    {"setSplitChar", (PyCFunction)Sbk_RichRubyParserFunc_setSplitChar, METH_O},
    {"splitChar", (PyCFunction)Sbk_RichRubyParserFunc_splitChar, METH_NOARGS},

    {0} // Sentinel
};

} // extern "C"

static int Sbk_RichRubyParser_traverse(PyObject* self, visitproc visit, void* arg)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_traverse(self, visit, arg);
}
static int Sbk_RichRubyParser_clear(PyObject* self)
{
    return reinterpret_cast<PyTypeObject*>(&SbkObject_Type)->tp_clear(self);
}
// Class Definition -----------------------------------------------
extern "C" {
static SbkObjectType Sbk_RichRubyParser_Type = { { {
    PyVarObject_HEAD_INIT(&SbkObjectType_Type, 0)
    /*tp_name*/             "pyrich.RichRubyParser",
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
    /*tp_traverse*/         Sbk_RichRubyParser_traverse,
    /*tp_clear*/            Sbk_RichRubyParser_clear,
    /*tp_richcompare*/      0,
    /*tp_weaklistoffset*/   0,
    /*tp_iter*/             0,
    /*tp_iternext*/         0,
    /*tp_methods*/          Sbk_RichRubyParser_methods,
    /*tp_members*/          0,
    /*tp_getset*/           0,
    /*tp_base*/             reinterpret_cast<PyTypeObject*>(&SbkObject_Type),
    /*tp_dict*/             0,
    /*tp_descr_get*/        0,
    /*tp_descr_set*/        0,
    /*tp_dictoffset*/       0,
    /*tp_init*/             Sbk_RichRubyParser_Init,
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
static void RichRubyParser_PythonToCpp_RichRubyParser_PTR(PyObject* pyIn, void* cppOut) {
    Shiboken::Conversions::pythonToCppPointer(&Sbk_RichRubyParser_Type, pyIn, cppOut);
}
static PythonToCppFunc is_RichRubyParser_PythonToCpp_RichRubyParser_PTR_Convertible(PyObject* pyIn) {
    if (pyIn == Py_None)
        return Shiboken::Conversions::nonePythonToCppNullPtr;
    if (PyObject_TypeCheck(pyIn, (PyTypeObject*)&Sbk_RichRubyParser_Type))
        return RichRubyParser_PythonToCpp_RichRubyParser_PTR;
    return 0;
}

// C++ to Python pointer conversion - tries to find the Python wrapper for the C++ object (keeps object identity).
static PyObject* RichRubyParser_PTR_CppToPython_RichRubyParser(const void* cppIn) {
    PyObject* pyOut = (PyObject*)Shiboken::BindingManager::instance().retrieveWrapper(cppIn);
    if (pyOut) {
        Py_INCREF(pyOut);
        return pyOut;
    }
    const char* typeName = typeid(*((::RichRubyParser*)cppIn)).name();
    return Shiboken::Object::newObject(&Sbk_RichRubyParser_Type, const_cast<void*>(cppIn), false, false, typeName);
}

void init_RichRubyParser(PyObject* module)
{
    SbkpyrichTypes[SBK_RICHRUBYPARSER_IDX] = reinterpret_cast<PyTypeObject*>(&Sbk_RichRubyParser_Type);

    if (!Shiboken::ObjectType::introduceWrapperType(module, "RichRubyParser", "RichRubyParser*",
        &Sbk_RichRubyParser_Type, &Shiboken::callCppDestructor< ::RichRubyParser >)) {
        return;
    }

    // Register Converter
    SbkConverter* converter = Shiboken::Conversions::createConverter(&Sbk_RichRubyParser_Type,
        RichRubyParser_PythonToCpp_RichRubyParser_PTR,
        is_RichRubyParser_PythonToCpp_RichRubyParser_PTR_Convertible,
        RichRubyParser_PTR_CppToPython_RichRubyParser);

    Shiboken::Conversions::registerConverterName(converter, "RichRubyParser");
    Shiboken::Conversions::registerConverterName(converter, "RichRubyParser*");
    Shiboken::Conversions::registerConverterName(converter, "RichRubyParser&");
    Shiboken::Conversions::registerConverterName(converter, typeid(::RichRubyParser).name());



}
