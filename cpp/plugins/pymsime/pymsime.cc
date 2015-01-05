// pymsime.cc
// 4/1/2013
#include "pymsime/pymsime.h"
#include "pymsime/pymsime_p.h"
#include "winime/winime.h"

#define DEBUG "msime.cc"
#include "sakurakit/skdebug.h"

// Helper functions

namespace { // unnamed

enum {
  MSIME_MODE_FLAGS =
    WINIME_MODE_AUTOMATIC |
    WINIME_MODE_SINGLECONVERT |
    WINIME_MODE_PLAURALCLAUSE |
    WINIME_MODE_PHRASEPREDICT
    //WINIME_MODE_NONE
};

inline ulong ruby2mode(int ruby)
{
  switch (ruby) {
  case Msime::Roman: return WINIME_MODE_ROMAN;
  case Msime::Pinyin: return WINIME_MODE_PINYIN;
  case Msime::Hiragana: return WINIME_MODE_HIRAGANA;
  case Msime::Katagana: return WINIME_MODE_KATAGANA;
  case Msime::Hangul: return WINIME_MODE_HANGUL;
  default: return 0;
  }
}

inline const wchar_t *lang2class(int lang)
{
  switch (lang) {
  case Msime::SimplifiedChinese: return WINIME_CLASS_CN;
  case Msime::TraditionalChinese: return WINIME_CLASS_TW;
  case Msime::Japanese: return WINIME_CLASS_JA;
  case Msime::Korean: return WINIME_CLASS_KO;
  default: return nullptr;
  }
}

} // unnamed namespace

/** Private class */

class Msime_p
{
public:
  winime_t *ime;
  Msime_p(): ime(nullptr) {}
};

/** Public class */

// - Construction -

Msime::Msime(int lang) : d_(new D)
{
  const wchar_t *cls = ::lang2class(lang);
  if (!cls)
    return;
  DOUT("class =" << QString::fromWCharArray(cls));
  d_->ime = ::winime_create(cls);
}

Msime::~Msime()
{
  ::winime_destroy(d_->ime);
  delete d_;
}

bool Msime::isValid() const { return d_->ime; }

// - Actions -

QString Msime::toKanji(const QString &text, ulong flags) const
{
  QString ret;
  if (d_->ime)
    ::winime_apply(d_->ime,
        flags & Autocorrect ? WINIME_REQ_RECONV : WINIME_REQ_CONV,
        MSIME_MODE_FLAGS,
        (const wchar_t *)text.utf16(),
        text.size(),
        ime_applier(&ret));
  return ret;
}

QList<QPair<QString, QString> > Msime::toKanjiList(const QString &text, ulong flags) const
{
  QList<QPair<QString, QString> > ret;
  if (d_->ime)
    ::winime_collect(d_->ime,
        flags & Autocorrect ? WINIME_REQ_RECONV : WINIME_REQ_CONV,
        MSIME_MODE_FLAGS,
        (const wchar_t *)text.utf16(),
        text.size(),
        ime_collector(&ret));
  return ret;
}

QString Msime::toRuby(const QString &text, int ruby) const
{
  QString ret;
  if (d_->ime)
    ::winime_apply(d_->ime,
        WINIME_REQ_REV,
        MSIME_MODE_FLAGS | ::ruby2mode(ruby),
        (const wchar_t *)text.utf16(),
        text.size(),
        ime_applier(&ret));
  return ret;
}

QList<QPair<QString, QString> > Msime::toRubyList(const QString &text, int ruby) const
{
  QList<QPair<QString, QString> > ret;
  if (d_->ime)
    ::winime_collect(d_->ime,
        WINIME_REQ_REV,
        MSIME_MODE_FLAGS | ::ruby2mode(ruby),
        (const wchar_t *)text.utf16(),
        text.size(),
        ime_collector(&ret));
  return ret;
}

// EOF
