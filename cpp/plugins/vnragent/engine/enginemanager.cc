// enginemanager.cc
// 4/26/2014 jichi

#include "engine/enginemanager.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "model/engine.h"
//#include "QxtCore/QxtJSON"
//#include "qtjson/qtjson.h"
#include "winevent/winevent.h"
#include "winmutex/winmutex.h"
#include <QtCore/QHash>
#include <QtCore/QStringList>
//#include "debug.h"

#define ENGINE_EVENT_NAME "vnragent_engine"
#define D_SYNCHRONIZE  win_mutex_locker<D::mutex_type> d_locker(&d_->mutex);

#define DEBUG "enginemanager"
#include "sakurakit/skdebug.h"

/** Private class */

class EngineManagerPrivate
{
  win_event event;
public:
  typedef win_mutex<CRITICAL_SECTION> mutex_type;
  mutex_type mutex;

  QHash<qint64, QString> trs;   // cached, {key:text}

  EngineManagerPrivate()
    : event(ENGINE_EVENT_NAME) {}

  // - Lock -
  void lock() { mutex.lock(); }
  void unlock() { mutex.unlock(); }

  // - Event -

  enum { SleepTimeout = 5000 }; // at most 5 seconds
  void sleep(int interval = SleepTimeout)
  {
    event.wait(interval);
    event.signal(false);
  }

  void notify() { event.signal(true); }
};

/** Public class */

// - Construction -

static EngineManager *instance_;
EngineManager *EngineManager::instance() { return instance_; }

EngineManager::EngineManager(QObject *parent)
  : Base(parent), d_(new D)
{ ::instance_ = this; }

EngineManager::~EngineManager()
{
  ::instance_ = nullptr;
  delete d_;
}

// - Actions -

void EngineManager::clearTranslation()
{
  D_SYNCHRONIZE
  d_->trs.clear();
}

void EngineManager::updateTranslation(const QString &text, qint64 hash, int role)
{
  D_SYNCHRONIZE
  qint64 key = Engine::hashTextKey(hash, role);
  d_->trs[key] = text;
  d_->notify();
}

//void EngineManager::abortTranslation()
//{ d_->unblock(); }

void EngineManager::addText(const QString &text, qint64 hash, int role)
{
  emit textReceived(text, hash, role);
}

QString EngineManager::findTranslation(qint64 hash, int role) const
{
  D_SYNCHRONIZE
  qint64 key = Engine::hashTextKey(hash, role);
  return d_->trs.value(key);
}

QString EngineManager::waitForTranslation(qint64 hash, int role) const
{
  D_SYNCHRONIZE
  qint64 key = Engine::hashTextKey(hash, role);
  auto it = d_->trs.constFind(key);
  if (it == d_->trs.constEnd()) { // FIXME: supposed to be while
  //while (it == d_->trs.constEnd()) {
    d_->unlock();
    d_->sleep();
    d_->lock();
    it = d_->trs.constFind(key);
  }
  return it == d_->trs.constEnd() ? QString() : it.value();
}

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
      foreach (const Task &it, otherTasks)
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

  //EngineManagerPrivate() : blocked(false) {}
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

  //explicit EngineManagerPrivate(QObject *parent)
  //  : loop(new QEventLoop(parent))
  //{ QObject::connect(qApp, SIGNAL(aboutToQuit()), loop, SLOT(quit())); }

  //bool isBlocked() const { return loop->isRunning(); }
  //void block(int interval = 0) { if (!loop->isRunning()) loop->exec(); }
  //void unblock() { if (loop->isRunning()) loop->quit(); }
