#pragma once

// engine.h
// 4/20/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QStringList>

class EngineSettings;
class AbstractEnginePrivate;
class AbstractEngine
{
  //Q_OBJECT
  SK_CLASS(AbstractEngine)
  SK_DECLARE_PRIVATE(AbstractEnginePrivate)
  SK_DISABLE_COPY(AbstractEngine)

public:
  static Self *instance(); // Needed to be explicitly deleted on exit

  AbstractEngine();
  virtual ~AbstractEngine();

  EngineSettings *settings() const;

  const char *name() const;

  const char *encoding() const;
  void setEncoding(const QString &v);

  bool isTranscodingNeeded() const;

  bool load();
  bool unload();

  //static bool isEnabled();
  //static void setEnabled(bool t);

protected:
  void setName(const char *v);
  void setWideChar(bool t);
  uint codePage() const;
  void setCodePage(uint cp);

  virtual bool attach() = 0; ///< Invoked by load
  virtual bool detach() { return false; } ///< Invoked by unload

  // Hook

  ///  Replace the address with the dispatch function, return whether succeed
  bool hookAddress(ulong addr);

  struct HookStack
  {
    ulong eflags;  // pushaf
    ulong edi,     // pushad
          esi,
          ebp,
          esp,
          ebx,
          edx,
          ecx,     // this
          eax;     // 0x24
    ulong retaddr; // 0x28
    ulong args[1]; // 0x2c
  };
  ///  The callback of the dispatch address
  typedef void (* hook_function)(HookStack *);
  void setHookFunction(hook_function v);

  // Filter functions

  typedef QString (* filter_function)(const QString &text, int role);
  void setTextFilter(filter_function v);
  void setTranslationFilter(filter_function v);

  // Hook

  typedef void *address_type;
  typedef const void *const_address_type;

  ///  Replace the instruction at the old_addr with jmp to new_addr. Return the address to the replaced code.
  static address_type replaceFunction(address_type old_addr, const_address_type new_addr);
  //static address_type restoreFunction(address_type restore_addr, const_address_type old_addr);

  // Ignore type checking
  template<typename Ret, typename Arg1, typename Arg2>
  static inline Ret replaceFunction(Arg1 arg1, Arg2 arg2)
  { return (Ret)replaceFunction((address_type)arg1, (const_address_type)arg2); }

  // Not used
  //template<typename Ret, typename Arg1, typename Arg2>
  //static inline Ret restoreFunction(Arg1 arg1, Arg2 arg2)
  //{ return (Ret)restoreFunction((address_type)arg1, (const_address_type)arg2); }

  ///  Return whether all relpaths exists
  static bool matchFiles(const QStringList &relpaths);

  // Interface to descendent classes
public:
  ///  Send LPCSTR text to VNR
  QByteArray dispatchTextA(const QByteArray &data, long signature, int role = 0);

  ///  Send LPCWSTR text to VNR
  QString dispatchTextW(const QString &text, long signature, int role = 0);

  // This function is not thread-safe
  //const char *exchangeTextA(const char *data, long signature, int role = 0);
};

// EOF

//signals:
//  // context is opaque, and the receiver of this signal is responsible to release the context
//  void textReceived(const QString &text, qint64 hash, int role, void *context);
//public:
//  virtual void drawText(const QString &text, const void *context) = 0;
//  virtual void releaseContext(void *context) = 0;
