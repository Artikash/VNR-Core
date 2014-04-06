#ifndef _QMLTEXT_CONTOURED_TEXTEDIT_H
#define _QMLTEXT_CONTOURED_TEXTEDIT_H

// contouredtext.h
// 4/5/2014 jichi

#include "sakurakit/skglobal.h"
#include <qt/src/declarative/graphicsitems/qdeclarativetextedit_p.h>

class DeclarativeContouredTextEditPrivate;
class DeclarativeContouredTextEdit : public QDeclarativeTextEdit
{
  Q_OBJECT
  SK_EXTEND_CLASS(DeclarativeContouredTextEdit, QDeclarativeTextEdit)
  Q_DISABLE_COPY(DeclarativeContouredTextEdit)
public:
  DeclarativeContouredTextEdit(QDeclarativeItem *parent = nullptr);
  ~DeclarativeContouredTextEdit();
};

QML_DECLARE_TYPE(DeclarativeContouredTextEdit)

#endif // _QMLTEXT_CONTOURED_TEXTEDIT_H
