#pragma once
// winmutex.h
// 12/11/2011 jichi

#include <windows.h>

#ifdef _MSC_VER
# pragma warning(disable:4800) // C4800: forcing value to bool
#endif // _MSC_VER

// - win_mutex_locker -

template <typename _Mutex>
  class win_mutex_locker
  {
    typedef win_mutex_locker<_Mutex> _Self;
    win_mutex_locker(const _Self&);
    _Self &operator=(const _Self&);

    _Mutex *_M_mutex;
  public:
    typedef _Mutex mutex_type;
    explicit win_mutex_locker(mutex_type *mutex)
      : _M_mutex(mutex) { mutex->lock(); }
    ~win_mutex_locker() { _M_mutex->unlock(); }
    mutex_type *mutex() const { return _M_mutex; }
  };

// - win_mutex -

template <typename _Mutex, size_t _Irql = 0>
  class win_mutex
  {
    typedef win_mutex<_Mutex> _Self;
    typedef _Mutex __native_type;
    enum { __minimal_irql = _Irql };
    __native_type _M_mutex;

    win_mutex(const _Self&);
    _Self &operator=(const _Self&);
  private:
    win_mutex() { }
    typedef __native_type *native_handle_type;
    native_handle_type native_handle() { return &_M_mutex; }
    static size_t minimal_irql() { return __minimal_irql; }

    void unlock() { }
    void lock() { }
    bool try_lock() { }
  };

template <>
  class win_mutex<CRITICAL_SECTION>
  {
    typedef win_mutex<CRITICAL_SECTION> _Self;
    typedef CRITICAL_SECTION __native_type;
    enum { __minimal_irql = 0 };

    __native_type _M_mutex;

    win_mutex(const _Self&);
    _Self &operator=(const _Self&);
  public:
    typedef __native_type *native_handle_type;
    native_handle_type native_handle() { return &_M_mutex; }
    static size_t minimal_irql() { return __minimal_irql; }

    win_mutex() { ::InitializeCriticalSection(&_M_mutex); }
    ~win_mutex() { ::DeleteCriticalSection(&_M_mutex); }
    void lock() { ::EnterCriticalSection(&_M_mutex); }
    void unlock() { ::LeaveCriticalSection(&_M_mutex); }
    bool try_lock() { return ::TryEnterCriticalSection(&_M_mutex); }
  };

// EOF
