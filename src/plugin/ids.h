// TigerFlame VST3 Plugin - Plugin IDs
// This file defines unique identifiers for the plugin and its components

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/main/pluginfactory.h"

// Plugin UIDs (Unique IDs)
// These must be unique across all VST3 plugins
// Use UUID generator to create new ones: https://www.uuidgenerator.net/

// Plugin class ID (for the AudioEffect)
// A1B2C3D4-E5F6-7890-1234-567890ABCDEF
DECLARE_UID(TigerFlameProcessorUID, 0xA1B2C3D4, 0xE5F67890, 0x12345678, 0x90ABCDEF0)

// Controller class ID (for the EditController)
// FEDCBA09-8765-4321-FEDC-BA9876543210
DECLARE_UID(TigerFlameControllerUID, 0xFEDCBA09, 0x87654321, 0xFEDCBA98, 0x76543210)

// Plugin bundle ID (for macOS)
#define TigerFlameBundleID "com.tigerflame.TigerFlame"

// Plugin version information
#define TIGERFLAME_VST3_VERSION 0x00010000 // v1.0.0 in hex (major.minor.patch.build)
