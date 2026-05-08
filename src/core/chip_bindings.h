// TigerFlame - Chip Bindings
// C-compatible interface for mml2vgm-rs or other chip synthesis backends

#pragma once

#include "parameter_model.h"

#include <cstdint>

// Opaque handle for chip instances
typedef void* ChipHandle;

// Chip types (must match ChipType enum)
typedef enum {
    CHIP_YM2151 = 0,
    CHIP_YM2612,
    CHIP_SN76489,
    CHIP_OPL2,
    CHIP_OPL3,
    CHIP_QSOUND,
    CHIP_C140,
    CHIP_POKEY,
    CHIP_VRC6,
    CHIP_COUNT
} ChipTypeFFI;

// Command types for chip commands
typedef enum {
    CHIP_CMD_NOTE_ON,
    CHIP_CMD_NOTE_OFF,
    CHIP_CMD_SET_PARAM,
    CHIP_CMD_PITCH_BEND,
    CHIP_CMD_RESET
} ChipCommandTypeFFI;

// Chip command structure
typedef struct {
    ChipCommandTypeFFI type;
    int chip_index;
    union {
        struct {
            int note;
            float velocity;
        } note;
        struct {
            int param_id;
            float value;
        } param;
        double pitch_bend;
    } data;
} ChipCommandFFI;

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the chip binding layer
// Call this before using any other functions
int chip_bindings_init();

// Shutdown the chip binding layer
void chip_bindings_shutdown();

// Create a new chip instance
// Returns NULL on failure
ChipHandle chip_create(ChipTypeFFI type, float sample_rate);

// Destroy a chip instance
void chip_destroy(ChipHandle handle);

// Process a single sample
// left and right are pointers to float values to accumulate into
void chip_process_sample(ChipHandle handle, float* left, float* right);

// Send a command to a chip
void chip_send_command(ChipHandle handle, const ChipCommandFFI* cmd);

// Reset a chip to initial state
void chip_reset(ChipHandle handle);

// Get the number of parameters for a chip type
int chip_get_param_count(ChipTypeFFI type);

// Get parameter name
const char* chip_get_param_name(ChipTypeFFI type, int param_id);

// Get parameter value
float chip_get_param(ChipHandle handle, int param_id);

// Set parameter value
void chip_set_param(ChipHandle handle, int param_id, float value);

// Get chip name
const char* chip_get_name(ChipTypeFFI type);

// Get chip short name
const char* chip_get_short_name(ChipTypeFFI type);

#ifdef __cplusplus
}
#endif

// C++ wrapper class for convenience
#ifdef __cplusplus

namespace TigerFlame {

class ChipBinding {
public:
    ChipBinding() {
        if (!initialized_) {
            chip_bindings_init();
            initialized_ = true;
        }
    }
    
    ~ChipBinding() {
        // Don't shutdown here - multiple instances might exist
    }
    
    ChipHandle createChip(ChipType type, float sampleRate) {
        return chip_create(static_cast<ChipTypeFFI>(type), sampleRate);
    }
    
    void destroyChip(ChipHandle handle) {
        chip_destroy(handle);
    }
    
    void processSample(ChipHandle handle, float* left, float* right) {
        chip_process_sample(handle, left, right);
    }
    
    int getParamCount(ChipType type) {
        return chip_get_param_count(static_cast<ChipTypeFFI>(type));
    }
    
    const char* getParamName(ChipType type, int paramId) {
        return chip_get_param_name(static_cast<ChipTypeFFI>(type), paramId);
    }
    
    float getParam(ChipHandle handle, int paramId) {
        return chip_get_param(handle, paramId);
    }
    
    void setParam(ChipHandle handle, int paramId, float value) {
        chip_set_param(handle, paramId, value);
    }
    
    const char* getName(ChipType type) {
        return chip_get_name(static_cast<ChipTypeFFI>(type));
    }
    
    const char* getShortName(ChipType type) {
        return chip_get_short_name(static_cast<ChipTypeFFI>(type));
    }

private:
    static bool initialized_;
};

} // namespace TigerFlame

#endif // __cplusplus
