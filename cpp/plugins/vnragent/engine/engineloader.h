#pragma once

// engineloader.h
// 4/26/2014 jichi

class AbstractEngine;

namespace Engine {
AbstractEngine *getEngine();  // Needed to be explicitly deleted on exit
} // namespace Engine

// EOF
