#ifndef D_QDECLARATIVETEXTEDIT_H
#define D_QDECLARATIVETEXTEDIT_H

// qobjectprivate.h
// 12/9/2012 jichi

#ifdef __clang__
# pragma clang diagnostic ignored "-Wunused-private-field"  // in <qmime.h> included by <qtextcontrol_p>
#endif // __clang__

#include "qtprivate/d/d_qgraphicsitem.h"

//#include <qt/src/gui/text/qtextcontrol_p.h>
#include <qt/src/declarative/graphicsitems/qdeclarativetextedit_p.h>
#include <qt/src/declarative/graphicsitems/qdeclarativetextedit_p_p.h>

QT_FORWARD_DECLARE_CLASS(QDeclarativeTextEdit)
QT_FORWARD_DECLARE_CLASS(QDeclarativeTextEditPrivate)
QT_FORWARD_DECLARE_CLASS(QTextDocument)

namespace QtPrivate {

inline QDeclarativeTextEditPrivate *d_qdeclarativetextedit(const QDeclarativeTextEdit *q)
{ return static_cast<QDeclarativeTextEditPrivate *>(d_qgraphicsitem(q)); }

// See: https://codereview.qt-project.org/#change,23617
inline QTextDocument *d_qdeclarativetextedit_document(const QDeclarativeTextEdit *q)
//{ return !q ? nullptr : d_qdeclarativetextedit(q)->control->document(); }
{ return !q ? nullptr : d_qdeclarativetextedit(q)->document; }

} // namespace QtPrivate

#endif // D_QDECLARATIVETEXTEDIT_H
