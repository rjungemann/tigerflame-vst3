// TigerFlame VST3 Plugin - Version Information

#pragma once

#include "tigerflame_build_info.h"

// Version constants
namespace TigerFlame {

constexpr int kVersionMajor = TIGERFLAME_VERSION_MAJOR;
constexpr int kVersionMinor = TIGERFLAME_VERSION_MINOR;
constexpr int kVersionPatch = TIGERFLAME_VERSION_PATCH;
constexpr int kVersionBuild = 0;

// Version as integer (for VST3)
constexpr int kVersionInt = 
    (kVersionMajor << 24) | 
    (kVersionMinor << 16) | 
    (kVersionPatch << 8) | 
    kVersionBuild;

// Version string
constexpr const char* kVersionString = TIGERFLAME_VERSION_STRING;

// SDK version compatibility
constexpr int kVst3SdkVersion = 0x030707; // VST3 SDK 3.7.7

} // namespace TigerFlame
