# libs.pro
# 7/31/2011 jichi

TEMPLATE = subdirs

SUBDIRS += qtprivate
win32: SUBDIRS += disasm wintimer

include(avrec/avrec.pri)
include(ccutil/ccutil.pri)
include(cppjson/cppjson.pri)
include(cpputil/cpputil.pri)
include(disasm/disasm.pri)
include(dyncodec/dyncodec.pri)
include(hanjaconv/hanjaconv.pri)
include(hanviet/hanviet.pri)
include(hashutil/hashutil.pri)
include(htmldefs/htmldefs.pri)
include(libqxt/libqxt.pri)
include(mono/mono.pri)
include(pinyinconv/pinyinconv.pri)
include(qmlgradient/qmlgradient.pri)
include(qmltext/qmltext.pri)
include(qtdyncodec/qtdyncodec.pri)
include(qteffects/qteffects.pri)
include(qtembedapp/qtembedapp.pri)
include(qtembedplugin/qtembedplugin.pri)
include(qtimage/qtimage.pri)
include(qtjson/qtjson.pri)
include(qtmeta/qtmeta.pri)
include(qtmetacall/qtmetacall.pri)
include(qtrichruby/qtrichruby.pri)
include(qtrubberband/qtrubberband.pri)
include(qtsocketsvc/qtsocketsvc.pri)
include(sakurakit/sakurakit.pri)
include(simplecc/simplecc.pri)
include(tahscript/tahscript.pri)
include(trcodec/trcodec.pri)
include(troscript/troscript.pri)
include(trrender/trrender.pri)
include(trsym/trsym.pri)
include(trscript/trscript.pri)
include(texscript/texscript.pri)
include(unistr/unistr.pri)
include(vnrsharedmemory/vnrsharedmemory.pri)
include(voiceroid/aitalked/aitalked.pri)
win32: include(ceviotts/ceviotts.pri)
win32: include(detoursutil/detoursutil.pri)
win32: include(funhook/funhook.pri)
win32: include(keyboardhook/keyboardhook.pri)
win32: include(kstl/kstl.pri)
win32: include(memdbg/memdbg.pri)
win32: include(mhook/mhook.pri)
win32: include(mhook-disasm/mhook-disasm.pri)
win32: include(modiocr/modiocr.pri)
win32: include(mousehook/mousehook.pri)
win32: include(ntdll/ntdll.pri)
win32: include(ntinspect/ntinspect.pri)
win32: include(qtkbsignal/qtkbsignal.pri)
win32: include(qtmousesel/qtmousesel.pri)
win32: include(winasm/winasm.pri)
win32: include(wincom/wincom.pri)
win32: include(windbg/windbg.pri)
win32: include(winddk/winddk.pri)
win32: include(winevent/winevent.pri)
win32: include(winhook/winhook.pri)
win32: include(winime/winime.pri)
win32: include(winiter/winiter.pri)
win32: include(winkey/winkey.pri)
win32: include(winmaker/winmaker.pri)
win32: include(winmutex/winmutex.pri)
win32: include(winquery/winquery.pri)
win32: include(winseh/winseh_unsafe.pri)
win32: include(winseh/winseh_safe.pri)
win32: include(winsinglemutex/winsinglemutex.pri)
win32: include(winshell/winshell.pri)
win32: include(wintimer/wintimer.pri)
win32: include(wintts/wintts.pri)
win32: include(winversion/winversion.pri)
win32: include(wmpcli/wmpcli.pri)

# EOF
