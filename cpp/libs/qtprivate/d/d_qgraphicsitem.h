#ifndef D_QGRAPHICSITEM_H
#define D_QGRAPHICSITEM_H

// d_qobject.h
// 12/9/2012 jichi

#include <QtGui/QGraphicsItem>

QT_FORWARD_DECLARE_CLASS(QGraphicsItemPrivate)

namespace QtPrivate {

class DQGraphicsItem : public QGraphicsItem
{
  DQGraphicsItem() {}
public:
  QGraphicsItemPrivate *d() const { return d_ptr.data(); }
};

inline QGraphicsItemPrivate *d_qgraphicsitem(const QGraphicsItem *q)
{ return !q ? nullptr : static_cast<const DQGraphicsItem *>(q)->d(); }

template <typename T>
inline T d_q(const QGraphicsItem *q)
{ return static_cast<T>(d_qgraphicsitem(q)); }

} // namespace QtPrivate

#endif // D_QGRAPHICSITEM_H
