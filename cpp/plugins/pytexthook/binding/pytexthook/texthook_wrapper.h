#ifndef SBK_TEXTHOOKWRAPPER_H
#define SBK_TEXTHOOKWRAPPER_H

#include <shiboken.h>

#include <texthook.h>

namespace PySide { class DynamicQMetaObject; }

class TextHookWrapper : public TextHook
{
public:
    TextHookWrapper(QObject * parent = nullptr);
    inline void childEvent_protected(QChildEvent * arg__1) { TextHook::childEvent(arg__1); }
    virtual void childEvent(QChildEvent * arg__1);
    inline void connectNotify_protected(const char * signal) { TextHook::connectNotify(signal); }
    virtual void connectNotify(const char * signal);
    inline void customEvent_protected(QEvent * arg__1) { TextHook::customEvent(arg__1); }
    virtual void customEvent(QEvent * arg__1);
    inline void disconnectNotify_protected(const char * signal) { TextHook::disconnectNotify(signal); }
    virtual void disconnectNotify(const char * signal);
    virtual bool event(QEvent * arg__1);
    virtual bool eventFilter(QObject * arg__1, QEvent * arg__2);
    virtual const QMetaObject * metaObject() const;
    inline int receivers_protected(const char * signal) const { return TextHook::receivers(signal); }
    inline QObject * sender_protected() const { return TextHook::sender(); }
    inline int senderSignalIndex_protected() const { return TextHook::senderSignalIndex(); }
    inline void timerEvent_protected(QTimerEvent * arg__1) { TextHook::timerEvent(arg__1); }
    virtual void timerEvent(QTimerEvent * arg__1);
    virtual ~TextHookWrapper();
public:
    virtual int qt_metacall(QMetaObject::Call call, int id, void** args);
    virtual void* qt_metacast(const char* _clname);
    static void pysideInitQtMetaTypes();
};

#endif // SBK_TEXTHOOKWRAPPER_H

