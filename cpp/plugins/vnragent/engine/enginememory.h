#pragma once

// enginememory.h
// 5/11/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QtGlobal>

/**
 *  This class is used to exchange data between the painting thread and Qt thread.
 *  Qt/QString/QByteArray must be avoided from accessing the data.
 */
class EngineSharedMemoryPrivate;
class EngineSharedMemory
{
  SK_CLASS(EngineSharedMemory)
  SK_DISABLE_COPY(EngineSharedMemory)
  SK_DECLARE_PRIVATE(EngineSharedMemoryPrivate)
public:
  EngineSharedMemory();
  ~EngineSharedMemory();

  enum DataStatus {
    EmptyStatus = 0 // There is no data
    , ReadyStatus   // The data is ready to read
    , BusyStatus    // The producer is busy writing the data
    , CancelStatus  // The data requires is cancelled
  };

  DataStatus requestStatus() const;
  void setRequestStatus(DataStatus status);

  DataStatus responseStatus() const;
  void setResponseStatus(DataStatus status);

  ulong requestKey() const;
  void setRequestKey(ulong role);

  ulong responseKey() const;
  void setResponseKey(ulong role);

  int requestRole() const;
  void setRequestRole(int role);

  int responseRole() const;
  void setResponseRole(int role);

  int responseRole() const;
  void setResponseRole(int role);

  static int requestTextCapacity();
  const char *requestText() const;
  void setRequestText(const char *data);

  static int responseTextCapacity();
  const char *responseText() const;
  void setResponseText(const char *data);
};

// EOF
