// hijackhelper.cc
// 5/1/2014 jichi
#include "hijack/hijackhelper.h"
#include "util/codepage.h"
#include <qt_windows.h>

/** Private class */

class HijackHelperPrivate
{
public:
  uint systemCodePage;
  quint8 systemCharSet;

  QString encoding;
  uint encodingCodePage;
  bool encodingEnabled;

  HijackHelperPrivate()
    : encodingCodePage(0)
    , encodingEnabled(true)
  {
    systemCodePage = ::GetACP();
    systemCharSet = Util::charSetForCodePage(systemCodePage);
  }
};

/** Public class */

static HijackHelper *instance_;
HijackHelper *HijackHelper::instance() { return ::instance_; }

HijackHelper::HijackHelper(QObject *parent)
  : Base(parent), d_(new D)
{ ::instance_ = this; }

HijackHelper::~HijackHelper()
{
  ::instance_ = nullptr;
  delete d_;
}

// - Properties -

quint8 HijackHelper::systemCharSet() const
{ return d_->systemCharSet; }

void HijackHelper::setEncoding(const QString &v)
{
  if (d_->encoding != v) {
    d_->encoding = v;
    d_->encodingCodePage = Util::codePageForEncoding(v);
  }
}

void HijackHelper::setEncodingEnabled(bool t)
{ d_->encodingEnabled = t; }

// - Encoding -

bool HijackHelper::isTranscodingNeeded() const
{ return d_->encodingEnabled && d_->encodingCodePage != d_->systemCodePage; }

// EOF
