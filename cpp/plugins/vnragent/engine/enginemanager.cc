// enginemanager.cc
// 4/26/2014 jichi

#include "engine/enginemanager.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "model/engine.h"
#include "QxtCore/QxtJSON"
#include "qtjson/qtjson.h"
#include <QtCore/QCoreApplication>
//#include <QtCore/QEventLoop>
#include <QtCore/QHash>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
//#include "debug.h"

#include "wintimer/wintimer.h"

// TODO: Create a wrapper for the event
#include <qt_windows.h>
#define MY_EVENT_NAME L"vnragent_engine"

#define DEBUG "enginemanager"
#include "sakurakit/skdebug.h"

/** Private class */

class EngineManagerPrivate
{
  SK_DECLARE_PUBLIC(EngineManager)
  //QEventLoop *loop;
  //bool blocked;
public:
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
      q_->emit textsReceived(json);
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

  // - Sync -

public:
  void sleep(int interval = 0)
  {
    HANDLE event = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, MY_EVENT_NAME);
    ::WaitForSingleObject(event, INFINITE);
    ::ResetEvent(event);
    ::CloseHandle(event);
  }

  void notify()
  {
    HANDLE event = ::OpenEvent(EVENT_ALL_ACCESS, FALSE, MY_EVENT_NAME);
    ::SetEvent(event);
    ::CloseHandle(event);
  }

  // - Construction -
public:
  enum { RefreshTaskInterval = 200 };

  explicit EngineManagerPrivate(Q *q)
    : q_(q), scenarioTaskDirty(false), nameTaskDirty(false), otherTaskCleanSize(0)
  {
    //refreshTaskTimer = new QTimer(q);
    refreshTaskTimer = new WinTimer;
    refreshTaskTimer->setSingleShot(true);
    refreshTaskTimer->setInterval(RefreshTaskInterval);
    //q->connect(refreshTaskTimer, SIGNAL(timeout()), SLOT(submitTasks()));
    refreshTaskTimer->setMethod(q, &EngineManager::submitTasks);

    HANDLE event = ::CreateEvent(nullptr, TRUE, FALSE, MY_EVENT_NAME);
    //::CloseHandle(event);
  }

public:
  QHash<qint64, QString> trs;   // cached, {key:text}
private:
  //QTimer *refreshTaskTimer;
  WinTimer *refreshTaskTimer;

  bool scenarioTaskDirty,
       nameTaskDirty;
  int otherTaskCleanSize;

  typedef QList<Task> TaskList;
  Task scenarioTask,
       nameTask;
  TaskList otherTasks;
};

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

/** Public class */

// - Construction -

static EngineManager *instance_;
EngineManager *EngineManager::instance() { return instance_; }

EngineManager::EngineManager(QObject *parent)
  : Base(parent), d_(new D(this))
{ ::instance_ = this; }

EngineManager::~EngineManager()
{
  ::instance_ = nullptr;
  delete d_;
}

// - Actions -

void EngineManager::clearTranslation()  { d_->trs.clear(); }

void EngineManager::updateTranslation(const QString &json)
{
  QVariant data = QxtJSON::parse(json);
  if (!data.isNull()) {
    QVariantList l = data.toList();
    if (!l.isEmpty())
      for (auto it = l.constBegin(); it != l.constEnd(); ++it) {
        QVariantMap map = it->toMap();
        if (!map.isEmpty()) {
          qint64 hash = map["hash"].toLongLong();
          int role = map["role"].toInt();
          QString text = map["text"].toString();
          qint64 key = Engine::hashTextKey(hash, role);
          d_->trs[key] = text;
          d_->doTask(text, hash, role);
        }
      }
  }
  d_->notify();
}

//void EngineManager::abortTranslation()
//{ d_->unblock(); }

void EngineManager::addText(const QString &text, qint64 hash, int role, void *context)
{
  D::Task t(text, hash, role, context);
  switch (role) {
  case Engine::ScenarioRole:
    d_->setScenarioTask(t); // TODO: Merge scenario tasks on certain condition, say, texts in multiple lines
    d_->clearOtherTasks();
    break;
  case Engine::NameRole:
    d_->setNameTask(t);
    d_->clearOtherTasks();
    break;
  case Engine::OtherRole:
    d_->addOtherTask(t);
    break;
  default: ;
  }

  d_->touchTasks();
}

void EngineManager::submitTasks() { d_->submitDirtyTasks(); }

QString EngineManager::findTranslation(qint64 hash, int role) const
{
  qint64 key = Engine::hashTextKey(hash, role);
  return d_->trs.value(key);
}

QString EngineManager::waitForTranslation(qint64 hash, int role) const
{
  d_->sleep();
  return findTranslation(hash, role);
}

// EOF
