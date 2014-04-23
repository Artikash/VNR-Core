# libs.pro
# 7/31/2011 jichi

TEMPLATE = subdirs

SUBDIRS += qtprivate
win32: SUBDIRS += disasm wintimer

include(cc/cc.pri)
include(disasm/disasm.pri)
include(htmldefs/htmldefs.pri)
include(libqxt/libqxt.pri)
include(qmltext/qmltext.pri)
include(qteffects/qteffects.pri)
include(qtembedded/qtembedded.pri)
include(qtimage/qtimage.pri)
include(qtjson/qtjson.pri)
include(qtmetacall/qtmetacall.pri)
include(sakurakit/sakurakit.pri)
include(singleapp/singleapp.pri)
include(texscript/texscript.pri)
win32: include(detoursutil/detoursutil.pri)
win32: include(kstl/kstl.pri)
win32: include(memdbg/memdbg.pri)
win32: include(ntdll/ntdll.pri)
win32: include(ntinspect/ntinspect.pri)
win32: include(windbg/windbg.pri)
win32: include(winddk/winddk.pri)
win32: include(winhook/winhook.pri)
win32: include(winime/winime.pri)
win32: include(winmaker/winmaker.pri)
win32: include(winmutex/winmutex.pri)
win32: include(winquery/winquery.pri)
win32: include(winseh/winseh_unsafe.pri)
win32: include(winseh/winseh_safe.pri)
win32: include(winshell/winshell.pri)
win32: include(wintimer/wintimer.pri)
win32: include(wintts/wintts.pri)

# EOF
