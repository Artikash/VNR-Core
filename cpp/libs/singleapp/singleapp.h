#ifndef SINGLEAPP_H
#define SINGLEAPP_H

// singleapp.h
// 2/28/2013 jichi

#ifndef SINGLEAPP_BEGIN_NAMESPACE
# define SINGLEAPP_BEGIN_NAMESPACE
#endif
#ifndef SINGLEAPP_END_NAMESPACE
# define SINGLEAPP_END_NAMESPACE
#endif

SINGLEAPP_BEGIN_NAMESPACE
/// Return if this is the first process of the application
bool single_app();
SINGLEAPP_END_NAMESPACE

#endif // SINGLEAPP_H
