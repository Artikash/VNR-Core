// windowmanager.cc
// 2/1/2013 jichi

#include "window/windowmanager.h"
#include "window/windowhash.h"
#include "util/codepage.h"
#include "qtjson/qtjson.h"
#include "QxtCore/QxtJSON"
#include <QtCore/QTimer>
#include <QtCore/QVariantHash>
#include <qt_windows.h> // for MultiByteToWideChar
//#include <unordered_set>

/** Private class */

namespace { WindowManager::TextEntry NULL_TEXT_ENTRY; }

class WindowManagerPrivate
{
  //SK_DECLARE_PUBLIC(WindowManager)
  typedef WindowManager Q;

  enum { RefreshInterval = 200 };
  QTimer *refreshTextsTimer_; // QTimer is not working
public:
  bool enabled;
  QString encoding;
  bool encodingEnabled;

  const uint systemCodePage;
  uint encodingCodePage;

  bool textsDirty;
  Q::TextEntryList entries;
  QHash<qint64, QString> translations; // {hash : text}

  //std::unordered_set<qint64> h_texts; // hashes
  //std::unordered_set<qint64> h_trs; // hashes

public:
  explicit WindowManagerPrivate(Q *q)
    : enabled(true)
    , encodingEnabled(false)
    , textsDirty(false)
    , systemCodePage(::GetACP())
    , encodingCodePage(0)
  {
    refreshTextsTimer_ = new QTimer(q);
    refreshTextsTimer_->setSingleShot(true);
    refreshTextsTimer_->setInterval(RefreshInterval);
    q->connect(refreshTextsTimer_, SIGNAL(timeout()), SLOT(sendDirtyTexts()));
  }

  bool isTranscodingNeeded() const
  { return encodingEnabled && encodingCodePage != systemCodePage; }

  void touchTexts()
  {
    textsDirty = true;
    if (!enabled)
      return;
    //if (!refreshTextsTimer_->isActive())
    refreshTextsTimer_->start();
  }

  void clearTranslation()
  {
    translations.clear();
    touchTexts();
  }

  void invalidateTexts()
  {
    if (entries.isEmpty())
      return;
    //foreach (auto &e, entries)
    //  e.text = decodeText(e.data);
    for (auto p = entries.begin(); p != entries.end(); ++p)
      p->text = decodeText(p->data);
    clearTranslation();
  }

  QString decodeText(const QByteArray &data) const;
};

// See: http://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
QString WindowManagerPrivate::decodeText(const QByteArray &data) const
{
  const wchar_t *ws = (LPCWSTR)data.constData();
  const int wsSize = data.size() * 2;

  if (!isTranscodingNeeded())
    return QString::fromWCharArray(ws, wsSize);

  int bufferSizeNeeded = ::WideCharToMultiByte(systemCodePage, 0, ws, wsSize, nullptr, 0, nullptr, nullptr);
  if (bufferSizeNeeded <= 0)
    return QString();

  std::string convertBuffer(bufferSizeNeeded, 0);
  bufferSizeNeeded = ::WideCharToMultiByte(systemCodePage, 0, ws, wsSize, &convertBuffer[0], bufferSizeNeeded, nullptr, nullptr);

  int returnSize = ::MultiByteToWideChar(encodingCodePage, 0, &convertBuffer[0], bufferSizeNeeded, nullptr, 0);
  if (returnSize <= 0)
    return QString();

  std::wstring returnBuffer(returnSize, 0);
  returnSize = ::MultiByteToWideChar(encodingCodePage, 0, &convertBuffer[0], bufferSizeNeeded, &returnBuffer[0], returnSize);
  return QString::fromStdWString(returnBuffer);
}

/** Public class */

// - Construction -

WindowManager::WindowManager(QObject *parent) : Base(parent), d_(new D(this)) {}

WindowManager::~WindowManager() { delete d_; }

// - Properties -

void WindowManager::setEnabled(bool t)
{
  if (d_->enabled != t) {
    d_->enabled = t;
    if (t)
      sendDirtyTexts();
  }
}

void WindowManager::setEncoding(const QString &v)
{
  if (d_->encoding != v) {
    d_->encoding = v;
    d_->encodingCodePage = Util::codePageForEncoding(d_->encoding);
    d_->invalidateTexts();
  }
}

void WindowManager::setEncodingEnabled(bool t)
{
  if (d_->encodingEnabled != t) {
    d_->encodingEnabled = t;
    d_->invalidateTexts();
  }
}

// - Queries -

QString WindowManager::decodeText(const QByteArray &data) const
{ return d_->decodeText(data); }

const WindowManager::TextEntry &WindowManager::findEntryWithAnchor(ulong anchor) const
{
  foreach (const auto &e, d_->entries)
    if (e.anchor == anchor)
      return e;
  return NULL_TEXT_ENTRY;
}

QString WindowManager::findTranslationWithHash(qint64 hash) const
{ return d_->translations.value(hash); }

// - Update -

void WindowManager::clearTranslation()
{ d_->clearTranslation(); }

void WindowManager::addEntry(const QByteArray &data, const QString &text, qint64 hash, ulong anchor)
{
  d_->entries.append(TextEntry(data, text, hash, anchor));
  d_->touchTexts();
}

void WindowManager::sendDirtyTexts()
{
  if (!d_->enabled || !d_->textsDirty)
    return;
  d_->textsDirty = false;

  if (d_->entries.isEmpty())
    return;

  QVariantHash texts;
  foreach (const auto &e, d_->entries)
    if (!d_->translations.contains(e.hash))
      texts[QString::number(e.hash)] = e.text;

  if (!texts.isEmpty()) {
    QString json = QtJson::stringify(texts);
    emit textDataChanged(json);
  }
}

void WindowManager::updateTranslationData(const QString &json)
{
  QVariant data = QxtJSON::parse(json);
  if (data.isNull())
    return;
  QVariantMap map = data.toMap();
  if (map.isEmpty())
    return;

  for (auto it = map.constBegin(); it != map.constEnd(); ++it)
    if (qint64 hash = it.key().toLongLong())
      d_->translations[hash] = it.value().toString();
      //QString t = d_->trs[hash] = it.value().toString();
      //d_->h_trs.insert(Window::hashString(t));
}

// EOF

//bool WindowManager::containsWindow(WId window, Window::TextRole role) const
//{
  //QMutexLocker locker(&d_->mutex);
//  foreach (const auto &e, d_->entries)
//    if (e.window == window && (!role || role == e.role))
//      return true;
//  return false;
//}

//bool WindowManager::containsAnchor(ulong anchor) const
//{
//  foreach (const auto &e, d_->entries)
//    if (e.anchor == anchor)
//      return true;
//  return false;
//}

//bool WindowManager::containsText(qint64 hash) const
//{ return d_->h_texts.find(hash) != d_->h_texts.end(); }

//bool WindowManager::containsTranslation(qint64 hash) const
//{ return d_->h_trs.find(hash) != d_->h_trs.end(); }

//const WindowManager::TextEntry *WindowManager::findEntryWithHash(qint64 hash) const
//{
//  foreach (const auto &e, d_->entries)
//    if (e.hash == hash)
//      return &e;
//  return nullptr;
//}

//{
//  QString ret = d_->trs.value(hash);
//  if (!ret.isEmpty())
//    ret = findTextEntry(hash).text + " | " + ret;
//  return ret;
//}

//void WindowManager::updateTextTranslation(const QString &tr, qint64 hash, qint64 trhash)
//{
//  Q_ASSERT(hash);
//  d_->trs[hash] = tr;
//  d_->h_trs.insert(trhash ? trhash : Window::hashString(tr));
//}

//void WindowManager::updateText(const QString &text, qint64 hash, ulong anchor)
//{
//  Q_ASSERT(anchor);
//  if (!hash)
//    hash = Window::hashString(text);
//  foreach (const auto &e, d_->entries)
//    if (e.hash == hash && e.anchor == anchor)
//      return;
//
//  d_->entries.append(TextEntry(text, hash, anchor));
//
//  if (d_->h_texts.find(hash) == d_->h_texts.end()) {
//    d_->h_texts.insert(hash);
//    d_->touchTexts();
//  }
//}
