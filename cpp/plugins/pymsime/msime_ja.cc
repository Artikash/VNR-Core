// msime_ja.cc
// 4/1/2013
#include "pymsime/msime_ja.h"
#include "winime/winime.h"

// - Helper classes -

enum {
  MSIME_MODE_FLAGS =
    WINIME_MODE_AUTOMATIC |
    WINIME_MODE_SINGLECONVERT |
    WINIME_MODE_PLAURALCLAUSE |
    WINIME_MODE_PHRASEPREDICT
    //WINIME_MODE_NONE
};

namespace { namespace detail { // unnamed

class ime_applier
{
  QString *d_;
public:
  explicit ime_applier(QString *d) : d_(d) { Q_ASSERT(d_); }
  void operator()(const wchar_t *ws, size_t len)
  { d_->append(QString::fromWCharArray(ws, len)); }
};

class ime_collector
{
  typedef QList<QPair<QString, QString> > D;
  D *d_;
public:
  explicit ime_collector(D *d) : d_(d) { Q_ASSERT(d_); }
  void operator()(const wchar_t *sx, size_t nx, const wchar_t *sy, size_t ny)
  {
    d_->append(qMakePair(QString::fromWCharArray(sx, nx),
                         QString::fromWCharArray(sy, ny)));
  }
};

}} // unnamed detail

/** Private class */

class Msime_ja_p
{
public:
  winime_t *ime;
  Msime_ja_p(): ime(nullptr) {}
};

/** Public class */

// - Construction -

Msime_ja::Msime_ja() : d_(new D)
{ d_->ime = ::winime_create(WINIME_CLASS_JA); }

Msime_ja::~Msime_ja()
{
  ::winime_destroy(d_->ime);
  delete d_;
}

bool Msime_ja::isValid() const { return d_->ime; }

// - Actions -

QString Msime_ja::toYomigana(const QString& text, int type) const
{
  QString ret;
  if (d_->ime)
    ::winime_apply(d_->ime,
         WINIME_REQ_REV,
         MSIME_MODE_FLAGS | (type == Katagana ? WINIME_MODE_KATAGANA : WINIME_MODE_HIRAGANA),
         detail::ime_applier(&ret),
         text.toStdWString().c_str(),
         text.size());
  return ret;
}

QList<QPair<QString, QString> > Msime_ja::toFurigana(const QString& text, int type) const
{
  QList<QPair<QString, QString> > ret;
  if (d_->ime)
    ::winime_collect(d_->ime,
         WINIME_REQ_REV,
         MSIME_MODE_FLAGS | (type == Katagana ? WINIME_MODE_KATAGANA : WINIME_MODE_HIRAGANA),
         detail::ime_collector(&ret),
         text.toStdWString().c_str(),
         text.size());
  return ret;
}

QString Msime_ja::toKanji(const QString &text, ulong flags) const
{
  QString ret;
  if (d_->ime)
    ::winime_apply(d_->ime,
         flags & Autocorrect ? WINIME_REQ_RECONV : WINIME_REQ_CONV,
         MSIME_MODE_FLAGS,
         detail::ime_applier(&ret),
         text.toStdWString().c_str(),
         text.size());
  return ret;
}

// EOF
