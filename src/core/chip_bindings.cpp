// TigerFlame - Chip Bindings Implementation
// C++ implementation that calls into mml2vgm-rs Rust FFI

#include "chip_bindings.h"

#include <cstring>

// External declarations from mml2vgm-rs FFI (src/ffi.rs)
// Functions are prefixed with mml2vgm_ to avoid C++ name mangling conflicts
extern "C" {
    int mml2vgm_chip_bindings_init();
    void mml2vgm_chip_bindings_shutdown();
    void* mml2vgm_chip_create(int type, float sample_rate);
    void mml2vgm_chip_destroy(void* handle);
    void mml2vgm_chip_process_sample(void* handle, float* left, float* right);
    void mml2vgm_chip_reset(void* handle);
    int mml2vgm_chip_get_param_count(int type);
    const char* mml2vgm_chip_get_param_name(int type, int param_id);
    float mml2vgm_chip_get_param(void* handle, int param_id);
    void mml2vgm_chip_set_param(void* handle, int param_id, float value);
    const char* mml2vgm_chip_get_name(int type);
    const char* mml2vgm_chip_get_short_name(int type);
    void mml2vgm_chip_free_string(const char* s);
}

// Static initialization flag
static bool g_initialized = false;

int chip_bindings_init() {
    if (g_initialized) {
        return 0;
    }
    
    int result = ::mml2vgm_chip_bindings_init();
    if (result == 0) {
        g_initialized = true;
    }
    return result;
}

void chip_bindings_shutdown() {
    if (g_initialized) {
        ::mml2vgm_chip_bindings_shutdown();
        g_initialized = false;
    }
}

ChipHandle chip_create(ChipTypeFFI type, float sample_rate) {
    return ::mml2vgm_chip_create(static_cast<int>(type), sample_rate);
}

void chip_destroy(ChipHandle handle) {
    ::mml2vgm_chip_destroy(handle);
}

void chip_process_sample(ChipHandle handle, float* left, float* right) {
    ::mml2vgm_chip_process_sample(handle, left, right);
}

void chip_send_command(ChipHandle handle, const ChipCommandFFI* cmd) {
    // TODO: Implement command handling
    // For now, this is a placeholder
    // In a full implementation, we'd convert ChipCommandFFI to Rust format
    // and call into mml2vgm-rs
    switch (cmd->type) {
        case CHIP_CMD_NOTE_ON:
            // Note on - would need to implement in Rust FFI
            break;
        case CHIP_CMD_NOTE_OFF:
            // Note off
            break;
        case CHIP_CMD_SET_PARAM:
            chip_set_param(handle, cmd->data.param.param_id, cmd->data.param.value);
            break;
        case CHIP_CMD_PITCH_BEND:
            // Pitch bend
            break;
        case CHIP_CMD_RESET:
            chip_reset(handle);
            break;
    }
}

void chip_reset(ChipHandle handle) {
    ::mml2vgm_chip_reset(handle);
}

int chip_get_param_count(ChipTypeFFI type) {
    return ::mml2vgm_chip_get_param_count(static_cast<int>(type));
}

const char* chip_get_param_name(ChipTypeFFI type, int param_id) {
    return ::mml2vgm_chip_get_param_name(static_cast<int>(type), param_id);
}

float chip_get_param(ChipHandle handle, int param_id) {
    return ::mml2vgm_chip_get_param(handle, param_id);
}

void chip_set_param(ChipHandle handle, int param_id, float value) {
    ::mml2vgm_chip_set_param(handle, param_id, value);
}

const char* chip_get_name(ChipTypeFFI type) {
    return ::mml2vgm_chip_get_name(static_cast<int>(type));
}

const char* chip_get_short_name(ChipTypeFFI type) {
    return ::mml2vgm_chip_get_short_name(static_cast<int>(type));
}
