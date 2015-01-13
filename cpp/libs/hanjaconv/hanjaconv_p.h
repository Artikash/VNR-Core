#ifndef HANJACONV_P_H
#define HANJACONV_P_H

// hanjaconv_p.h
// 1/6/2015 jichi

#include "hanjaconv/hanjaconv.h"

/** Private class */

class HanjaConverterPrivate
{
  typedef HanjaConverter Q;
  struct Entry
  {
    std::wstring hangul,  // source
                 hanja;   // target

    void reset(const std::wstring &first, const std::wstring &second)
    {
      hangul = first;
      hanja = second;
    }
  };

public:
  typedef Entry entry_type;

  entry_type *entries;
  size_t entry_count;

  HanjaConverterPrivate() : entries(nullptr), entry_count(0) {}
  ~HanjaConverterPrivate() { if (entries) delete[] entries; }

  void clear()
  {
    entry_count = 0;
    if (entries) {
      delete[] entries;
      entries = nullptr;
    }
  }

  void resize(size_t size)
  {
    //Q_ASSERT(size > 0);
    if (entry_count != size) {
      clear(); // clear first for thread-safety
      if (entries)
        delete[] entries;
      entries = new entry_type[size];
      entry_count = size;
    }
  }

  // Replace hangul with hanja in text
  void replace(std::wstring &text) const;
  // Detect (hangul, hanja) pairs and pass to fun
  void collect(const std::wstring &text, const Q::collect_fun_t &fun) const;

  // Helpers
private:
  /** Replace only at text[start].
   *  @param  text  target text to replace
   *  @param  start  offset of the text to look at
   */
  size_t replace_first(std::wstring &text, size_t start) const;

  /** Collect only at text[start].
   *  @param  text  target text to replace
   *  @param  start  offset of the text to look at
   *  @param  last  where last collect happens
   *  @param  fun
   */
  size_t collect_first(const std::wstring &text, size_t start, size_t last,
                       const Q::collect_fun_t &fun) const;
};

#endif // HANJACONV_P_H
