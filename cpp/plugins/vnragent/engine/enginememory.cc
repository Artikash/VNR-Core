// enginememory.cc
// 5/11/2014 jichi

#include "engine/enginememory.h"

/** Private class */

class EngineSharedMemoryPrivate
{
  typedef EngineSharedMemory Q;
public:
  enum { DataTextCapacity = 4096 };
  struct Data {
    Q::DataStatus requestStatus,
                  responseStatus;
    ulong requestKey,
          responseKey;
    int requestRole,
        responseRole;
    char requestText[DataTextCapacity],
         responseText[DataTextCapacity];

    Data() : requestStatus(Q::EmptyStatus), responseStatus(Q::EmptyStatus)  {}
  };
  Data data;
};

/** Public class */

EngineSharedMemory::EngineSharedMemory() : d_(new D) {}
EngineSharedMemory::~EngineSharedMemory() { delete d_; }

// Properties

EngineSharedMemory::DataStatus EngineSharedMemory::requestStatus() const { return d_->data.requestStatus; }
void EngineSharedMemory::setRequestStatus(DataStatus v) { d_->data.requestStatus = v; }

EngineSharedMemory::DataStatus EngineSharedMemory::responseStatus() const { return d_->data.responseStatus; }
void EngineSharedMemory::setResponseStatus(DataStatus v) { d_->data.responseStatus = v; }

int EngineSharedMemory::requestKey() const { return d_->data.requestKey; }
void EngineSharedMemory::setRequestKey(int v) { d_->data.requestKey = v; }

ulong EngineSharedMemory::responseKey() const { return d_->data.responseKey; }
void EngineSharedMemory::setResponseKey(ulong v) { d_->data.responseKey = v; }

int EngineSharedMemory::requestRole() const { return d_->data.requestRole; }
void EngineSharedMemory::setRequestRole(int v) { d_->data.requestRole = v; }

int EngineSharedMemory::responseRole() const { return d_->data.responseRole; }
void EngineSharedMemory::setResponseRole(int v) { d_->data.responseRole = v; }

int EngineSharedMemory::requestTextCapacity() { return D::DataTextCapacity; }
const char *EngineSharedMemory::requestText() const { return d_->data.requestText; }
void EngineSharedMemory::setRequestText(const char *v)
{
  //if (!v) return;
  int size = qMin<int>(::strlen(v), D::DataTextCapacity);
  ::memcpy(d_->data.requestText, v, size);
}

int EngineSharedMemory::responseTextCapacity() { return D::DataTextCapacity; }
const char *EngineSharedMemory::responseText() const { return d_->data.responseText; }
void EngineSharedMemory::setResponseText(const char *v)
{
  //if (!v) return;
  int size = qMin<int>(::strlen(v) + 1, D::DataTextCapacity);
  ::memcpy(d_->data.responseText, v, size);
}

// EOF
