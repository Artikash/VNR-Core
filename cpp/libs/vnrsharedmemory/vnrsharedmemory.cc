// vnrsharedmemory.cc
// 5/7/2014 jichi
#include "vnrsharedmemory/vnrsharedmemory.h"
#include <QtCore/QSharedMemory>

/** Private class */

class VnrSharedMemoryPrivate
{
public:
  struct Head {
    qint8 status;
    qint64 hash;
    //qint32 signature;
    qint8 role;
    qint32 textSize;
    wchar_t text[1];
  private:
    Head() {} // disable constructor
  };

  QSharedMemory *memory;

  explicit VnrSharedMemoryPrivate(QObject *parent)
    : memory(new QSharedMemory(parent)) {}
  VnrSharedMemoryPrivate(const QString &key, QObject *parent)
    : memory(new QSharedMemory(key, parent)) {}

  Head *head() { return reinterpret_cast<Head *>(memory->data()); }
  const Head *constHead() const { return reinterpret_cast<const Head *>(memory->constData()); }

  int textCapacity() const
  { return (memory->size() - sizeof(Head)) / 2; }
};

/** Public class */

VnrSharedMemory::VnrSharedMemory(QObject *parent)
  : Base(parent), d_(new D(this)) {}

VnrSharedMemory::VnrSharedMemory(const QString &key, QObject *parent)
  : Base(parent), d_(new D(key, this)) {}

VnrSharedMemory::~VnrSharedMemory() { delete d_; }

// Shared Memory
QString VnrSharedMemory::key() const { return d_->memory->key(); }
void VnrSharedMemory::setKey(const QString &v) { d_->memory->setKey(v); }

int VnrSharedMemory::size() const { return d_->memory->size(); }

bool VnrSharedMemory::create(int size, bool readOnly)
{ return d_->memory->create(size, readOnly ? QSharedMemory::ReadOnly : QSharedMemory::ReadWrite); }

bool VnrSharedMemory::attach(bool readOnly)
{ return d_->memory->attach(readOnly ? QSharedMemory::ReadOnly : QSharedMemory::ReadWrite); }

bool VnrSharedMemory::detach() { return d_->memory->detach(); }
bool VnrSharedMemory::isAttached() const { return d_->memory->isAttached(); }

bool VnrSharedMemory::lock() { return d_->memory->lock(); }
bool VnrSharedMemory::unlock() { return d_->memory->unlock(); }

QString VnrSharedMemory::errorString() const { return d_->memory->errorString(); }
bool VnrSharedMemory::hasError() const { return d_->memory->error() != QSharedMemory::NoError; }

// Contents

const char *VnrSharedMemory::constData() const
{ return reinterpret_cast<const char *>(d_->memory->constData()); }

int VnrSharedMemory::dataTextCapacity() const
{ return qMax(0, d_->textCapacity()); }

qint64 VnrSharedMemory::dataHash() const
{
  if (auto h = d_->constHead())
    return h->hash;
  else
    return 0;
}

void VnrSharedMemory::setDataHash(qint64 v)
{
  if (auto h = d_->head())
    h->hash = v;
}

qint8 VnrSharedMemory::dataStatus() const
{
  if (auto h = d_->constHead())
    return h->status;
  else
    return 0;
}

void VnrSharedMemory::setDataStatus(qint8 v)
{
  if (auto h = d_->head())
    h->status = v;
}

//qint32 VnrSharedMemory::dataSignature() const
//{
//  if (auto h = d_->constHead())
//    return h->signature;
//  else
//    return 0;
//}
//
//void VnrSharedMemory::setDataSignature(qint32 v)
//{
//  if (auto h = d_->head())
//    h->signature = v;
//}

qint8 VnrSharedMemory::dataRole() const
{
  if (auto h = d_->constHead())
    return h->role;
  else
    return 0;
}

void VnrSharedMemory::setDataRole(qint8 v)
{
  if (auto h = d_->head())
    h->role = v;
}

QString VnrSharedMemory::dataText() const
{
  if (auto h = d_->constHead())
    if (h->textSize > 0 && h->textSize <= d_->textCapacity())
      return QString::fromWCharArray(h->text, h->textSize);
  return QString();
}

void VnrSharedMemory::setDataText(const QString &v)
{
  if (auto h = d_->head()) {
    int limit = d_->textCapacity();
    if (v.size() <= limit) {
      v.toWCharArray(h->text);
      h->textSize = v.size();
    } else {
      QString w = v.left(limit);
      w.toWCharArray(h->text);
      h->textSize = w.size();
    }
  }
}

// EOF
