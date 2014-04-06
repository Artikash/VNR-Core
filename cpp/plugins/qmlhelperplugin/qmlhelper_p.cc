// qmlhelper_p.cc
// 12/9/2012 jichi

#include "qmlhelperplugin/qmlhelper_p.h"
#include "qtprivate/d/d_qdeclarativetextedit.h"
#include <QtCore/QObject>

QmlHelper::QmlHelper(QObject *parent): Base(parent) {}

QObject *QmlHelper::textedit_document(QDeclarativeItem *q)
{
  //if (auto p = dynamic_cast<const QDeclarativeTextEdit *>(q))
  //  return QtPrivate::d_qdeclarativetextedit_document(p);
  //else
  //  return nullptr;
  if (!q || !q->metaObject() || qstrcmp("QDeclarativeTextEdit", q->metaObject()->className()))
    return nullptr;
  return QtPrivate::d_qdeclarativetextedit_document(static_cast<const QDeclarativeTextEdit *>(q));
}

// EOF
