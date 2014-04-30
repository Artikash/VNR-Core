// wintimer.cc
// 6/6/2012 jichi

#include "wintimer/wintimer.h"

//#define DEBUG "wintimer.cc"
#include "sakurakit/skdebug.h"

WINTIMER_BEGIN_NAMESPACE

void WinTimer::singleShot(int msecs, const function_type &f, WId parent)
{
  Self *t = new Self(parent);
  t->setInterval(msecs);
  t->setSingleShot(true);
  t->setFunction([t, &f]() {
    f_();
    delete t_;
  });
  t->start();
}

WINTIMER_END_NAMESPACE

// EOF

/*
// - Single shot -

namespace { // unnamed

class apply_delete
{
  typedef WinTimer::function_type function_type;
  function_type f_;
  WinTimer *t_;
public:
  apply_delete(const function_type &f, WinTimer *t)
    : f_(f), t_(t)  { Q_ASSERT(t); }

  void operator()()
  {
    f_();
    delete t_;
  }
};

} // unnamed namespace
*/
