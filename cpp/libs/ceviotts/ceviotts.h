#pragma once

// ceviotts.h
// 4/15/2014 jichi

#ifdef _MSC_VER
# include <cstddef> // for wchar_t
#endif // _MSC_VER

// - Types -

namespace CeVIO { struct ITalker; }
typedef CeVIO::ITalker ceviotts_t; // opaque handle

// - Functions -

ceviotts_t *ceviotts_create();
void ceviotts_destroy(ceviotts_t *talker);

//bool ceviotts_set_cast(ceviotts_t *talker, const char *castname);

bool ceviotts_speak(ceviotts_t *talker, const wchar_t *sentence, unsigned long flags = 0);

// EOF
