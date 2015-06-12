#ifndef DYNSJIS_H
#define DYNSJIS_H

// dynsjis.h
// 6/11/2015 jichi

#if defined(_WIN32) || defined(_WIN64)
# define stdcall __stdcall
#else
# define stdcall
#endif // _WIN32 || _WIN64

namespace dynsjis {

char * stdcall CharNextA(const char *s);

} // namespace dynsjis

#endif // DYNSJIS_H
