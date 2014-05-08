// vnrsharedmemory.cc
// 5/7/2014 jichi
#include "vnrsharedmemory/vnrsharedmemory.h"
#include <QtCore/QSharedMemory>

/** Private class */

class VnrSharedMemoryPrivate
{
public:
  struct Head {
    qint64 hash;
    qint32 role;
    qint32 textSize;
    wchar_t text[0];
  private:
    Head() {} // disable constructor
  };

  QSharedMemory *memory;
  explicit VnrSharedMemoryPrivate(QObject *parent)
    : memory(new QSharedMemory(parent)) {}

  Head *head() { return reinterpret_cast<Head *>(memory->data()); }
  const Head *constHead() const { return reinterpret_cast<const Head *>(memory->constData()); }

  int maxTextSize() const
  { return (memory->size() - sizeof(Head)) / 2; }
};

/** Public class */

VnrSharedMemory::VnrSharedMemory(QObject *parent)
  : Base(parent), d_(new D(this)) {}

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

int VnrSharedMemory::maximumTextSize() const
{ return qMax(0, d_->maxTextSize()); }

qint64 VnrSharedMemory::hash() const
{
  if (auto h = d_->constHead())
    return h->hash;
  else
    return 0;
}

void VnrSharedMemory::setHash(qint64 v)
{
  if (auto h = d_->head())
    h->hash = v;
}

qint32 VnrSharedMemory::role() const
{
  if (auto h = d_->constHead())
    return h->role;
  else
    return 0;
}

void VnrSharedMemory::setRole(qint32 v)
{
  if (auto h = d_->head())
    h->role = v;
}

QString VnrSharedMemory::text() const
{
  if (auto h = d_->constHead())
    if (h->textSize > 0 && h->textSize <= d_->maxTextSize())
      return QString::fromWCharArray(h->text, h->textSize);
  return QString();
}

void VnrSharedMemory::setText(const QString &v)
{
  if (auto h = d_->head()) {
    int limit = d_->maxTextSize();
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
