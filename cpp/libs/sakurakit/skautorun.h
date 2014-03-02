#ifndef SKAUTORUN_H
#define SKAUTORUN_H

// skautorun.h
// 9/30/2012 jichi

#include "sakurakit/skglobal.h"
#include <boost/function.hpp>

SK_BEGIN_NAMESPACE

class SkAutoRun
{
public:
  typedef boost::function<void ()> function_t;
  SkAutoRun(const function_t &start, const function_t &exit)
    : exit_(exit) { start(); }
  ~SkAutoRun() { exit_(); }
private:
  function_t exit_;
};

class SkAutoRunAtStartup
{
public:
  typedef SkAutoRun::function_t function_t;
  explicit SkAutoRunAtStartup(const function_t &start) { start(); }
};

class SkAutoRunAtExit
{
public:
  typedef SkAutoRun::function_t function_t;
  explicit SkAutoRunAtExit(const function_t &exit) : exit_(exit) {}
  ~SkAutoRunAtExit() { exit_(); }
private:
  function_t exit_;
};

SK_END_NAMESPACE

#endif // SkAUTORUN_H
