#ifndef trsymbol_H
#define trsymbol_H

// trsymbol.h
// 9/20/2014 jichi

#include <string>

namespace trsymbol {
///  Return if text contains [[symbol]]
bool contains_symbol(const std::wstring &text);

///  Return {{x:id}} for [[x]]
std::wstring create_symbol(const std::wstring &token, int id);

///  Replace [[x]] by regular expression for {{x}}
std::wstring encode_symbol(const std::wstring &text);
};

#endif // trsymbol_H
