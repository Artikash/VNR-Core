// bgi.cc
// 5/11/2014 jichi
#include "engine/model/bgi.h"
#include "engine/engineenv.h"

bool BGIEngine::match() { return Engine::glob("BGI.*"); }

bool BGIEngine::attach()
{
  // New BGI game (BGI2) does not have old BGI pattern (BGI1)
  //return attachBGIType2() || attachBGIType1();
  return attachBGIType2(); // Temporarily disable BGI1
}

// EOF
