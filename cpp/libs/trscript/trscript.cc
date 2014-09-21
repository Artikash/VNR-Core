// trscript.cc
// 9/20/2014 jichi

#include "trscript/trscript.h"
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

//#include <QtCore/QReadWriteLock> // thread-safety
//#include <QtCore/QReadLocker>
//#include <QtCore/QWriteLocker>

#include <list> // instead of QList which is slow that stores pointers instead of elements
#include <tuple>
#include <boost/foreach.hpp>

#define DEBUG "trscript.cc"
#include "sakurakit/skdebug.h"

//#define DEBUG_RULE // output the rule that is applied

#define SCRIPT_CH_COMMENT   '#' // indicate the beginning of a line comment
#define SCRIPT_CH_REGEX     'r'

#define SCRIPT_RULE_DELIM        '\t' // deliminator of the rule pair
enum { SCRIPT_RULE_DELIM_LEN = 1 };
//enum { SCRIPT_RULE_DELIM_LEN = (sizeof(SCRIPT_RULE_DELIM)  - 1) }; // strlen

/** Helpers */

namespace { // unnamed

struct TranslationScriptRule
{
  QString source;
  QString target;
  QRegExp *sourceRe; // cached compiled regex

  TranslationScriptRule() : sourceRe(nullptr) {}
  ~TranslationScriptRule() { if (sourceRe) delete sourceRe; }

  bool init(const QString &s, const QString &t, bool regex)
  {
    if (regex) {
      QRegExp *re = new QRegExp(s, Qt::CaseSensitive, QRegExp::RegExp2); // use Perl-compatible syntax, default in Qt5
      if (re->isEmpty()) {
        DOUT("invalid regexp:" << s);
        delete re;
        return false;
      }
      sourceRe = re;
      target = t;
      //target.replace('$', '\\'); // convert Javascript RegExp to Perl
    } else {
      source = s;
      target = t;
    }
    return true;
  }

  bool init(const std::tuple<QString, QString, bool> &t)
  { return init(std::get<0>(t), std::get<1>(t), std::get<2>(t)); }

};

} // unnamed namespace

/** Private class */

class TranslationScriptManagerPrivate
{
public:
  //QReadWriteLock lock;

  TranslationScriptRule *rules; // use array for performance reason
  int ruleCount;

  TranslationScriptManagerPrivate() : rules(nullptr), ruleCount(0) {}
  ~TranslationScriptManagerPrivate() { if (rules) delete[] rules; }

  void clear()
  {
    ruleCount = 0;
    if (rules) {
      delete rules;
      rules = nullptr;
    }
  }

  void reset(int size)
  {
    DOUT(size);
    Q_ASSERT(size > 0);
    ruleCount = size;
    if (rules)
      delete[] rules;
    rules = new TranslationScriptRule[size];
  }
};

/** Public class */

// Construction

TranslationScriptManager::TranslationScriptManager() : d_(new D) {}
TranslationScriptManager::~TranslationScriptManager() { delete d_; }

int TranslationScriptManager::size() const { return d_->ruleCount; }
bool TranslationScriptManager::isEmpty() const { return !d_->ruleCount; }

void TranslationScriptManager::clear()
{
  //QWriteLocker locker(&d_->lock);
  d_->clear();
}

// Initialization
bool TranslationScriptManager::loadFile(const QString &path)
{
  // File IO
  // http://stackoverflow.com/questions/2612103/qt-reading-from-a-text-file
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    DOUT("failed to open file at path:" << path);
    return false;
  }

  std::list<std::tuple<QString, QString, bool> > lines; // pattern, text, regex

  QTextStream in(&file);
  in.setCodec("UTF-8"); // enforce UTF-8
  while (!in.atEnd()) {
    QString line = in.readLine(); //.trimmed(); // trim the ending new line char
    if (!line.isEmpty()) {
      bool regex = false;
      int textStartIndex = 1; // index of the text after flags, +1 to skip \t
      switch (line[0].unicode()) {
      case SCRIPT_CH_COMMENT: continue;
      case SCRIPT_CH_REGEX: regex = true; textStartIndex++; break;
      }
      int index = line.indexOf(SCRIPT_RULE_DELIM, textStartIndex);
      QString left, right;
      if (index == -1)
        left = line.mid(textStartIndex);
      else {
        left = line.mid(textStartIndex, index - textStartIndex); //.trimmed()
        right = line.mid(index + SCRIPT_RULE_DELIM_LEN);
      }
      lines.push_back(std::make_tuple(left, right, regex));
    }
  }
  file.close();

  if (lines.empty())
    return false;

  //QWriteLocker locker(&d_->lock);
  d_->reset(lines.size());

  int i = 0;
  BOOST_FOREACH (const auto &it, lines)
    d_->rules[i++].init(it);

  return true;
}

// Translation
QString TranslationScriptManager::translate(const QString &text) const
{
  //QReadLocker locker(&d_->lock);
  QString ret = text;
#ifdef DEBUG_RULE
  QString previous;
#endif // DEBUG_RULE
  if (d_->ruleCount && d_->rules)
    for (int i = 0; i < d_->ruleCount; i++) {
      const auto &rule = d_->rules[i];
      if (rule.sourceRe) {
        if (rule.target.isEmpty())
          ret.remove(*rule.sourceRe);
        else
          ret.replace(*rule.sourceRe, rule.target);
      } else if (!rule.source.isEmpty()) {
        if (rule.target.isEmpty())
          ret.remove(rule.source);
        else
          ret.replace(rule.source, rule.target);
      }

#ifdef DEBUG_RULE
      if (previous != ret) {
        if (rule.sourceRe)
          DOUT(rule.sourceRe->pattern() << rule.target << ret);
        else
          DOUT(rule.source << rule.target << ret);
      }
      previous = ret;
#endif // DEBUG_RULE
    }
  return ret;
}

// EOF
