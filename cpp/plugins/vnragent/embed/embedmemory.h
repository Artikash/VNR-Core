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
};

// EOF
