#ifndef MOUSESELECTOR_H
#define MOUSESELECTOR_H

// mouseselector.h
// 8/21/2014 jichi

#include <QtCore/QObject>
#include "sakurakit/skglobal.h"

QT_FORWARD_DECLARE_CLASS(QWidget)

class MouseSelectorPrivate;
class MouseSelector : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(MouseSelector)
  SK_EXTEND_CLASS(MouseSelector, QObject)
  SK_DECLARE_PRIVATE(MouseSelectorPrivate)
public:
  explicit MouseSelector(QObject *parent = nullptr);
  ~MouseSelector();

  //QWidget *parentWidget() const;
  //void setParentWidget(QWidget *parentWidget);

  bool isEnabled() const;
  void setEnabled(bool t);

signals:
  void selected(int x, int y, int width, int height);
};

#endif // MOUSESELECTOR_H
