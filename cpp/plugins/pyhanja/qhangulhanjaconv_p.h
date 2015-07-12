#ifndef QHANGULHANJACONV_P_H
#define QHANGULHANJACONV_P_H

// hangulparse.h
// 1/6/2015 jichi

#include <functional>
#include <string>

void hangul_iter_parse(const std::wstring &text, const std::function<void (size_t start, size_t length)> &fun);
//void hangul_iter_parse(const QString &text, const std::function<void (size_t start, size_t length)> &fun);

#endif // QHANGULHANJACONV_P_H
