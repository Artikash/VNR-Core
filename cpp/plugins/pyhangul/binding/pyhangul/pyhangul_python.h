

#ifndef SBK_PYHANGUL_PYTHON_H
#define SBK_PYHANGUL_PYTHON_H

#include <sbkpython.h>
#include <conversions.h>
#include <sbkenum.h>
#include <basewrapper.h>
#include <bindingmanager.h>
#include <memory>

#include <pysidesignal.h>
// Module Includes
#include <pyside_qtcore_python.h>

// Binded library includes
#include <hangulconv.h>
// Conversion Includes - Primitive Types
#include <QString>
#include <signalmanager.h>
#include <typeresolver.h>
#include <QtConcurrentFilter>
#include <QStringList>
#include <qabstractitemmodel.h>

// Conversion Includes - Container Types
#include <QList>
#include <QMap>
#include <QStack>
#include <QMultiMap>
#include <QVector>
#include <QPair>
#include <pysideconversions.h>
#include <QSet>
#include <QQueue>
#include <QLinkedList>

// Type indices
#define SBK_HANGULHANJACONVERTER_IDX                                 0
#define SBK_pyhangul_IDX_COUNT                                       1

// This variable stores all Python types exported by this module.
extern PyTypeObject** SbkpyhangulTypes;

// This variable stores all type converters exported by this module.
extern SbkConverter** SbkpyhangulTypeConverters;

// Converter indices
#define SBK_PYHANGUL_QPAIR_QSTRING_QSTRING_IDX                       0 // QPair<QString, QString >
#define SBK_PYHANGUL_QLIST_QPAIR_QSTRING_QSTRING_IDX                 1 // QList<QPair<QString, QString > >
#define SBK_PYHANGUL_QLIST_QLIST_QPAIR_QSTRING_QSTRING_IDX           2 // QList<QList<QPair<QString, QString > > >
#define SBK_PYHANGUL_QLIST_QVARIANT_IDX                              3 // QList<QVariant >
#define SBK_PYHANGUL_QLIST_QSTRING_IDX                               4 // QList<QString >
#define SBK_PYHANGUL_QMAP_QSTRING_QVARIANT_IDX                       5 // QMap<QString, QVariant >
#define SBK_pyhangul_CONVERTERS_IDX_COUNT                            6

// Macros for type check

namespace Shiboken
{

// PyType functions, to get the PyObjectType for a type T
template<> inline PyTypeObject* SbkType< ::HangulHanjaConverter >() { return reinterpret_cast<PyTypeObject*>(SbkpyhangulTypes[SBK_HANGULHANJACONVERTER_IDX]); }

} // namespace Shiboken

#endif // SBK_PYHANGUL_PYTHON_H

