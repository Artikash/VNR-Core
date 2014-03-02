#pragma once

// ith_p.h
// 10/15/2011 jichi
// Internal header.
// Wrapper of functions from ITH.

#include <QtCore/QString>

struct HookParam; // opaque, declared in ITH/common.h

namespace Ith {

///  Parse hook code, and save the result to hook param if succeeded.
bool parseHookCode(__in const QString &code, __out HookParam *hp);
bool verifyHookCode(__in const QString &code);

} // namespace Ith

// EOF
