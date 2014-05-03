#pragma once

// windowmanager.h
// 2/1/2013 jichi
// Window text manager.

#include "sakurakit/skglobal.h"
#include <QtCore/QList>
#include <QtCore/QObject>

class WindowManagerPrivate;
class WindowManager : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(WindowManager)
  SK_EXTEND_CLASS(WindowManager, QObject)
  SK_DECLARE_PRIVATE(WindowManagerPrivate)

public:
  struct TextEntry { // Window text entry QString text;
    QString text;
    qint64 hash;  // text hash
    ulong anchor; // window hash

    TextEntry() : hash(0), anchor(0) {}
    TextEntry(const QString &t, qint64 h, ulong a)
      : text(t), hash(h), anchor(a) {}
  };
  typedef QList<TextEntry> TextEntryList;

  explicit WindowManager(QObject *parent = nullptr);
  ~WindowManager();

public:
  bool containsAnchor(ulong anchor) const;
  bool containsText(qint64 hash) const;
  bool containsTranslation(qint64 trhash) const;

  const TextEntry &findTextEntry(qint64 hash) const;
  const TextEntry &findTextEntryAtAnchor(ulong anchor) const;
  QString findTextTranslation(qint64 hash) const;

  void updateText(const QString &text, qint64 hash, ulong anchor);
  void updateTextTranslation(const QString &tr, qint64 hash, qint64 trhash = 0);

  void updateTranslation(const QString &json); // received from the server
  void clearTranslation();

signals:
  //void translationChanged(); // send to main object
  void textsChanged(QString json); // send to server

private slots:
  void refreshTexts(); // invoked by refresh timer
};

// EOF
