// TigerFlame VST3/CLAP Plugin
// Copyright (c) 2025 TigerFlame
// SPDX-License-Identifier: MIT

#include "editor_core.h"

namespace UI {

EditorCore::EditorCore() = default;

EditorCore::~EditorCore() = default;

bool EditorCore::initialize(void* systemWindow, int32_t width, int32_t height) {
    systemWindow_ = systemWindow;
    width_ = width;
    height_ = height;
    
    // TODO: Phase 5 - Initialize ImGui context and UI
    return true;
}

void EditorCore::attached() {
    // TODO: Phase 5 - Editor attached
}

void EditorCore::detached() {
    // TODO: Phase 5 - Editor detached
}

void EditorCore::beforeProcess() {
    // TODO: Phase 5 - Before processing
}

void EditorCore::afterProcess() {
    // TODO: Phase 5 - After processing
}

void EditorCore::resize(int32_t width, int32_t height) {
    width_ = width;
    height_ = height;
    // TODO: Phase 5 - Resize UI
}

void EditorCore::setParameterValue(int32_t paramId, float value) {
    // TODO: Phase 5 - Set parameter value
    (void)paramId;
    (void)value;
}

float EditorCore::getParameterValue(int32_t paramId) const {
    // TODO: Phase 5 - Get parameter value
    (void)paramId;
    return 0.0f;
}

const char* EditorCore::getTitle() const {
    return "TigerFlame";
}

} // namespace UI
