

#ifndef SBK_PYHANVIET_PYTHON_H
#define SBK_PYHANVIET_PYTHON_H

#include <sbkpython.h>
#include <conversions.h>
#include <sbkenum.h>
#include <basewrapper.h>
#include <bindingmanager.h>
#include <memory>

// Binded library includes
#include <hanviet.h>
// Conversion Includes - Primitive Types
#include <string>

// Type indices
#define SBK_HANVIETTRANSLATOR_IDX                                    0
#define SBK_pyhanviet_IDX_COUNT                                      1

// This variable stores all Python types exported by this module.
extern PyTypeObject** SbkpyhanvietTypes;

// This variable stores all type converters exported by this module.
extern SbkConverter** SbkpyhanvietTypeConverters;

// Converter indices
#define SBK_STD_WSTRING_IDX                                          0
#define SBK_pyhanviet_CONVERTERS_IDX_COUNT                           1

// Macros for type check

namespace Shiboken
{

// PyType functions, to get the PyObjectType for a type T
template<> inline PyTypeObject* SbkType< ::HanVietTranslator >() { return reinterpret_cast<PyTypeObject*>(SbkpyhanvietTypes[SBK_HANVIETTRANSLATOR_IDX]); }

} // namespace Shiboken

#endif // SBK_PYHANVIET_PYTHON_H

