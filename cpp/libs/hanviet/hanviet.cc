// hanviet.cc
// 2/2/2015 jichi

#include "hanviet/hanviet.h"
#include "hanviet/phrasedic.h"
#include "hanviet/worddic.h"
#include "unistr/unistr.h"
//#include <iostream>
//#include <QDebug>

/** Private class */

class HanVietTranslatorPrivate
{
public:
  HanVietWordDictionary *wordDic;
  HanVietPhraseDictionary *phraseDic;

  HanVietTranslatorPrivate()
    : wordDic(new HanVietWordDictionary)
    , phraseDic(new HanVietPhraseDictionary)
  {}

  ~HanVietTranslatorPrivate()
  {
    delete wordDic;
    delete phraseDic;
  }
};

/** Public class */

// Construction

HanVietTranslator::HanVietTranslator() : d_(new D) {}
HanVietTranslator::~HanVietTranslator() { delete d_; }

void HanVietTranslator::clear()
{
  d_->wordDic->clear();
  d_->phraseDic->clear();
}

HanVietWordDictionary *HanVietTranslator::wordDicionary() const
{ return d_->wordDic; }

HanVietPhraseDictionary *HanVietTranslator::phraseDicionary() const
{ return d_->phraseDic; }

bool HanVietTranslator::addWordFile(const std::wstring &path)
{ return d_->wordDic->addFile(path); }

bool HanVietTranslator::addPhraseFile(const std::wstring &path)
{ return d_->phraseDic->addFile(path); }

// Conversion

std::wstring HanVietTranslator::lookupWord(int ch) const
{ return d_->wordDic->lookup(ch); }

std::wstring HanVietTranslator::lookupPhrase(const std::wstring &text) const
{ return d_->phraseDic->lookup(text); }

std::wstring HanVietTranslator::toReading(const std::wstring &text) const
{ return d_->wordDic->translate(text); }

std::wstring HanVietTranslator::translate(const std::wstring &text, bool mark) const
{
  std::wstring ret = d_->phraseDic->translate(text);
  ret = d_->wordDic->translate(ret);
  unistr::to_thin(ret);
  return ret;
}

// EOF
