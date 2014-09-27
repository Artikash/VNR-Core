

#ifndef SBK_PYWINTTS_PYTHON_H
#define SBK_PYWINTTS_PYTHON_H

#include <sbkpython.h>
#include <conversions.h>
#include <sbkenum.h>
#include <basewrapper.h>
#include <bindingmanager.h>
#include <memory>

// Binded library includes
#include <pywintts.h>
// Conversion Includes - Primitive Types
#include <string>

// Type indices
#define SBK_WINTTS_IDX                                               0
#define SBK_pywintts_IDX_COUNT                                       1

// This variable stores all Python types exported by this module.
extern PyTypeObject** SbkpywinttsTypes;

// This variable stores all type converters exported by this module.
extern SbkConverter** SbkpywinttsTypeConverters;

// Converter indices
#define SBK_STD_WSTRING_IDX                                          0
#define SBK_pywintts_CONVERTERS_IDX_COUNT                            1

// Macros for type check

namespace Shiboken
{

// PyType functions, to get the PyObjectType for a type T
template<> inline PyTypeObject* SbkType< ::WinTts >() { return reinterpret_cast<PyTypeObject*>(SbkpywinttsTypes[SBK_WINTTS_IDX]); }

} // namespace Shiboken

#endif // SBK_PYWINTTS_PYTHON_H

