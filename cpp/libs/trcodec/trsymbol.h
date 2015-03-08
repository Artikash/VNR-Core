#ifndef trsymbol_H
#define trsymbol_H

// trsymbol.h
// 9/20/2014 jichi

#include <string>

namespace trsymbol {

///  Return if source text contains [[symbol]]
bool contains_raw_symbol(const std::wstring &source);

///  Return if source text contains {{symbol}}
bool contains_encoded_symbol(const std::wstring &source);

/** Return {{x:id}} for [[x]]
 *  @param  token
 *  @param  id
 *  @param  argc  number of tokens in the target
 */
std::wstring create_symbol_target(const std::wstring &token, int id, int argc);

///  Return number of [[x]] in the target
int count_raw_symbols(const std::wstring &target);

///  Replace [[x]] by regular expression for {{x}}
std::wstring encode_symbol(const std::wstring &text);
};

#endif // trsymbol_H
