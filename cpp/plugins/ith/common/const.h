#pragma once

// ith/common/const.h
// 8/23/2013 jichi
// Branch: ITH/common.h, rev 128

// jichi 9/9/2013: Another importnat function is lstrcatA, which is already handled by
// Debonosu hooks. Wait until it is really needed by certain games.
// The order of the functions is used in several place.
// I need to recompile all of the dlls to modify the order.
enum HookFunType {
  HF_Null = -1
  , HF_GetTextExtentPoint32A
  , HF_GetGlyphOutlineA
  , HF_ExtTextOutA
  , HF_TextOutA
  , HF_GetCharABCWidthsA
  , HF_DrawTextA
  , HF_DrawTextExA
  //, HF_lstrlenA
  , HF_GetTextExtentPoint32W
  , HF_GetGlyphOutlineW
  , HF_ExtTextOutW
  , HF_TextOutW
  , HF_GetCharABCWidthsW
  , HF_DrawTextW
  , HF_DrawTextExW
  //, HF_lstrlenW
  , HookFunCount // 14
};

enum { HOOK_FUN_COUNT = HookFunCount };
enum { MAX_HOOK = 32 }; // must be larger than HookFunCount

// jichi 375/2014: Add offset of pusha/pushad
// http://faydoc.tripod.com/cpu/pushad.htm
// http://agth.wikia.com/wiki/Cheat_Engine_AGTH_Tutorial
enum pusha_off{
  pusha_eax_off = -0x4
  , pusha_ecx_off = -0x8
  , pusha_edx_off = -0xc
  , pusha_ebx_off = -0x10
  , pusha_esp_off = -0x14
  , pusha_ebp_off = -0x18
  , pusha_esi_off = -0x1c
  , pusha_edi_off = -0x20
};

#define HOOK_FUN_NAME_LIST \
  L"GetTextExtentPoint32A" \
  , L"GetGlyphOutlineA" \
  , L"ExtTextOutA" \
  , L"TextOutA" \
  , L"GetCharABCWidthsA" \
  , L"DrawTextA" \
  , L"DrawTextExA" \
  , L"GetTextExtentPoint32W" \
  , L"GetGlyphOutlineW" \
  , L"ExtTextOutW" \
  , L"TextOutW" \
  , L"GetCharABCWidthsW" \
  , L"DrawTextW" \
  , L"DrawTextExW"
  //, L"lstrlenA"
  //, L"lstrlenW"

enum IhfCommandType {
  IHF_COMMAND = -1 // null type
  , IHF_COMMAND_NEW_HOOK = 0
  , IHF_COMMAND_REMOVE_HOOK = 1
  , IHF_COMMAND_MODIFY_HOOK = 2
  , IHF_COMMAND_DETACH = 3
};

enum IhfNotificationType {
  IHF_NOTIFICATION = -1 // null type
  , IHF_NOTIFICATION_TEXT = 0
  , IHF_NOTIFICATION_NEWHOOK = 1
};

// jichi 9/8/2013: The meaning are gussed
enum HookParamType : unsigned long {
  HP_Null             = 0       // never used
  , USING_STRING      = 0x1     // type(data) is char* or wchar_t* and has length
  , USING_UNICODE     = 0x2     // type(data) is wchar_t or wchar_t*
  , BIG_ENDIAN        = 0x4     // type(data) is char
  , DATA_INDIRECT     = 0x8
  , USING_SPLIT       = 0x10    // aware of split time?
  , SPLIT_INDIRECT    = 0x20
  , MODULE_OFFSET     = 0x40    // do hash module, and the address is relative to module
  , FUNCTION_OFFSET   = 0x80    // do hash function, and the address is relative to funccion
  , PRINT_DWORD       = 0x100
  , STRING_LAST_CHAR  = 0x200
  , NO_CONTEXT        = 0x400
  , EXTERN_HOOK       = 0x800   // use external hook function
  , HOOK_AUXILIARY    = 0x2000  // jichi 12/13/2013: None of known hooks are auxiliary
  , HOOK_ENGINE       = 0x4000
  , HOOK_ADDITIONAL   = 0x8000
};

// jichi 12/18/2013:
// These dlls are used to guess the range for non-NO_CONTEXT hooks.
//
// Disabling uxtheme.dll would crash certain system: http://tieba.baidu.com/p/2764436254
#define IHF_FILTER_DLL_LIST \
  /* ITH original filters */ \
  L"gdiplus.dll"    /* Graphics functions like TextOutA */ \
  , L"lpk.dll"      /* Language package scripts and fonts */ \
  , L"msctf.dll"    /* Text service */ \
  , L"psapi.dll"    /* Processes */ \
  , L"usp10.dll"    /* UNICODE rendering */ \
  , L"user32.dll"   /* Non-graphics functions like lstrlenA */ \
  , L"uxtheme.dll"  /* Theme */ \
  \
  /* Windows DLLs */ \
  , L"advapi32.dll" /* Advanced services */ \
  , L"apphelp.dll"  /* Appliation help */ \
  , L"audioses.dll" /* Audios */  \
  , L"avrt.dll"     /* Audio video runtime */ \
  , L"cfgmgr32.dll" /* Configuration manager */ \
  , L"clbcatq.dll"  /* COM query service */ \
  , L"comctl32.dll" /* Common control library */ \
  , L"comdlg32.dll" /* Common dialogs */ \
  , L"crypt32.dll"  /* Security cryption */ \
  , L"cryptbase.dll"/* Security cryption */ \
  , L"cryptsp.dll"  /* Security cryption */ \
  , L"d3d8thk.dll"  /* Direct3D 8 */ \
  , L"d3d9.dll"     /* Direct3D 9 */ \
  , L"dbghelp.dll"  /* Debug help */ \
  , L"dciman32.dll" /* Display cotrol */ \
  , L"devobj.dll"   /* Device object */ \
  , L"ddraw.dll"    /* Direct draw */ \
  , L"dinput.dll"   /* Diret input */ \
  , L"dsound.dll"   /* Direct sound */ \
  , L"DShowRdpFilter.dll" /* Direct show */ \
  , L"dwmapi.dll"   /* Windows manager */ \
  , L"gdi32.dll"    /* GDI32 */ \
  , L"hid.dll"      /* HID user library */ \
  , L"iertutil.dll" /* IE runtime */ \
  , L"imagehlp.dll" /* Image help */ \
  , L"imm32.dll"    /* Input method */ \
  , L"ksuser.dll"   /* Kernel service */ \
  , L"ole32.dll"    /* COM OLE */ \
  , L"oleacc.dll"   /* OLE access */ \
  , L"oleaut32.dll" /* COM OLE */ \
  , L"kernel.dll"   /* Kernel functions */ \
  , L"kernelbase.dll" /* Kernel functions */ \
  , L"midimap.dll"  /* MIDI */ \
  , L"mmdevapi.dll" /* Audio device */ \
  , L"mpr.dll"      /* Winnet */ \
  , L"msacm32.dll"  /* MS ACM */ \
  , L"msacm32.drv"  /* MS ACM */ \
  , L"msasn1.dll"   /* Encoding/decoding */ \
  , L"msimg32.dll"  /* Image */ \
  , L"msvfw32.dll"  /* Media play */ \
  , L"netapi32.dll" /* Network service */ \
  , L"normaliz.dll" /* Normalize */ \
  , L"nsi.dll"      /* NSI */ \
  , L"ntdll.dll"    /* NT functions */ \
  , L"ntmarta.dll"  /* NT MARTA */ \
  , L"nvd3dum.dll"  /* Direct 3D */ \
  , L"powerprof.dll"/* Power profile */ \
  , L"profapi.dll"  /* Profile API */ \
  , L"propsys.dll"  /* System properties */ \
  , L"quartz.dll"   /* OpenGL */ \
  , L"rpcrt4.dll"   /* RPC runtime */ \
  , L"rpcrtremote.dll" /* RPC runtime */ \
  , L"rsabase.dll"  /* RSA cryption */ \
  , L"rsaenh.dll"   /* RSA cryption */ \
  , L"schannel.dll" /* Security channel */ \
  , L"sechost.dll"  /* Service host */ \
  , L"setupapi.dll" /* Setup service */ \
  , L"shell32.dll"  /* Windows shell */ \
  , L"shlwapi.dll"  /* Light-weighted shell */ \
  , L"slc.dll"      /* SLC */ \
  , L"srvcli.dll"   /* Service client */ \
  , L"version.dll"  /* Windows version */ \
  , L"wdmaud.drv"   /* Wave output */ \
  , L"wldap32.dll"  /* Wireless */ \
  , L"wininet.dll"  /* Internet access */ \
  , L"winmm.dll"    /* Windows sound */ \
  , L"winsta.dll"   /* Connection system */ \
  , L"wtsapi32.dll" /* Windows terminal server */ \
  , L"wintrust.dll" /* Windows trust */ \
  , L"wsock32.dll"  /* Windows sock */ \
  , L"ws2_32.dll"   /* Terminal server */ \
  , L"wkscli.dll"   /* ACIS */ \
  \
  /* MSVCRT */ \
  , L"msvcrt.dll"   /* VC rutime */ \
  , L"msvcr80.dll"  /* VC rutime 8 */ \
  , L"msvcp80.dll"  /* VC rutime 8 */ \
  , L"msvcr90.dll"  /* VC rutime 9 */ \
  , L"msvcp90.dll"  /* VC rutime 9 */ \
  , L"msvcr100.dll" /* VC rutime 10 */ \
  , L"msvcp100.dll" /* VC rutime 10 */ \
  , L"msvcr110.dll" /* VC rutime 11 */ \
  , L"msvcp110.dll" /* VC rutime 11 */ \
  \
  /* VNR */ \
  , L"vnrcli.dll" \
  , L"vnrclixp.dll" \
  , L"vnreng.dll" \
  , L"vnrengxp.dll" \
  \
  /* Sogou IME */ \
  , L"sogoupy.ime" \
  , L"PicFace.dll" \
  , L"AddressSearch.dll" \
  \
  /* QQ IME */ \
  , L"QQPINYIN.IME" \
  \
  /* AlphaROM */ \
  , L"kDays.dll" \
  \
  /* 360Safe */ \
  , L"safemon.dll" \
  \
  /* Locale changers */ \
  , L"AlLayer.dll"  /* AppLocale */ \
  , L"LocaleEmulator.dll" /* Locale Emulator */ \
  , L"LSH.dll"      /* LocaleSwitch */ \
  , L"ntleah.dll"   /* NTLEA */

  // Google Japanese IME
  //, L"GoogleIMEJaTIP32.dll"

enum {
  //IHF_FILTER_COUNT = 7
  IHF_FILTER_COUNT = 7 + 72 + 9 + 4 + 3 + 1 + 1 + 1 + 4 // count of total dlls to filter
  , IHF_FILTER_CAPACITY = IHF_FILTER_COUNT + 1  // one more than the dll count
};

// EOF
