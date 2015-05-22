// embedmanager.cc
// 4/26/2014 jichi

#include "config.h"
#include "embed/embedmanager.h"
#include "embed/embedmemory.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "driver/rpcclient.h"
//#include "QxtCore/QxtJSON"
//#include "qtjson/qtjson.h"
#include "winevent/winevent.h"
#include "winmutex/winmutex.h"
#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <qt_windows.h>
//#include "debug.h"

//#define ENGINE_SLEEP_EVENT "vnragent_engine_sleep"
#define D_LOCK win_mutex_lock<D::mutex_type> d_lock(d_->mutex)

// TODO: Use read-write lock instead
//#define D_LOCK (void)0 // locking is temporarily disabled to avoid hanging

#define DEBUG "EmbedManager"
#include "sakurakit/skdebug.h"

/** Private class */

class EmbedManagerPrivate
{
public:
  typedef win_mutex<CRITICAL_SECTION> mutex_type;
  mutex_type mutex; // mutex to lock translations
  QHash<qint64, QString> translations;   // cached, {key:text}

  int waitTime;

  EmbedMemory *memory;

  EmbedManagerPrivate(QObject *parent)
    : waitTime(VNRAGENT_MEMORY_TIMEOUT)
  {
    memory = new EmbedMemory(parent);
    memory->create();
  }

  bool wait() // return if get signaled
  {
    win_event ev(VNRAGENT_MEMORY_EVENT); // create new event on each wait
    return ev.wait(waitTime);
  }

  //static void sleep(int msecs) { ::Sleep(msecs); }
  //void notify() {}
};

//  // - Event -
//  // Sleep 10*100 = 1 second
//  enum { SleepTimeout = 100 };
//  enum { SleepCount = 10 };
//  void sleep(int interval = SleepTimeout, int count = SleepCount)
//  {
//    sleepEvent.signal(false);
//    for (int i = 0; !sleepEvent.wait(interval) && i < count; i++);
//    //sleepEvent.signal(false);
//  }
//
//  void notify() { sleepEvent.signal(true); }

//  mutex_type sleepMutex;
//  typedef win_mutex_cond<CONDITION_VARIABLE> cond_type;
//  cond_type sleepCond;
//  void sleep()
//  {
//    sleepMutex.lock();
//    sleepCond.wait_for(sleepMutex, SleepTimeout * SleepCount);
//  }
//  void notify() { sleepCond.notify_all(); }

/** Public class */

// - Construction -

static EmbedManager *instance_;
EmbedManager *EmbedManager::instance() { return instance_; }

EmbedManager::EmbedManager(QObject *parent)
  : Base(parent), d_(new D(this))
{ ::instance_ = this; }

EmbedManager::~EmbedManager()
{
  ::instance_ = nullptr;
  delete d_;
}

void EmbedManager::setTranslationWaitTime(int msecs)
{ d_->waitTime = msecs; }

// - Actions -

void EmbedManager::quit()
{
  D_LOCK;
  if (d_->memory->isAttached())
    d_->memory->detach();
}

void EmbedManager::clearTranslation()
{
  D_LOCK;
  d_->translations.clear();
}

//void EmbedManager::updateTranslation(const QString &text, qint64 hash, int role)
//{
//  D_LOCK;
//  qint64 key = Engine::hashTextKey(hash, role);
//  d_->translations[key] = text;
//  //d_->notify();
//}

//void EmbedManager::abortTranslation()
//{ d_->unblock(); }

void EmbedManager::sendText(const QString &text, qint64 hash, long signature, int role, bool needsTranslation)
{
#ifdef VNRAGENT_ENABLE_NATIVE_PIPE
  if (needsTranslation)
    RpcClient::instance()->directSendEngineText(text, hash, signature, role, needsTranslation);
  else
#endif // VNRAGENT_ENABLE_NATIVE_PIPE
  emit textReceived(text, hash, signature, role, needsTranslation);
}

QString EmbedManager::findTranslation(qint64 hash, int role) const
{
  D_LOCK;
  qint64 key = Engine::hashTextKey(hash, role);
  return d_->translations.value(key);
}

QString EmbedManager::waitForTranslation(qint64 hash, int role) const
{
  D_LOCK;

  qint64 key = Engine::hashTextKey(hash, role);
  QString ret = d_->translations.value(key);
  if (ret.isEmpty())
    for (int i = 0; i < 2; i++) { // repeat twice
      if (d_->memory->isAttached() && d_->memory->isDataReady() && d_->memory->dataHash() == hash && d_->memory->dataRole() == role) {
        // Lock is not needed since DataReady status has been set
        //d_->memory->lock();
        ret = d_->memory->dataText();
        //d_->memory->unlock();
        if (!ret.isEmpty())
          d_->translations[key] = ret;
        break;
      }
      if (i == 0) // wait only twice
        d_->wait();
    }

  return ret;
}

// EOF
