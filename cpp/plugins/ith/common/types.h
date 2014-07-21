#pragma once

// ith/common/types.h
// 8/23/2013 jichi
// Branch: ITH/common.h, rev 128

#include <windows.h> // needed for windef types

 /** jichi 3/7/2014: Add guessed comment
  *
  *  DWORD  addr  absolute or relative address
  *  DWORD  split   esp offset of the split character
  *
  *  http://faydoc.tripod.com/cpu/pushad.htm
  *  http://agth.wikia.com/wiki/Cheat_Engine_AGTH_Tutorial
  *  The order is the same as pushd
  *  EAX, ECX, EDX, EBX, EBP, ESP (original value), EBP, ESI, and EDI (if the current operand-size attribute is 32) and AX, CX, DX, BX, SP
  *  Negative values of 'data_offset' and 'sub_offset' refer to registers:-4 for EAX, -8 for ECX, -C for EDX, -10 for EBX, -14 for ESP, -18 for EBP, -1C for ESI, -20 for EDIhttp://agth.wikia.com/wiki/Cheat_Engine_AGTH_Tutorial
  */
struct HookParam { // size: 0x24
  // jichi 8/24/2013: For special hooks. Orignial name: DataFun
  typedef void (*extern_fun_t)(DWORD, HookParam *, DWORD *, DWORD *, DWORD *);

  DWORD addr;   // absolute or relative address
  DWORD off,    // offset of the data in the memory
        ind,    // ?
        split,  // esp offset of the split character = pusha offset - 4
        split_ind;  // ?
  DWORD module, // hash of the module
        function;
  extern_fun_t extern_fun;
  DWORD type;   // flags
  WORD length_offset; // index of the string length
  BYTE hook_len, // ?
       recover_len; // ?

  // 7/20/2014: jichi additional parameters for PSP games
  DWORD userFlags,
        userValue;
};

// jichi 6/1/2014: Structure of the esp for extern functions
struct HookStack
{
  // pushad
  DWORD edi, // -0x24
        esi, // -0x20
        ebp, // -0x1c
        esp, // -0x18
        ebx, // -0x14
        edx, // -0x10
        ecx, // -0xc
        eax; // -0x8
  // pushfd
  DWORD eflags; // -0x4
  DWORD retaddr; // 0
  DWORD args[1]; // 0x4
};

struct SendParam {
  DWORD type;
  HookParam hp;
};

struct Hook { // size: 0x80
  HookParam hp;
  LPWSTR hook_name;
  int name_length;
  BYTE recover[0x68 - sizeof(HookParam)];
  BYTE original[0x10];

  DWORD Address() const { return hp.addr; }
  DWORD Type() const { return hp.type; }
  WORD Length() const { return hp.hook_len; }
  LPWSTR Name() const { return hook_name; }
  int NameLength() const { return name_length; }
};

// EOF
