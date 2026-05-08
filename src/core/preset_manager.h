// TigerFlame VST3/CLAP Plugin
// Copyright (c) 2025 TigerFlame
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Core {

class PluginState;

class PresetManager {
public:
    PresetManager();
    ~PresetManager();

    // Load preset from file
    bool loadPreset(const std::string& path);
    
    // Save preset to file
    bool savePreset(const std::string& path, const std::string& name);
    
    // Get preset list
    std::vector<std::string> getPresetList() const;
    
    // Get current preset name
    const std::string& getCurrentPresetName() const { return currentPresetName_; }
    
    // Set current preset name
    void setCurrentPresetName(const std::string& name) { currentPresetName_ = name; }
    
    // Apply preset to state
    void applyPreset(PluginState* state);
    
    // Store current state as preset
    void storePreset(PluginState* state, const std::string& name);

private:
    std::string currentPresetName_;
    std::vector<std::string> presetList_;
    
    // Prevent copying
    PresetManager(const PresetManager&) = delete;
    PresetManager& operator=(const PresetManager&) = delete;
};

} // namespace Core
