#ifndef D_QOBJECT_H
#define D_QOBJECT_H

// d_qobject.h
// 12/9/2012 jichi

#include <QtCore/QObject>

QT_FORWARD_DECLARE_CLASS(QObjectPrivate)

namespace QtPrivate {

class DQObject : public QObject
{
  DQObject() {}
public:
  QObjectPrivate d() const { return d_ptr.data(); }
};

inline QObjectPrivate *d_qobject(const QObject *q)
{ return !q ? nullptr : static_cast<const DQObject *>(q)->d(); }

inline QObjectPrivate *d_q(const QObject *q)
{ return d_qobject(q); }

} // namespace QtPrivate

#endif // D_QOBJECT_H
