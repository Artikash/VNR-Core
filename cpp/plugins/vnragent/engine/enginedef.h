#pragma once

// enginedef.h
// 4/26/2014 jichi

namespace Engine {

// These values must be consistent with VNR's textman.py
enum TextRole { UnknownRole = 0, ScenarioRole,  NameRole, OtherRole,
                RoleCount };

// Emperical maximum memory range of an application
enum { MaximumMemoryRange = 0x300000 };

} // namespace Engine

// EOF
