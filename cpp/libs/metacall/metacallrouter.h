#ifndef METACALLROUTER_H
#define METACALLROUTER_H

// metacallrouter.h
// 2/1/2013 jichi

// Interface
class MetaCallRouter
{
public:
  virtual ~MetaCallRouter() {}

  virtual int convertReceiveMethodId(int value) { return value; }
  virtual int convertSendMethodId(int value) { return value; }
};

#endif // METACALLROUTER_H
