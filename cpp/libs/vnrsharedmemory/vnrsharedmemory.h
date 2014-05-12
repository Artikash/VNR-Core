#ifndef VNRSHAREDMEMORY_H
#define VNRSHAREDMEMORY_H

// contour.h
// 4/5/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QObject>

class VnrSharedMemoryPrivate;
class VnrSharedMemory : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(VnrSharedMemory)
  SK_EXTEND_CLASS(VnrSharedMemory, QObject)
  SK_DECLARE_PRIVATE(VnrSharedMemoryPrivate)
public:
  explicit VnrSharedMemory(const QString &key, QObject *parent = nullptr);
  explicit VnrSharedMemory(QObject *parent = nullptr);
  ~VnrSharedMemory();

  // Attach

  QString key() const;
  void setKey(const QString &v);

  int size() const;

  ///  The initial size must be larger than 20.
  bool create(int size, bool readOnly = false);
  bool attach(bool readOnly = false);
  bool isAttached() const;

  bool detach();
  bool detach_() { return detach(); } // escape keyword for Shiboken

  bool lock();
  bool unlock();

  bool hasError() const;
  QString errorString() const;

  // Contents

  const char *constData() const; // for debuggong purpose

  qint8 dataStatus() const;
  void setDataStatus(qint8 v);

  qint64 dataHash() const;
  void setDataHash(qint64 v);

  qint8 dataRole() const;
  void setDataRole(qint8 v);

  QString dataText() const;
  void setDataText(const QString &v);
  int dataTextCapacity() const;
};

#endif // VNRSHAREDMEMORY_H
