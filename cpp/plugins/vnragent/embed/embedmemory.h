#pragma once

// embedmemory.h
// 5/8/2014 jichi
// Shared memory reader.

#include "vnrsharedmemory/vnrsharedmemory.h"

class EmbedMemory : public VnrSharedMemory
{
  Q_OBJECT
  Q_DISABLE_COPY(EmbedMemory)
  SK_EXTEND_CLASS(EmbedMemory, VnrSharedMemory)
public:
  explicit EmbedMemory(QObject *parent = nullptr);
  ~EmbedMemory();

  bool create();

  enum { EmptyStatus = 0, ReadyStatus, BusyStatus, CancelStatus };

  bool isDataEmpty() const { return dataStatus() == EmptyStatus; }
  bool isDataReady() const { return dataStatus() == ReadyStatus; }
  bool isDataBusy() const { return dataStatus() == BusyStatus; }
  bool isDataCanceled() const { return dataStatus() == CancelStatus; }
};

// EOF
