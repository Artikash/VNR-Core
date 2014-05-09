// embedmanager.cc
// 4/26/2014 jichi

#include "embed/embedmanager.h"
#include "embed/embedmemory.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
//#include "QxtCore/QxtJSON"
//#include "qtjson/qtjson.h"
#include "winevent/winevent.h"
#include "winmutex/winmutex.h"
#include <QtCore/QHash>
#include <QtCore/QStringList>
//#include "debug.h"

//#define ENGINE_SLEEP_EVENT "vnragent_engine_sleep"
#define D_LOCK win_mutex_lock<D::mutex_type> d_lock(d_->mutex)

#define DEBUG "EmbedManager"
#include "sakurakit/skdebug.h"

/** Private class */

class EmbedManagerPrivate
{
public:
  typedef win_mutex<CRITICAL_SECTION> mutex_type;
  mutex_type mutex;

  QHash<qint64, QString> translations;   // cached, {key:text}

  EmbedMemory *memory;

  EmbedManagerPrivate(QObject *parent)
  {
    memory = new EmbedMemory(parent);
    memory->create();
  }

  static void sleep(int msecs) { ::Sleep(msecs); }
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

void EmbedManager::sendText(const QString &text, qint64 hash, int role, bool needsTranslation)
{
  if (needsTranslation)
    emit textReceived(text, hash, role, needsTranslation);
  else
    emit textReceivedDelayed(text, hash, role, needsTranslation);
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

  enum { SleepInterval = 10, SleepCount = 100 }; // sleep for at most 1 second

  qint64 key = Engine::hashTextKey(hash, role);
  QString ret = d_->translations.value(key);
  if (ret.isEmpty())
    for (int i = 0; i < SleepCount; i++) {
      if (!d_->memory->isAttached() || d_->memory->isDataCanceled())
        break;
      if (d_->memory->isDataReady() && d_->memory->dataHash() == hash && d_->memory->dataRole() == role) {
        d_->memory->lock();
        ret = d_->memory->dataText();
        d_->memory->unlock();
        if (!ret.isEmpty())
          d_->translations[key] = ret;
        break;
      }
      D::sleep(SleepInterval);
    }
  return ret;
}

  //D_LOCK;
  //qint64 key = Engine::hashTextKey(hash, role);
  //auto it = d_->translations.constFind(key);
  //if (it == d_->translations.constEnd()) { // FIXME: supposed to be while
  //while (it == d_->translations.constEnd()) {
  //  d_->unlock();
  //  d_->sleep();
  //  d_->lock();
  //  it = d_->translations.constFind(key);
  //}
  //return it == d_->translations.constEnd() ? QString() : it.value();

// EOF

/*
  // - Tasks -

  struct Task
  {
    QString text;
    qint64 hash;
    int role;
    void *context;

    Task() : hash(0), role(0), context(nullptr) {}
    Task(const QString &text, qint64 hash, int role, void *context)
      : text(text), hash(hash), role(role), context(context) {}

    QVariantHash toVariant() const
    {
      QVariantHash ret;
      ret["text"] = text;
      ret["hash"] = hash;
      ret["role"] = role;
      return ret;
    }

    bool isEmpty() const { return !role; }
    void release() const
    {
      if (context)
        AbstractEngine::instance()->releaseContext(context);
        //context = nullptr;
    }
  };

  void setNameTask(const Task &t)
  {
    nameTask.release();
    nameTask = t;
    nameTaskDirty = true;
  }

  void setScenarioTask(const Task &t)
  {
    scenarioTask.release();
    scenarioTask = t;
    scenarioTaskDirty = true;
  }

  void addOtherTask(const Task &t)
  {
    otherTasks.append(t);
  }

  void clearOtherTasks()
  {
    if (!otherTasks.isEmpty()) {
      foreach (const auto &it, otherTasks)
        it.release();
      otherTasks.clear();
    }
    otherTaskCleanSize = 0;
  }

  void touchTasks()
  {
    //refreshTaskTimer->start();
    submitDirtyTasks();
  }

  void submitDirtyTasks()
  {
    QVariantList l;
    if (scenarioTaskDirty)
      l.append(scenarioTask.toVariant());
    if (nameTaskDirty)
      l.append(nameTask.toVariant());
    if (otherTaskCleanSize < otherTasks.size())
      for (auto p = otherTasks.constBegin() + qMax(0, otherTaskCleanSize); p != otherTasks.constEnd(); ++p)
        l.append(p->toVariant());
    if (!l.isEmpty()) {
      QString json = QtJson::stringify(l);
      q_->emit textReceived(json);
    }
  }

  void doTask(const QString &text, qint64 hash, int role)
  {
    Task t;
    switch (role) {
    case Engine::ScenarioRole:
      if (hash == scenarioTask.hash) {
        t = scenarioTask;
        scenarioTaskDirty = false;
      } break;
    case Engine::NameRole:
      if (hash == nameTask.hash) {
        t = nameTask;
        nameTaskDirty = false;
      } break;
    case Engine::OtherRole:
      if (!otherTasks.isEmpty())  {
        auto it = otherTasks.begin();
        int index = 0;
        while (it != otherTasks.end()) {
          if (it->hash == hash) {
            t = *it;
            it = otherTasks.erase(it);
            if (index < otherTaskCleanSize)
              otherTaskCleanSize--;
          } else
            ++it;
          index++;
        }
      }
    }
    if (!t.isEmpty()) {
      AbstractEngine::instance()->drawText(text.isEmpty() ? t.text : text, t.context);
      t.release();
    }
  }
*/


  //enum { TranslationTimeout = 5000 }; // wait for at most 5 seconds
  //enum { TranslationTimeout = 50 }; // wait for at most 5 seconds

  //EmbedManagerPrivate() : blocked(false) {}
  //bool isBlocked() const { return blocked; }
  //void unblock() { blocked = false; }
  // TODO: Add a timer to quit loop
  //void block(int interval = 0) // TODO: get system time to prevent timeout
  //{
  //  if (!blocked) {
  //    blocked = true;
  //    while (blocked) // FIXME: Avoid using spin lock
  //      qApp->processEvents(QEventLoop::AllEvents, interval);
  //  }
  //}

  //explicit EmbedManagerPrivate(QObject *parent)
  //  : loop(new QEventLoop(parent))
  //{ QObject::connect(qApp, SIGNAL(aboutToQuit()), loop, SLOT(quit())); }

  //bool isBlocked() const { return loop->isRunning(); }
  //void block(int interval = 0) { if (!loop->isRunning()) loop->exec(); }
  //void unblock() { if (loop->isRunning()) loop->quit(); }
