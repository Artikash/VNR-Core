// bgi.cc
// 5/11/2014 jichi
#include "engine/model/bgi.h"
#include "engine/engineenv.h"

bool BGIEngine::match() { return Engine::globs("BGI.*"); }

// BGI2 pattern also exists in BGI1
bool BGIEngine::attach() { return attachBGIType2() || attachBGIType1(); }

// EOF
