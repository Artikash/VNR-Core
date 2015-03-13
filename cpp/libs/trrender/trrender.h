#ifndef TRRENDER_H
#define TRRENDER_H

// trrender.h
// 9/20/2014 jichi
#include <string>

// Render a translation rule into a json href link.
std::wstring tr_render_rule(const std::wstring &target,
                            const std::wstring &source = std::wstring(),
                            int id = 0,
                            bool regex = false);

#endif // TRRENDER_H
