#ifndef trsymbol_H
#define trsymbol_H

// trsymbol.h
// 9/20/2014 jichi

#include <string>

namespace trsymbol {
bool contains_symbol(const std::wstring &text);
std::wstring render_symbol(const std::wstring &token, int id);
};

#endif // trsymbol_H
