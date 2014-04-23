#ifndef QTJSON_H
#define QTJSON_H

// qtjson.h
// 2/1/2013 jichi

#include <QtCore/QString>

QT_FORWARD_DECLARE_CLASS(QVariant)

namespace QtJson {

QString stringify(const QVariant &v);

} // namespace QtJson

#endif // QTJSON
