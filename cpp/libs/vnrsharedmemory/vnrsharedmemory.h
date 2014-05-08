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
  explicit VnrSharedMemory(QObject *parent = nullptr);
  ~VnrSharedMemory();

  // Attach

  QString key() const;
  void setKey(const QString &v);

  int size() const;

  ///  The initial size must be larger than 20.
  bool create(int size, bool readOnly = false);
  bool attach(bool readOnly = false);
  bool detach();
  bool isAttached() const;

  bool lock();
  bool unlock();

  bool hasError() const;
  QString errorString() const;

  // Contents

  const char *constData() const; // for debuggong purpose
  int maximumTextSize() const;

  qint64 hash() const;
  void setHash(qint64 v);

  qint32 role() const;
  void setRole(qint32 v);

  QString text() const;
  void setText(const QString &v);
};

#endif // VNRSHAREDMEMORY_H
