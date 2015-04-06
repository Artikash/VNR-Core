# coding: utf8
# 4/5/2015 jichi
if __name__ == '__main__':
  import sys
  sys.path.append('..')

from functools import partial
from PySide import QtCore
from PySide.QtCore import Qt
from sakurakit.skdebug import dwarn, debugfunc

class Task(QtCore.QObject, QtCore.QRunnable):
  def __init__(self, trigger=None):
    #super(Task, self).__init__(parent)
    QtCore.QObject.__init__(self) # no parent!
    QtCore.QRunnable.__init__(self)
    self.trigger = trigger # function
    self.value = None # return value
    self.completed = False # bool

  finished = QtCore.Signal() # QObject is needed to support Signal

  def run(self):
    """@reimp @public"""
    try: self.value = self.trigger()
    except Exception, e: derror(e)
    self.finished.emit()
    self.completed = True

@debugfunc
def waittasks(tasks, timeout=0, abortSignal=None, autoQuit=False):
  """
  @param  tasks  [Task]
  @param* timeout  int
  @param* abortSignal  Signal or None  signal with auto type
  @param* autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """
  if not tasks:
    return
  loop = QtCore.QEventLoop()
  connectCount = 0
  for task in tasks:
    if not task.completed:
      task.finished.connect(loop.quit, Qt.QueuedConnection)
      connectCount += 1
  if not connectCount:
    return

  aborted = False
  def abort():
    aborted = True
    loop.quit()

  if abortSignal:
    abortSignal.connect(abort)
  # Make sure the eventloop quit before closing
  if autoQuit:
    qApp = QtCore.QCoreApplication.instance()
    qApp.aboutToQuit.connect(abort)
  timer = None
  if timeout:
    timer = QtCore.QTimer()
    timer.setInterval(timeout)
    timer.setSingleShot(True)
    timer.timeout.connect(abort)

  finishCount = 0
  completeCount = 0
  while not aborted and finishCount < connectCount and completeCount < len(tasks):
    loop.exec_()
    finishCount += 1
    completeCount = 0
    for it in tasks:
      if it.completed:
        completeCount += 1

  if timer:
    timer.timeout.disconnect(abort)
  if abortSignal:
    abortSignal.disconnect(abort)
  if autoQuit:
    qApp.aboutToQuit.disconnect(abort)

def _runlater(slot, interval=0):
  """
  @param  slot not None  callback function
  @param  interval  int  time in msecs
  @param  autoQuit  bool  if quit eventloop when qApp.aboutToQuit
  """
  QtCore.QTimer.singleShot(interval, slot)

def runsync(funcs, nthreads=0, abortSignal=None, timeout=0):
  """
  @param  funcs  [function]
  @param* nthreads  maximum number of threads  not used!
  @param* timeout  int
  @param* abortSignal  Signal or None  signal with auto type
  @return  [result of each task function] not None
  """
  tasks = map(Task, funcs)
  if not tasks:
    return []

  def runtasks(tasks):
    for it in tasks:
      QtCore.QThreadPool.globalInstance().start(it)

  _runlater(partial(runtasks, tasks))
  waittasks(tasks, abortSignal=abortSignal, timeout=timeout)
  return [it.value for it in tasks]

if __name__ == '__main__':
  def main():
    task = lambda it: it * it
    tasks = (partial(task, it) for it in [1,2,3,4])
    l = runsync(tasks, nthreads=4)
    print l

  app = QtCore.QCoreApplication(sys.argv)
  _runlater(main)
  app.exec_()

# EOF
