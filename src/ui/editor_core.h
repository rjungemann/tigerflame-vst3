// TigerFlame VST3/CLAP Plugin
// Copyright (c) 2025 TigerFlame
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <memory>

namespace Steinberg {
    namespace Vst {
        class IEditorDelegate;
    }
}

namespace UI {

class EditorCore {
public:
    EditorCore();
    ~EditorCore();
    
    // Initialize the editor
    bool initialize(void* systemWindow, int32_t width, int32_t height);
    
    // Called when editor is attached
    void attached();
    
    // Called when editor is detached
    void detached();
    
    // Called before processing
    void beforeProcess();
    
    // Called after processing
    void afterProcess();
    
    // Resize the editor
    void resize(int32_t width, int32_t height);
    
    // Set parameter value from UI
    void setParameterValue(int32_t paramId, float value);
    
    // Get parameter value for UI
    float getParameterValue(int32_t paramId) const;
    
    // Get editor title
    const char* getTitle() const;

private:
    void* systemWindow_ = nullptr;
    int32_t width_ = 800;
    int32_t height_ = 600;
    
    // Prevent copying
    EditorCore(const EditorCore&) = delete;
    EditorCore& operator=(const EditorCore&) = delete;
};

} // namespace UI
