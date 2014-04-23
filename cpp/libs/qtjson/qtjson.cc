// qtjson.cc
// 4/21/2014 jichi
//
// See:
// http://dev.libqxt.org/libqxt/src
// https://github.com/da4c30ff/qt-json/blob/master/json.cpp

#include "qtjson/qtjson.h"

#include <QtCore/QStringList>
#include <QtCore/QVariant>

// Helpers
namespace { // unamed

  ///  Return escaped special string if succeed
  const char *escapeSpecialChar(ushort ch)
  {
    switch (ch) {
    case '\\': return "\\\\";
    case '"': return "\\\"";
    case '\n': return "\\n";
    case '\r': return "\\r";
    case '\t': return "\\t";
    case '\b': return "\\b";
    case '\f': return "\\f";
    default: return nullptr;
    }
  }

  ///  Return \uXXXX
  QString escapeUnicode(ushort ch)
  {
    QString r = QString::number(ch, 16);
    while (r.size() < 4)
      r.prepend('0');
    return "\\u" + r;
  }

} // namespace unamed

// Stringify

QString QtJson::stringify(const QVariant &v)
{
  if (v.isNull())
    return "null";
  switch (v.type()) {
    case QVariant::Bool: return v.toBool() ? "true" : "false";

    case QVariant::ULongLong: return QString::number(v.toULongLong());
    case QVariant::UInt: return QString::number(v.toUInt());

    case QVariant::LongLong: return QString::number(v.toLongLong());
    case QVariant::Int: return QString::number(v.toInt());

    case QVariant::Double: return QString::number(v.toDouble());

    case QVariant::Map:
      {
        QString r = "{";
        QMap<QString, QVariant> map = v.toMap();
        QMapIterator<QString, QVariant> i(map);
        while (i.hasNext()) {
          i.next();
          r += "\"" + i.key() + "\":" + stringify(i.value()) + "," ;
        }
        if (r.length() > 1)
          r.chop(1); // chop the last comma
        r.append('}');
        return r;
      }

#if QT_VERSION >= 0x040500
    case QVariant::Hash:
      {
        QString r="{";
        QHash<QString, QVariant> map = v.toHash();
        QHashIterator<QString, QVariant> i(map);
        while (i.hasNext()) {
          i.next();
          r += "\"" + i.key() + "\":" + stringify(i.value()) + ",";
        }
        if (r.length()>1)
          r.chop(1); // chop the last comma
        r.append('}');
        return r;
      }
#endif

    case QVariant::StringList:
      {
        QString r = "[";
        QStringList l = v.toStringList();
        foreach (const QString &it, l)
          r += "\"" + it + "\",";
        if (r.length() > 1)
          r.chop(1); // chop the trailing comma
        r.append(']');
        return r;
      }

    case QVariant::List:
      {
        QString r = "[";
        QVariantList l = v.toList();
        foreach (const QVariant &it, l)
          r += stringify(it) + ",";
        if (r.length() > 1)
          r.chop(1);
        r.append(']');
        return r;
      }

    //case QVariant::Char:
    //case QVariant::ByteArray:
    //case QVariant::Date:
    //case QVariant::DateTime:
    //case QVariant::Time:
    case QVariant::String:
    default:
      {
        QString in = v.toString();
        QString out;
        for (QString::ConstIterator p = in.constBegin(); p != in.constEnd(); ++p) {
          ushort w = p->unicode();
          if (!w) // w == 0, which should never happen
            continue;
          else if (const char *s = escapeSpecialChar(w))
            out.append(s);
          else if (w > 127 || w < 32) // non-ascii or non-printable
            out += escapeUnicode(w);
          else
            out.append(*p);
        }
        //return "\"" + out + "\"";
        out.append('"')
           .prepend('"');
        return out;
      }
  }
  return QString();
}

// EOF
