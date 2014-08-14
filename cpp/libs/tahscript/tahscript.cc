// tahscript.cc
// 8/14/2014 jichi

#include "tahscript/tahscript.h"
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

#define DEBUG "tahscript.cc"
#include "sakurakit/skdebug.h"

/** Helpers */

namespace { // unnamed

struct TahScriptRule
{
  QString source;
  QString target;
  QRegExp *sourceRe; // cached compiled regex

  TahScriptRule() : sourceRe(nullptr) {}
  ~TahScriptRule() { if (sourceRe) delete sourceRe; }
};

} // unnamed namespace


/** Private class */

class TahScriptManagerPrivate
{
public:
  TahScriptRule *rules; // use array for performance reason
  int ruleCount;

  TahScriptManagerPrivate() : rules(nullptr), ruleCount(0) {}
  ~TahScriptManagerPrivate() { if (rules) delete rules; }

  void clear()
  {
    ruleCount = 0;
    if (rules) {
      delete rules;
      rules = nullptr;
    }
  }
};

/** Public class */

// Construction

TahScriptManager::TahScriptManager() : d_(new D) {}
TahScriptManager::~TahScriptManager() { delete d_; }


int TahScriptManager::size() const { return d_->ruleCount; }
bool TahScriptManager::isEmpty() const { return d_->ruleCount; }

void TahScriptManager::clear() { d_->clear(); }

// Initialization
bool TahScriptManager::loadFile(const QString &path)
{
  // File IO
  // http://stackoverflow.com/questions/2612103/qt-reading-from-a-text-file
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    DOUT("failed to open file at path:" << path);
    return false;
  }

  QStringList lines;

  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine();
  }

  file.close();
  return false;
}

// Translation
QString TahScriptManager::translate(const QString &text) const
{
  QString ret = text;
  if (d_->ruleCount && d_->rules)
    for (int i = 0; i < d_->ruleCount; i++) {
      const TahScriptRule &rule = d_->rules[i];
      if (rule.sourceRe) {
        if (rule.target.isEmpty())
          ret.remove(*rule.sourceRe);
        else
          ret.replace(*rule.sourceRe, rule.target);
      } else {
        if (rule.target.isEmpty())
          ret.remove(rule.source);
        else
          ret.replace(rule.source, rule.target);
      }
    }
  return ret;
}

// EOF
