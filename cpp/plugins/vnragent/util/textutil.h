#pragma once

// textutil.h
// 5/7/2014 jichi

#include <QtCore/QtGlobal>

QT_FORWARD_DECLARE_CLASS(QString)

namespace Util {

///  Assume the text is Japanese, and return if it is not understandable for non-Japanese speakers.
bool needsTranslation(const QString &text);

} // namespace Util

// EOF
