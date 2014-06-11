#pragma once

// enginedef.h
// 4/26/2014 jichi

namespace Engine {

// These values must be consistent with VNR's textman.py
enum TextRole { UnknownRole = 0, ScenarioRole,  NameRole, OtherRole, ChoiceRole = OtherRole, // TODO: Add support for choice
                RoleCount };

// Dummy fixed signature the same as ITH for single text thread
enum {
  SingleThreadSignatureBase = 0x10000
  , ScenarioThreadSignature = SingleThreadSignatureBase + ScenarioRole
  , NameThreadSignature = SingleThreadSignatureBase + NameRole
};

} // namespace Engine

// EOF
