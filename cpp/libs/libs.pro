# libs.pro
# 7/31/2011 jichi

TEMPLATE = subdirs

win32: SUBDIRS += disasm wintimer

include(cc/cc.pri)
include(disasm/disasm.pri)
include(graphicseffect/graphicseffect.pri)
include(htmlutil/htmlutil.pri)
include(libqxt/libqxt.pri)
include(metacall/metacall.pri)
include(qtinside/qtinside.pri)
include(qtmodule/qtmodule.pri)
include(sakurakit/sakurakit.pri)
include(singleapp/singleapp.pri)
include(texscript/texscript.pri)
win32: include(kstl/kstl.pri)
win32: include(ntdll/ntdll.pri)
win32: include(winddk/winddk.pri)
win32: include(winime/winime.pri)
win32: include(winmaker/winmaker.pri)
win32: include(winmutex/winmutex.pri)
win32: include(winseh/winseh_unsafe.pri)
win32: include(winseh/winseh_safe.pri)
win32: include(wintimer/wintimer.pri)
win32: include(wintts/wintts.pri)
win32: include(winutil/winutil.pri)

# EOF
