#ifndef DATAMAN_H
#define DATAMAN_H

// dataman.h
// 2/1/2013 jichi

#include "winhook/qt/mydef.h"
#include "sakurakit/skglobal.h"
#include <QtCore/QList>
#include <QtCore/QObject>

class DataManagerPrivate;
class DataManager : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(DataManager)
  SK_EXTEND_CLASS(DataManager, QObject)
  SK_DECLARE_PRIVATE(DataManagerPrivate)

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

  explicit DataManager(QObject *parent = nullptr);
  ~DataManager();

public:
  bool containsAnchor(ulong anchor) const;
  bool containsText(qint64 hash) const;
  bool containsTranslation(qint64 trhash) const;

  const TextEntry &findTextEntry(qint64 hash) const;
  const TextEntry &findTextEntryAtAnchor(ulong anchor) const;
  QString findTextTranslation(qint64 hash) const;

  void updateText(const QString &text, qint64 hash, ulong anchor);
  void updateTextTranslation(const QString &tr, qint64 hash, qint64 trhash = 0);

signals:
  //void translationChanged(); // send to main object
  void textsChanged(QString json); // send to server

public slots:
  void updateTranslation(const QString &json); // received from the server
  void clearTranslation();
};

#endif // DATAMAN_H
