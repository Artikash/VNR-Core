// simplecc.cc
// 10/18/2014 jichi

#include "simplecc/simplecc.h"
#include <fstream>
#include <boost/unordered_map.hpp>
#include "cpputil/cpplocale.h"

//#define DELIM   '\t' // deliminator of the rule pair

/** Helpers */

static const std::locale UTF8_LOCALE = ::cpp_utf8_locale<wchar_t>();

/** Private class */

class SimpleChineseConverterPrivate
{
public:
  typedef boost::unordered_map<wchar_t, wchar_t>  map_type;
  map_type map;
};

/** Public class */

// Construction

SimpleChineseConverter::SimpleChineseConverter() : d_(new D) {}
SimpleChineseConverter::~SimpleChineseConverter() { delete d_; }

int SimpleChineseConverter::size() const { return d_->map.size(); }
bool SimpleChineseConverter::isEmpty() const { return d_->map.empty(); }

void SimpleChineseConverter::clear() { d_->map.clear(); }

// Initialization
bool SimpleChineseConverter::addFile(const std::wstring &path, bool reverse)
{
  std::wifstream fin(path);
  if(!fin.is_open())
    return false;
  fin.imbue(UTF8_LOCALE);

  if (reverse) {
    for (std::wstring line; std::getline(fin, line);)
      if (line.size() >= 3) {
        d_->map[line[2]] = line[0];
        if (line.size() >= 5)
          d_->map[line[4]] = line[0];
      }
  } else {
    for (std::wstring line; std::getline(fin, line);)
      if (line.size() >= 3)
        d_->map[line[0]] = line[2];
  }

  fin.close();
  return true;
}

// Conversion

std::wstring SimpleChineseConverter::convert(const std::wstring &text) const
{
  if (text.empty() || d_->map.empty())
    return text;

  std::wstring ret = text;
  D::map_type::iterator p;
  for (size_t i = 0; i < text.size(); i++) {
    p = d_->map.find(text[i]);
    if (p != d_->map.end())
      ret[i] = p->second;
  }
  return ret;
}

// EOF
