#pragma once

// engine/engine_p.h
// 8/23/2013 jichi
// See: http://ja.wikipedia.org/wiki/プロジェクト:美少女ゲーム系/ゲームエンジン

#include "config.h"

namespace Engine {

// Global variables
extern wchar_t process_name_[MAX_PATH]; // cached
extern DWORD module_base_,
             module_limit_;

//extern LPVOID trigger_addr;
typedef bool (* trigger_fun_t)(LPVOID addr, DWORD frame, DWORD stack);
extern trigger_fun_t trigger_fun_;

// Engine-specific hooks

bool InsertAbelHook();          // Abel
bool InsertAliceHook();         // System40@AliceSoft; do not work for latest alice games
bool InsertAnex86Hook();        // Anex86: anex86.exe
bool InsertApricotHook();       // Apricot: arc.a*
bool InsertArtemisHook();       // Artemis Engine: *.pfs
bool InsertAtelierHook();       // Atelier Kaguya: message.dat
bool InsertBGIHook();           // BGI: BGI.*
bool InsertC4Hook();            // C4: C4.EXE or XEX.EXE
bool InsertCaramelBoxHook();    // Caramel: *.bin
bool InsertCandyHook();         // SystemC@CandySoft: *.fpk
bool InsertCatSystem2Hook();    // CatSystem2: *.int
bool InsertCMVSHook();          // CMVS: data/pack/*.cpz; do not support the latest cmvs32.exe and cmvs64.exe
bool InsertCotophaHook();       // Cotopha: *.noa
bool InsertDebonosuHook();      // Debonosu: bmp.bak and dsetup.dll
bool InsertEMEHook();           // EmonEngine: emecfg.ecf
bool InsertGesen18Hook();       // Gsen18: *.szs
bool InsertGXPHook();           // GXP: *.gxp
bool InsertLiveHook();          // Live: live.dll
bool InsertMalieHook();         // Malie@light: malie.ini
bool InsertMajiroHook();        // MAJIRO: *.arc
bool InsertMEDHook();           // MED: *.med
bool InsertNextonHook();        // NEXTON: aInfo.db
bool InsertNitroPlusHook();     // NitroPlus: *.npa
bool InsertPensilHook();        // Pensil: PSetup.exe
bool InsertQLIEHook();          // QLiE: GameData/*.pack
//bool InsertRai7Hook();          // Rai7puk: rai7.exe
bool InsertRejetHook();         // Rejet: Module/{gd.dat,pf.dat,sd.dat}
bool InsertRUGPHook();          // rUGP: rUGP.exe
bool InsertRetouchHook();       // Retouch: resident.dll
bool InsertRREHook();           // RunrunEngine: rrecfg.rcf
bool InsertShinaHook();         // ShinaRio: Rio.ini
bool InsertShinyDaysHook();     // ShinyDays
bool InsertSystem43Hook();      // System43@AliceSoft: AliceStart.ini
bool InsertSiglusHook();        // SiglusEngine: SiglusEngine.exe
bool InsertTanukiHook();        // Tanuki: *.tak
bool InsertTaskforce2Hook();    // Taskforce2.exe
bool InsertTencoHook();         // Tenco: Check.mdx
bool InsertTriangleHook();      // Triangle: Execle.exe
//bool InsertSolfaHook();         // sol-fa-soft: *.iar
bool InsertWhirlpoolHook();     // YU-RIS: *.ypf
bool InsertWillPlusHook();      // WillPlus: Rio.arc
bool InsertWolfHook();          // Wolf: Data.wolf

//bool InsertBaldrHook();         // BaldrSkyZero (Unity3D): bsz.exe

void InsertAB2TryHook();        // Yane@AkabeiSoft2Try: YaneSDK.dll.
void InsertBrunsHook();         // Bruns: bruns.exe
void InsertLuneHook();          // Lune: *.mbl
void InsertKiriKiriHook();      // KiriKiri: *.xp3, resource string
void InsertIronGameSystemHook();// IroneGameSystem: igs_sample.exe
void InsertLucifenHook();       // Lucifen@Navel: *.lpk
void InsertRyokuchaHook();      // Ryokucha: _checksum.exe
void InsertRealliveHook();      // RealLive: RealLive*.exe
void InsertSoftHouseHook();     // SoftHouse: *.vfs
void InsertStuffScriptHook();   // Stuff: *.mpk
void InsertTinkerBellHook();    // TinkerBell: arc00.dat
void InsertWaffleHook();        // WAFFLE: cg.pak

// CIRCUS: avdata/
bool InsertCircusHook1();
bool InsertCircusHook2();

} // namespace Engine

// EOF
