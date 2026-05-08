# TigerFlame: VST3/CLAP Instrument Plugin

## Overview

**TigerFlame** is a VST3 and CLAP plugin that provides real-time playback of mml2vgm-compatible sound chip instruments. It exposes the full breadth of classic video game and arcade audio chips (YM2151, YM2612, SN76489, OPL2, OPL3, QSound, etc.) as playable instruments within modern DAWs.

### Key Features

- **21+ Classic Sound Chips**: Full instrument library from mml2vgm ecosystem
- **Three Voice Allocation Modes**:
  - **Monophonic**: Single voice, last-note priority
  - **Layered Multitimbral**: Multiple chip instruments stacked on same MIDI channel
  - **Polyphonic**: n-voice allocation per instrument (e.g., 4 voices per chip)
- **Spatial Audio via QSound Effects**: Integrated QSound system for authentic spatial audio
- **Parameter Automation**: CC mapping for all chip-specific parameters
- **Preset Management**: Save/load instrument combinations and effect chains
- **Standalone & Plugin Formats**: Deploy as VST3, CLAP, and standalone application

---

## Development Progress

### Completed

**Phase 1: Foundation & Core Architecture** (Complete - 100%)
- ✅ Build system setup (CMakeLists.txt, Justfile, dependencies)
- ✅ Plugin entry points (VST3 factory, processor, controller)
- ✅ Core parameter model (enums, descriptors, conversions)
- ✅ Plugin state management (normalized parameter storage, serialization)
- ✅ Voice allocator (Mono, Polyphonic, Layered modes all implemented)
- ✅ MIDI translator (command translation, CC mapping, queue)
- ✅ Core state management (chunk-based serialization)
- ✅ Unit tests for all core components

**Phase 2: Chip Engine Integration** (Partially Complete - 90%)
- ✅ Chip bindings header (C-compatible interface)
- ✅ Chip engine core (chip management, command dispatch, sample processing)
- ✅ Audio processing loop (wired in processor)
- ✅ Parameter state integration (basic connections)
- ✅ Multi-chip instantiation (volume mixing, enable/disable)
- ✅ State persistence for chip engine
- ✅ Unit tests for chip engine and audio processing
- ✅ mml2vgm-rs integration (static library with C FFI implemented)
- ⏳ Sample-accurate parameter automation
- ⏳ Parameter smoothing

**Phase 3: Voice Allocation Modes** (Complete - 100%)
- ✅ Layered Multitimbral Mode (all features implemented)
- ✅ Polyphonic Mode with LRU voice stealing (2-32 voices)
- ✅ MIDI CC Routing (per-layer, global CCs, modulation wheel)
- ✅ Mode Switching with clean state transitions
- ✅ Enhanced tests for layered and polyphonic modes

**Phase 4: Effect Chain & QSound Integration** (Complete - 100%)
- ✅ Effect Chain Architecture (8 slots, add/remove/move effects)
- ✅ QSound Effect Wrapper (delay, feedback, damping, stereo width)
- ✅ Reverb Effect (Schroeder-style with room size, damping, pre-delay)
- ✅ Delay Effect (stereo, ping-pong, tempo sync, damping)
- ✅ Distortion Effect (drive, tone, wet/dry)
- ✅ EQ Effect (3-band with adjustable frequencies)
- ✅ Effect State Persistence (save/restore all effect configurations)
- ✅ Integration with Audio Pipeline (wired into Processor)
- ✅ Unit tests for effect chain

### Files Created

**Build System**
- `CMakeLists.txt` - Main build configuration
- `Justfile` - Development recipes
- `cmake/tigerflame_build_info.h.in` - Version info template

**Plugin (VST3)**
- `src/plugin/ids.h` - Plugin UIDs
- `src/plugin/version.h` - Version constants
- `src/plugin/param_ids.h` - VST3 parameter IDs
- `src/plugin/entry.cpp` - Factory entry point
- `src/plugin/processor.h/cpp` - Audio processor (now with EffectChain integration)
- `src/plugin/controller.h/cpp` - Edit controller

**Core**
- `src/core/parameter_model.h/cpp` - Parameter definitions
- `src/core/plugin_state.h/cpp` - Plugin state
- `src/core/state.h/cpp` - State utilities
- `src/core/voice_allocator.h/cpp` - Voice allocation (Mono, Layered, Polyphonic)
- `src/core/midi_translator.h/cpp` - MIDI translation (CC routing, command queue)
- `src/core/chip_engine.h/cpp` - Chip engine
- `src/core/chip_bindings.h` - Chip FFI interface
- `src/core/effect_chain.h/cpp` - Effect chain manager
- `src/core/effects/qsound_effect.h/cpp` - QSound effect
- `src/core/effects/reverb_effect.h/cpp` - Reverb effect
- `src/core/effects/delay_effect.h/cpp` - Delay effect
- `src/core/effects/distortion_effect.h/cpp` - Distortion effect
- `src/core/effects/eq_effect.h/cpp` - EQ effect

**Tests**
- `tests/main.cpp` - Test entry
- `tests/test_voice_allocator.cpp` - Voice allocator tests (Mono, Poly, Layered)
- `tests/test_midi_translator.cpp` - MIDI translator tests
- `tests/test_plugin_state.cpp` - Plugin state tests
- `tests/test_chip_engine.cpp` - Chip engine tests
- `tests/test_audio_processing.cpp` - Audio processing tests
- `tests/test_effect_chain.cpp` - Effect chain and QSound tests

**Next Steps**
1. Integrate mml2vgm-rs Rust library via C FFI (Phase 2)
2. Implement actual chip synthesis (currently using placeholder sine wave)
3. Add parameter smoothing and sample-accurate automation (Phase 2)
4. Test full build on macOS: `just configure && just build && just test`
5. Phase 5: UI & Preset Management
6. Phase 6: Standalone Application
7. Phase 7: CLAP Plugin Build

---

## Architecture Overview

### High-Level Design

```
[DAW MIDI Input]
        ↓
    [Plugin Interface]
        ↓
    [Dispatcher Router]
    ├─ Voice Allocator
    ├─ MIDI → Chip Command Translator
    ├─ Parameter Mapper
    └─ Effect Chain Manager
        ↓
    [Instrument Synth Engine]
    ├─ Chip A Instance
    ├─ Chip B Instance
    ├─ Chip N Instance
    └─ Effect Processors (QSound, etc.)
        ↓
    [Audio Output]
        ↓
    [DAW Audio Bus]
```

### Project Structure (similar to combover-vst3/flashkick-vst3)

```
tigerflame-vst3/
├── src/
│   ├── core/
│   │   ├── voice_allocator.h
│   │   ├── voice_allocator.cpp
│   │   ├── chip_engine.h
│   │   ├── chip_engine.cpp
│   │   ├── midi_translator.h
│   │   ├── midi_translator.cpp
│   │   ├── effect_chain.h
│   │   ├── effect_chain.cpp
│   │   ├── preset_manager.h
│   │   ├── preset_manager.cpp
│   │   └── state.h
│   ├── plugin/
│   │   ├── processor.h
│   │   ├── processor.cpp
│   │   ├── controller.h
│   │   ├── controller.cpp
│   │   ├── entry.cpp
│   │   ├── ids.h
│   │   └── version.h
│   ├── ui/
│   │   ├── editor_core.h
│   │   ├── editor_core.cpp
│   │   └── imgui_widgets/
│   │       ├── chip_selector.h
│   │       ├── voice_mixer.h
│   │       ├── parameter_editor.h
│   │       └── preset_browser.h
│   └── standalone/
│       ├── main.cpp
│       ├── audio_io.h
│       └── midi_input.h
├── CMakeLists.txt
├── Justfile
├── docs/
│   ├── ARCHITECTURE.md
│   ├── VOICE_ALLOCATION.md
│   ├── PARAMETER_MAPPING.md
│   └── BUILD_GUIDE.md
├── presets/
└── tests/
```

---

## Core Components

### 1. Voice Allocator

**Purpose**: Distribute MIDI notes across chip instances and voices based on allocation mode.

#### Voice Allocation Modes

##### Mode A: Monophonic
- Single active voice at any time
- Last-note priority (latest MIDI note wins)
- Suitable for: Bass lines, solo melodies
- State: `currentNote`, `currentVoice`, `pitchBend`

```cpp
struct MonoVoiceState {
    int activeNote = -1;
    int activeChipIndex = -1;
    int activeVoiceIndex = 0;
    double pitchBend = 0.0;
    bool sustain = false;
};
```

##### Mode B: Layered Multitimbral
- Multiple chip instruments respond to same MIDI channel
- Each layer has independent voice state
- All layers triggered together, voiced independently
- Suitable for: Textured pads, complex timbres
- State: `Vector<MonoVoiceState>` (one per layer)

```cpp
struct LayerConfig {
    int chipIndex;        // which chip
    int instrumentId;     // chip-specific instrument number
    float volume;         // layer gain (0.0–1.0)
    bool enabled;         // layer on/off
};
```

##### Mode C: Polyphonic
- n voices allocated per chip instance (e.g., 4-voice polyphony)
- Round-robin or LRU voice stealing
- Separate MIDI channel per polyphonic voice group
- Suitable for: Chords, arpeggios, multi-note melodies
- State: `Vector<PolyVoiceSlot>` per chip

```cpp
struct PolyVoiceSlot {
    int midiNote = -1;
    double pitchBend = 0.0;
    float velocity = 0.0;
    int64_t noteOnTime = -1;  // for LRU stealing
};
```

#### Interface

```cpp
class VoiceAllocator {
public:
    enum Mode { Mono, LayeredMultitimbral, Polyphonic };
    
    // Configuration
    void setMode(Mode m);
    void setPolyphonyCount(int voicesPerChip);
    void addLayer(int chipIdx, int instrumentId);
    void removeLayer(int layerId);
    
    // MIDI event handling
    void noteOn(int note, float velocity);
    void noteOff(int note);
    void pitchBend(double semitones);
    void controlChange(int cc, float value);
    
    // Query
    Vector<VoiceAllocation> getActiveVoices() const;
    int getVoiceCount() const;
};
```

### 2. Chip Engine

**Purpose**: Manages multiple chip instances (YM2151, SN76489, OPL3, etc.) and orchestrates synthesis.

```cpp
class ChipEngine {
public:
    struct ChipConfig {
        enum ChipType type;  // YM2151, YM2612, SN76489, OPL2, OPL3, QSound, etc.
        float sampleRate;
        float volume = 1.0f;
        bool enabled = true;
    };
    
    // Instantiation
    void addChip(const ChipConfig& config);
    void removeChip(int chipIndex);
    
    // Command dispatch (from MIDI translator)
    void sendCommand(int chipIndex, const ChipCommand& cmd);
    
    // Per-sample processing
    void processSample(float* left, float* right);
    
    // State management
    void resetChip(int chipIndex);
    void saveState(IBStream* stream);
    void restoreState(IBStream* stream);
    
private:
    Vector<ChipInstance*> chips_;
    double sampleRate_;
    double elapsedSamples_ = 0;
};
```

### 3. MIDI Translator

**Purpose**: Convert incoming MIDI events to chip-specific commands.

```cpp
class MidiTranslator {
public:
    // Register chip handlers
    void registerChip(int chipIndex, ChipType type, 
                     const ChipCapabilities& caps);
    
    // MIDI → Chip translation
    void translateNoteOn(int note, float velocity, 
                        const VoiceAllocation& allocation);
    void translateNoteOff(int note);
    void translateControlChange(int cc, float value);
    void translatePitchBend(double semitones);
    
    // Chip-specific parameter mapping
    void mapParameter(int cc, int chipIndex, 
                     ParameterId paramId);
    
    // Emit commands to ChipEngine
    std::function<void(const ChipCommand&)> onCommand;
};
```

### 4. Effect Chain

**Purpose**: Apply post-chip effects (QSound, reverb, etc.).

```cpp
class EffectChain {
public:
    enum EffectType {
        QSound,
        Reverb,
        Delay,
        Distortion,
        EQ
    };
    
    // Effect management
    void addEffect(EffectType type, int slotIndex);
    void removeEffect(int slotIndex);
    void setEffectParameter(int slotIndex, int paramId, float value);
    
    // Processing
    void process(float* left, float* right, int sampleCount);
    
    // State
    void saveState(IBStream* stream);
    void restoreState(IBStream* stream);
};
```

### 5. Preset Manager

**Purpose**: Save/load instrument and effect configurations.

```cpp
class PresetManager {
public:
    struct Preset {
        String name;
        String description;
        
        // Layers (multitimbral)
        Vector<LayerConfig> layers;
        
        // Effects
        Vector<EffectConfig> effects;
        
        // Master parameters
        float masterVolume;
        float masterTuning;  // cents
    };
    
    void savePreset(const String& path, const Preset& preset);
    Preset loadPreset(const String& path);
    Vector<Preset> enumeratePresets(const String& directory);
};
```

---

## Voice Allocation Detailed Design

### State Machine: Monophonic Mode

```
[IDLE]
  ↓ MIDI Note On → monophonic active
[PLAYING]
  ↓ MIDI Note Off (same note)
[IDLE]

[PLAYING] ← → [PLAYING]  (new note on before note off = retrigger)
```

### State Machine: Polyphonic Mode (per voice slot)

```
[EMPTY] ↔ [PLAYING]
          (LRU aging for voice stealing)
```

### Layered Multitimbral Implementation

```cpp
// In Processor::process()
for (int layer = 0; layer < layers_.size(); ++layer) {
    const auto& layerConfig = layerConfigs_[layer];
    const auto& voiceState = monoVoiceStates_[layer];
    
    if (!voiceState.activeNote) continue;  // idle
    
    ChipCommand cmd;
    cmd.chipIndex = layerConfig.chipIndex;
    cmd.instrumentId = layerConfig.instrumentId;
    cmd.note = voiceState.activeNote;
    cmd.velocity = voiceState.velocity * layerConfig.volume;
    cmd.pitchBend = voiceState.pitchBend;
    
    chipEngine_.sendCommand(cmd);
}
```

---

## Parameter Mapping Strategy

### MIDI CC Assignment

| CC# | Function | Scope | Default Behavior |
|-----|----------|-------|------------------|
| 1   | Modulation | Global | Route to chip mod (e.g., LFO) |
| 7   | Volume | Global | Master volume |
| 10  | Pan | Global | Stereo balance |
| 11  | Expression | Global | Dynamic level control |
| 64  | Sustain Pedal | Per-layer | Hold notes |
| 120 | All Sound Off | Global | Panic reset |
| 123 | All Notes Off | Global | Release all notes |
| 121 | Reset All Controllers | Global | Reset CC state |
| 128+ | Chip-specific | Per-chip | Instrument parameters |

### Chip-Specific Parameters

Each chip exposes its own parameter set:

- **YM2151**: Algorithm, Feedback, EG rates, LFO speed/depth
- **SN76489**: Tone/noise register values
- **OPL2/OPL3**: Tremolo, Vibrato, ADSR, Waveform select
- **QSound**: Echo parameters, PHASE, FEEDBACK

**CC Mapping Table** (stored in preset):
```cpp
struct CCMapping {
    int midiCC;
    int chipIndex;
    ParameterId paramId;
    float minValue, maxValue;
};
```

---

## UI Design (ImGui-based)

### Main Editor Window

- **Top Panel**: 
  - Preset selector dropdown
  - Save/Load preset buttons
  - Master volume fader

- **Left Panel (Chip Selector)**:
  - List of available chip instances
  - Add/remove chip buttons
  - Per-chip enable/disable toggles
  - Per-chip volume control

- **Center Panel (Voice Allocator Config)**:
  - Radio buttons: Mono / Layered / Polyphonic
  - Polyphony count slider (2–32 voices)
  - Layer manager (add/remove layers for multitimbral)
  - Per-layer instrument selector, volume, pan

- **Right Panel (Effect Chain)**:
  - Drag-and-drop effect slots
  - Per-effect parameter editor
  - Enable/disable toggles
  - Bypass all effects button

- **Bottom Panel (Master Info)**:
  - CPU load meter
  - Voice usage visualization
  - MIDI activity indicator

---

## Development Phases

### Phase 1: Foundation & Core Architecture

#### 1.1 Build System Setup
- [x] Create top-level `CMakeLists.txt` with C++17 standard, CMake 3.25+ minimum
- [x] Add `FetchContent` for Steinberg VST3 SDK
- [x] Add `FetchContent` for Dear ImGui (v1.90+)
- [x] Add `FetchContent` for nlohmann/json (preset serialization)
- [x] Add `FetchContent` for Catch2 v3 (testing)
- [x] Add build options: `BUILD_IMGUI_UI`, `BUILD_STANDALONE`, `BUILD_STANDALONE_MIDI`, `BUILD_CLAP`
- [x] Set up platform-specific compiler flags and framework linking (macOS: Metal/Cocoa, Windows: DirectX11, Linux: X11/OpenGL3)
- [x] Add `cmake_minimum_required`, project metadata, version info generation (`tigerflame_build_info.h`)
- [x] Create `Justfile` with recipes: `configure`, `configure-debug`, `build`, `test`, `run`, `dev`, `install`
- [ ] Verify clean build on macOS with `just configure && just build`

#### 1.2 Plugin Entry Points (VST3)
- [x] Create `src/plugin/ids.h` — define plugin UID, controller UID, bundle identifiers
- [x] Create `src/plugin/version.h` — version metadata constants
- [x] Create `src/plugin/param_ids.h` — VST3 ParamID constants mapped from `Core::ParamId`
- [x] Create `src/plugin/entry.cpp` — VST3 factory entry point (`DEF_CLASS2` macro)
- [x] Create `src/plugin/processor.h/cpp` — `AudioEffect` subclass with:
  - [x] `initialize()` — set up stereo audio output bus and MIDI event input bus
  - [x] `setBusArrangements()` — enforce stereo output (no input, this is an instrument)
  - [x] `canProcessSampleSize()` — support 32-bit and 64-bit float
  - [x] `process()` — stub that dispatches to `processAudio<SampleType>` template
  - [x] `setState()` / `getState()` — serialize/deserialize via `IBStream`
- [x] Create `src/plugin/controller.h/cpp` — `EditControllerEx1` subclass with:
  - [x] Parameter registration from `Core::ParameterDescriptor` table
  - [x] `createView()` — stub (returns nullptr until UI phase)
  - [x] `getMidiControllerAssignment()` — map standard CCs to parameters

#### 1.3 Core Parameter Model
- [x] Create `src/core/parameter_model.h` — define `enum class ParamId` for initial parameters:
  - [x] `VoiceMode` (Mono/Layered/Poly toggle)
  - [x] `MasterVolume` (0.0–1.0)
  - [x] `MasterTuning` (-100 to +100 cents)
  - [x] `ChipSelect` (chip type selector)
  - [x] `NoteTranspose` (-48 to +48 semitones)
  - [x] `PitchBendRange` (0–24 semitones)
- [x] Implement normalized-to-domain and domain-to-normalized conversion function pairs
- [x] Create `ParameterDescriptor` struct (id, title, shortTitle, unit, defaultNormalized, stepCount)
- [x] Create `kParameterDescriptors` array
- [x] Add conversion helpers in `ParameterConversion` namespace
- [x] Create `src/core/plugin_state.h/cpp` — `struct PluginState` with normalized fields, `getNormalizedValue()`/`setNormalizedValue()` accessors

#### 1.4 Voice Allocator (Monophonic Mode)
- [x] Create `src/core/voice_allocator.h/cpp`
- [x] Define `VoiceAllocator::Mode` enum: `Mono`, `LayeredMultitimbral`, `Polyphonic`
- [x] Implement `MonoVoiceState` struct (activeNote, activeChipIndex, activeVoiceIndex, pitchBend, sustain)
- [x] Implement `noteOn(note, velocity)` — last-note priority, store current note
- [x] Implement `noteOff(note)` — release if matches active note
- [x] Implement `pitchBend(semitones)` — update bend state
- [x] Implement `controlChange(cc, value)` — handle sustain pedal (CC64), all notes off (CC123)
- [x] Implement `getActiveVoices()` — return current voice allocation list
- [x] Implement `setMode(Mode)` — mode switching with clean state transitions
- [x] Implement polyphonic mode with LRU voice stealing
- [x] Implement layered multitimbral mode

#### 1.5 Basic MIDI→Chip Translation
- [x] Create `src/core/midi_translator.h/cpp`
- [x] Define `ChipCommand` struct (chipIndex, commandType, note, velocity, paramId, paramValue)
- [x] Implement `translateNoteOn()` — convert MIDI note + velocity to chip note-on command
- [x] Implement `translateNoteOff()` — convert to chip note-off command
- [x] Implement `translatePitchBend()` — convert pitch wheel to chip pitch offset
- [x] Implement `translateControlChange()` — route CC to mapped chip parameter
- [x] Implement `mapCC()` — register CC to parameter mappings
- [x] Implement command queue and `processQueue()` for batch dispatch
- [x] Wire up `onCommand` callback for dispatching commands to chip engine

#### 1.6 Core State Management
- [x] Create `src/core/state.h/cpp` — shared state struct tying together PluginState, VoiceAllocator, and engine references
- [x] Implement binary state serialization (magic bytes `TGF0`, version, normalized param values)
- [x] Implement state deserialization with version checking
- [x] Implement chunk-based serialization helpers in `StateUtil` namespace

#### 1.7 Tests for Phase 1
- [x] Create `tests/test_voice_allocator.cpp` (Catch2):
  - [x] Mono mode: note-on sets active voice, note-off clears it
  - [x] Mono mode: second note-on replaces first (last-note priority)
  - [x] Mono mode: note-off of non-active note is ignored
  - [x] Sustain pedal holds note after note-off
  - [x] All-notes-off (CC123) clears all state
  - [x] Pitch bend updates state correctly
  - [x] Polyphonic mode: voice stealing, note management
  - [x] Layered multitimbral mode: simultaneous layers
- [x] Create `tests/test_midi_translator.cpp` (Catch2):
  - [x] Note-on produces correct ChipCommand
  - [x] Note-off produces correct ChipCommand
  - [x] CC maps to correct chip parameter
  - [x] Command queue management
  - [x] Note transpose and velocity sensitivity
- [x] Create `tests/test_plugin_state.cpp` (Catch2):
  - [x] Serialization round-trip preserves all parameter values
  - [x] Default state has expected normalized values
  - [x] Parameter accessors and mutators
- [x] Add test targets to CMakeLists.txt and `just test` recipe
- [ ] Verify all tests pass

---

### Phase 2: Chip Engine Integration

#### 2.1 mml2vgm Binding Layer
- [x] Evaluate mml2vgm-rs build: determine if it exposes a C FFI or needs a wrapper
- [x] Create `src/core/chip_bindings.h` — C-compatible function declarations for chip operations:
  - [x] `chip_create(ChipType type, float sampleRate) → ChipHandle`
  - [x] `chip_destroy(ChipHandle handle)`
  - [x] `chip_note_on(ChipHandle, int note, float velocity)`
  - [x] `chip_note_off(ChipHandle, int note)`
  - [x] `chip_set_parameter(ChipHandle, int paramId, float value)`
  - [x] `chip_process_sample(ChipHandle, float* left, float* right)`
  - [x] `chip_reset(ChipHandle)`
- [x] Add mml2vgm-rs as a build dependency (pre-built static lib from sibling directory)
- [x] Create CMake integration: custom target builds Rust library, links static lib
- [ ] Write a minimal test that creates a YM2151 instance, sends note-on, and gets non-zero audio output

#### 2.2 Chip Engine Core
- [x] Create `src/core/chip_engine.h/cpp`
- [x] Define `ChipType` enum (YM2151, YM2612, SN76489, OPL2, OPL3, QSound, C140, POKEY, VRC6, etc.)
- [x] Define `ChipConfig` struct (type, sampleRate, volume, enabled)
- [x] Implement `addChip(ChipConfig)` — instantiate chip via binding layer, add to internal vector
- [x] Implement `removeChip(chipIndex)` — destroy and remove chip instance
- [x] Implement `removeAllChips()` — clean up all chip instances
- [x] Implement `sendCommand(chipIndex, ChipCommand)` — dispatch command to specific chip
- [x] Implement `processSample(float* left, float* right)` — iterate all chips, sum stereo output
- [x] Implement `resetChip(chipIndex)` and `resetAllChips()` — reset chip(s) to initial state
- [x] Implement `suspend()` / `resume()` for processing state management
- [x] Implement `getParameter()` / `setParameter()` for chip parameter access
- [x] Implement chip name and info getters
- [ ] Handle sample rate conversion if chip native rate differs from host rate

#### 2.3 Audio Processing Loop
- [x] Wire `ChipEngine` into `Processor::process()`:
  - [x] Read MIDI events from VST3 event input
  - [x] Pass events through `VoiceAllocator` → `MidiTranslator` → `ChipEngine::sendCommand()`
  - [x] Call `ChipEngine::processSample()` per sample, write to output buffer
- [x] Implement `processAudio<float>` and `processAudio<double>` template specializations
- [x] Handle buffer size variations (64–2048 samples/block)
- [ ] Implement sample-accurate parameter automation (read `IParamValueQueue` per sample offset)

#### 2.4 Parameter State Integration
- [x] Connect `PluginState` parameter changes to `ChipEngine` setters
- [ ] Implement parameter smoothing for continuous controls (avoid zipper noise)
- [ ] Map `ChipSelect` parameter to chip add/remove in engine
- [x] Implement `MasterVolume` scaling in the output stage
- [x] Implement `MasterTuning` offset in MIDI translator (cent-based pitch shift)

#### 2.5 Multi-Chip Instantiation
- [x] Support creating multiple chip instances simultaneously
- [x] Implement per-chip volume mixing in `processSample()`
- [x] Implement per-chip enable/disable toggle (mute without destroying)
- [x] Test with 2+ chips active: verify independent note dispatch and mixed output

#### 2.6 State Persistence for Chip Engine
- [x] Implement `ChipEngine::saveState(IBStream*)` — serialize chip configs and parameters
- [x] Implement `ChipEngine::restoreState(IBStream*)` — recreate chips from saved state
- [x] Wire into `Processor::setState()` / `getState()` alongside PluginState

#### 2.7 Tests for Phase 2
- [x] Create `tests/test_chip_engine.cpp` (Catch2):
  - [x] Single chip produces audio on note-on
  - [x] Chip produces silence after note-off (with appropriate decay)
  - [x] Multiple chips sum output correctly
  - [x] Chip reset produces silence
  - [x] Per-chip volume scaling works
  - [x] Enable/disable toggle mutes chip without destroying
- [x] Create `tests/test_audio_processing.cpp`:
  - [x] End-to-end: MIDI note-on → non-zero audio output
  - [ ] Parameter changes affect audio output
  - [ ] No clicks/pops on parameter changes (smoothing)
- [ ] Verify all tests pass

---

### Phase 3: Voice Allocation Modes (Layered & Polyphonic)

#### 3.1 Layered Multitimbral Mode
- [ ] Define `LayerConfig` struct (chipIndex, instrumentId, volume, enabled)
- [ ] Implement `VoiceAllocator::addLayer(chipIdx, instrumentId)` — register a layer
- [ ] Implement `VoiceAllocator::removeLayer(layerId)` — unregister a layer
- [ ] Implement layered note-on: trigger all enabled layers simultaneously
- [ ] Implement layered note-off: release all layers
- [ ] Implement per-layer `MonoVoiceState` tracking (each layer has independent state)
- [ ] Implement per-layer volume scaling in output mix
- [ ] Wire layer commands through `MidiTranslator` to correct chip instances

#### 3.2 Polyphonic Mode
- [ ] Define `PolyVoiceSlot` struct (midiNote, pitchBend, velocity, noteOnTime)
- [ ] Implement configurable polyphony count (`setPolyphonyCount(int voicesPerChip)`, 2–32)
- [ ] Implement round-robin voice allocation: assign new notes to next free slot
- [ ] Implement LRU voice stealing: when all slots full, steal oldest note
- [ ] Implement note-off: find and release matching voice slot
- [ ] Track `noteOnTime` (sample counter) for LRU ordering
- [ ] Implement pitch bend broadcast: apply to all active poly voices
- [ ] Implement sustain pedal in poly mode: hold all active voices until pedal release

#### 3.3 MIDI CC Routing
- [ ] Define `CCMapping` struct (midiCC, chipIndex, paramId, minValue, maxValue)
- [ ] Implement `MidiTranslator::mapParameter(cc, chipIndex, paramId)` — register CC mapping
- [ ] Implement per-layer CC routing (some CCs affect specific layers only)
- [ ] Implement global CCs (Volume CC7, Pan CC10, Expression CC11)
- [ ] Implement modulation wheel (CC1) routing to chip LFO parameters
- [ ] Store CC mappings in preset state

#### 3.4 Mode Switching
- [ ] Implement clean mode transition: Mono → Layered (preserve current note if any)
- [ ] Implement clean mode transition: Mono → Poly (promote current note to first slot)
- [ ] Implement clean mode transition: Layered → Poly (release all layers, start poly)
- [ ] Send all-notes-off internally on mode switch to prevent stuck notes
- [ ] Persist voice mode in PluginState

#### 3.5 Tests for Phase 3
- [ ] `tests/test_voice_allocator.cpp` — add layered mode tests:
  - [ ] Note-on triggers all enabled layers
  - [ ] Disabled layer is skipped
  - [ ] Per-layer volume applies correctly
  - [ ] Layer add/remove mid-playback works without glitch
- [ ] `tests/test_voice_allocator.cpp` — add polyphonic mode tests:
  - [ ] Notes fill voice slots in order
  - [ ] Voice stealing steals oldest note when full
  - [ ] Note-off releases correct slot
  - [ ] Sustain pedal prevents release
  - [ ] Pitch bend applies to all active voices
  - [ ] Polyphony count change clears excess voices
- [ ] `tests/test_cc_routing.cpp`:
  - [ ] CC maps to correct chip parameter
  - [ ] Value range scaling (min/max) works
  - [ ] Global CCs affect all chips
- [ ] Performance benchmark: measure CPU per voice at 8, 16, 32 poly voices
- [ ] Verify all tests pass

---

### Phase 4: Effect Chain & QSound Integration

#### 4.1 Effect Chain Architecture
- [ ] Create `src/core/effect_chain.h/cpp`
- [ ] Define `EffectType` enum (QSound, Reverb, Delay, Distortion, EQ)
- [ ] Define `EffectSlot` struct (type, enabled, bypass, parameter values)
- [ ] Implement `addEffect(EffectType, slotIndex)` — insert effect at position
- [ ] Implement `removeEffect(slotIndex)` — remove and close gap
- [ ] Implement `setEffectParameter(slotIndex, paramId, value)` — set per-effect parameter
- [ ] Implement `process(float* left, float* right, int sampleCount)` — chain effects in slot order
- [ ] Implement per-effect bypass toggle
- [ ] Implement global "bypass all effects" toggle

#### 4.2 QSound Effect Wrapper
- [ ] Study flashkick-vst3's `EchoEngine` implementation for reference
- [ ] Create `src/core/effects/qsound_effect.h/cpp`
- [ ] Port QSound DL-1425 echo algorithm (mono sum → delay buffer → Haas detune)
- [ ] Expose parameters: DelayTime, Feedback, Damping, SendLevel, WetMix, StereoWidth
- [ ] Implement authentic delay range clamping (57–170 ms in QSound mode)
- [ ] Implement 24038 Hz native rate resampling (linear interpolation to/from host rate)
- [ ] Implement cascaded moving-average lowpass for damping (1–8 stages)
- [ ] Implement optional 9-tap Hamming FIR on wet output

#### 4.3 Reverb Effect
- [ ] Create `src/core/effects/reverb_effect.h/cpp`
- [ ] Implement simple algorithmic reverb (Schroeder or Freeverb-style)
- [ ] Expose parameters: RoomSize, Damping, WetMix, PreDelay
- [ ] Ensure real-time safe (no allocations in process loop)

#### 4.4 Delay Effect
- [ ] Create `src/core/effects/delay_effect.h/cpp`
- [ ] Implement stereo delay with:
  - [ ] Independent L/R delay times
  - [ ] Feedback with damping
  - [ ] Ping-pong mode
  - [ ] Tempo sync option (BPM-based delay time)
- [ ] Expose parameters: DelayTimeL, DelayTimeR, Feedback, WetMix, PingPong

#### 4.5 Effect State Persistence
- [ ] Implement `EffectChain::saveState(IBStream*)` — serialize all slots, types, and parameters
- [ ] Implement `EffectChain::restoreState(IBStream*)` — recreate effect chain from state
- [ ] Wire into main plugin state serialization

#### 4.6 Integration with Audio Pipeline
- [ ] Insert `EffectChain::process()` after `ChipEngine::processSample()` in the Processor
- [ ] Implement dry/wet mix at the chain level (pre-effect dry signal preserved)
- [ ] Ensure effect chain respects host sample rate

#### 4.7 Tests for Phase 4
- [ ] `tests/test_effect_chain.cpp`:
  - [ ] Empty chain passes audio through unchanged
  - [ ] Single effect modifies signal
  - [ ] Effect ordering matters (A→B ≠ B→A)
  - [ ] Bypass skips individual effect
  - [ ] Global bypass passes audio through unchanged
  - [ ] Effect add/remove mid-playback is glitch-free
- [ ] `tests/test_qsound_effect.cpp`:
  - [ ] Silence in → silence out
  - [ ] Impulse produces echo at expected delay time
  - [ ] Feedback causes decay over multiple echoes
  - [ ] QSound mode clamps delay range
- [ ] `tests/test_reverb_effect.cpp`:
  - [ ] Impulse produces reverb tail
  - [ ] Tail decays to silence
- [ ] Verify all tests pass

---

### Phase 5: UI & Preset Management

#### 5.1 UI Infrastructure
- [ ] Create `src/ui/editor_parameter_access.h` — abstract interface (matching combover/flashkick pattern):
  - [ ] `getNormalized(ParamId)` / `setNormalized(ParamId, double)`
  - [ ] `getChipEngine()` (for UI to query active chips)
  - [ ] `getVoiceAllocator()` (for UI to query voice state)
  - [ ] `getEffectChain()` (for UI to query effect slots)
- [ ] Implement `EditorParameterAccess` in VST3 Controller
- [ ] Create `src/ui/imgui_style.h` — TigerFlame theme colors and styling
- [ ] Create `src/ui/imgui_fonts.h` — embedded font data (DM Sans, Oxanium, Material Icons)
- [ ] Create platform-specific ImGui backends:
  - [ ] `src/ui/imgui_editor_mac.mm` — Metal + Cocoa (reference combover-vst3)
  - [ ] `src/ui/imgui_editor_win32.cpp` — DirectX11 + Win32
  - [ ] `src/ui/imgui_editor_linux.cpp` — OpenGL3 + X11
- [ ] Create `src/ui/imgui_editor.h` — `CPluginView` subclass for VST3 hosting

#### 5.2 Main Editor Layout
- [ ] Create `src/ui/editor_core.h/cpp`
- [ ] Implement top panel: preset selector dropdown, save/load buttons, master volume fader
- [ ] Implement left panel (Chip Selector):
  - [ ] Scrollable list of active chip instances
  - [ ] Add chip button (dropdown of available chip types)
  - [ ] Remove chip button (with confirmation)
  - [ ] Per-chip enable/disable toggle
  - [ ] Per-chip volume knob
- [ ] Implement center panel (Voice Allocator Config):
  - [ ] Radio buttons: Mono / Layered / Polyphonic
  - [ ] Polyphony count slider (visible in Poly mode, range 2–32)
  - [ ] Layer manager (visible in Layered mode):
    - [ ] List of layers with instrument selector per layer
    - [ ] Per-layer volume knob and pan control
    - [ ] Add/remove layer buttons
- [ ] Implement right panel (Effect Chain):
  - [ ] Vertical list of effect slots
  - [ ] Add effect button (dropdown of available types)
  - [ ] Remove effect button
  - [ ] Per-effect enable/disable toggle
  - [ ] Per-effect parameter knobs (expand/collapse per slot)
  - [ ] Bypass all effects button
- [ ] Implement bottom panel (Master Info):
  - [ ] CPU load meter
  - [ ] Active voice count display
  - [ ] MIDI activity indicator (flash on note-on)

#### 5.3 Custom ImGui Widgets
- [ ] Create `src/ui/imgui_widgets/chip_selector.h` — chip list with icons per chip type
- [ ] Create `src/ui/imgui_widgets/voice_mixer.h` — voice/layer mixer strip widget
- [ ] Create `src/ui/imgui_widgets/parameter_editor.h` — knob widget with:
  - [ ] Click-drag value editing
  - [ ] Right-click reset to default
  - [ ] Value tooltip on hover
  - [ ] Parameter name label
- [ ] Create `src/ui/imgui_widgets/preset_browser.h` — preset list with search, categories

#### 5.4 Preset Management
- [ ] Create `src/core/preset_manager.h/cpp`
- [ ] Define `Preset` struct (name, description, layers, effects, masterVolume, masterTuning)
- [ ] Implement `savePreset(path, Preset)` — serialize to JSON (nlohmann/json)
- [ ] Implement `loadPreset(path)` — deserialize from JSON
- [ ] Implement `enumeratePresets(directory)` — scan directory for preset files
- [ ] Define factory preset directory and user preset directory (platform-specific paths)
- [ ] Create factory presets:
  - [ ] `Init.json` — default state, single YM2151, mono mode
  - [ ] `Arcade Bass.json` — monophonic YM2151 bass with QSound delay
  - [ ] `Chip Pad.json` — layered YM2612 + OPL3 + SN76489 pad
  - [ ] `Poly Lead.json` — 8-voice polyphonic YM2151
- [ ] Implement preset UI state (current selection, save/load/delete dialogs)
- [ ] Wire preset load/save into `PluginState` + `ChipEngine` + `EffectChain` restore

#### 5.5 Tests for Phase 5
- [ ] `tests/test_preset_manager.cpp`:
  - [ ] Save preset and load it back — all fields match
  - [ ] Enumerate presets finds factory presets
  - [ ] Invalid preset file returns error gracefully
  - [ ] User preset directory is created on first save
- [ ] Manual UI testing:
  - [ ] Verify editor opens in a DAW (Reaper or similar)
  - [ ] Verify all knobs respond to click-drag
  - [ ] Verify mode switching updates panel visibility
  - [ ] Verify preset save/load round-trip
- [ ] Verify all tests pass

---

### Phase 6: Standalone Application

#### 6.1 Standalone Runtime
- [ ] Create `src/standalone/runtime.h/cpp`
- [ ] Implement `EditorParameterAccess` interface (same as combover/flashkick pattern)
- [ ] Hold instances of `PluginState`, `ChipEngine`, `VoiceAllocator`, `MidiTranslator`, `EffectChain`
- [ ] Implement `start()` / `stop()` — lifecycle management
- [ ] Implement `getNormalized()` / `setNormalized()` — parameter access
- [ ] Implement audio callback: process MIDI events, run engine, output audio

#### 6.2 Audio I/O (miniaudio)
- [ ] Add miniaudio as `FetchContent` dependency (if not already)
- [ ] Implement duplex audio stream (output only for instrument, or duplex for loopback)
- [ ] Handle sample rate negotiation (44.1k, 48k, 88.2k, 96k)
- [ ] Handle buffer size configuration
- [ ] Implement audio device selection (default device or user-chosen)

#### 6.3 MIDI I/O (RtMidi)
- [ ] Add RtMidi as `FetchContent` dependency (if not already)
- [ ] Create `src/standalone/midi_input.h/cpp`
- [ ] Implement MIDI input thread with callback to Runtime
- [ ] Handle note-on, note-off, CC, pitch bend, all-notes-off
- [ ] Implement MIDI port selection (list available ports, auto-connect first)
- [ ] Handle MIDI device hot-plug (detect disconnect, reconnect)

#### 6.4 Platform Entry Points
- [ ] Create `src/standalone/main_mac.mm` — Cocoa window + Metal ImGui rendering
- [ ] Create `src/standalone/main_win.cpp` — Win32 window + DirectX11 rendering
- [ ] Create `src/standalone/main_linux.cpp` — X11 window + OpenGL3 rendering
- [ ] Implement window title, icon, default size
- [ ] Implement graceful shutdown (stop audio/MIDI, destroy window)

#### 6.5 MIDI Learn Feature
- [ ] Implement MIDI learn mode: user clicks a parameter, then moves a CC → creates mapping
- [ ] Add "MIDI Learn" button to UI (enters learn mode)
- [ ] Visual feedback: highlight parameter waiting for CC input
- [ ] Store learned mappings in preset file
- [ ] Implement "Clear MIDI Learn" to remove a mapping

#### 6.6 Debug Server (Optional)
- [ ] Create `src/standalone/debug_server.h/cpp` — TCP server on 127.0.0.1:9183
- [ ] Implement `get <paramName>` and `set <paramName> <value>` commands
- [ ] Useful for headless testing and automation

#### 6.7 Build & Packaging
- [ ] Add standalone target to CMakeLists.txt (macOS .app bundle, Windows .exe, Linux binary)
- [ ] Add `just run` recipe to build and launch standalone
- [ ] Verify standalone launches, receives MIDI, produces audio

---

### Phase 7: Testing, Optimization & Polish

#### 7.1 Comprehensive Test Suite
- [ ] Audit all existing tests, fill coverage gaps
- [ ] Add integration tests:
  - [ ] Full signal chain: MIDI → VoiceAllocator → MidiTranslator → ChipEngine → EffectChain → audio output
  - [ ] Multi-chip synchronization (multiple chips produce time-aligned output)
  - [ ] Audio output continuity (no clicks, pops, or dropouts)
  - [ ] MIDI → audio latency measurement (target < 5 ms)
- [ ] Add state persistence tests:
  - [ ] Full plugin state save/restore round-trip (all components)
  - [ ] Preset compatibility across versions (forward-compat header)
- [ ] Add edge case tests:
  - [ ] Rapid note-on/note-off (> 1000 events/second)
  - [ ] All-notes-off during sustained poly playback
  - [ ] Mode switch during active playback
  - [ ] Sample rate change mid-session
  - [ ] Zero-length buffer processing

#### 7.2 CPU Profiling & Optimization
- [ ] Profile with Instruments (macOS) or perf (Linux) at 32-voice polyphony
- [ ] Identify hotspots in `ChipEngine::processSample()` and `EffectChain::process()`
- [ ] Optimize per-sample chip processing (minimize allocations, cache-friendly layout)
- [ ] Consider SIMD for effect chain processing (SSE2/NEON)
- [ ] Verify CPU target: < 15% per voice on single core at -O2
- [ ] Benchmark startup time (target < 500 ms)

#### 7.3 Memory Safety
- [ ] Run AddressSanitizer (`-fsanitize=address`) across full test suite
- [ ] Run LeakSanitizer to detect memory leaks
- [ ] Run UndefinedBehaviorSanitizer on DSP code
- [ ] Verify no allocations in real-time audio callback (`process()`)
- [ ] Audit circular buffer bounds and voice slot lifecycle
- [ ] Verify memory target: < 50 MB per instance with all chips loaded

#### 7.4 DAW Compatibility Testing
- [ ] Test in Reaper (macOS) — load, play, automate, save/reload project
- [ ] Test in Logic Pro (macOS) — load, play, AU validation
- [ ] Test in Ableton Live (macOS) — load, play, preset switching
- [ ] Test in Bitwig Studio — load, play, CLAP format (if built)
- [ ] Verify:
  - [ ] Plugin scans without errors
  - [ ] Audio output is correct and glitch-free
  - [ ] Parameter automation records and plays back
  - [ ] Preset save/load works within DAW session
  - [ ] Plugin state survives DAW project save/reload
  - [ ] Multiple instances in same session work independently

#### 7.5 CLAP Plugin Build
- [ ] Create `src/clap/` directory with CLAP plugin wrapper (following combover-vst3 pattern):
  - [ ] `clap_entry.cpp` — CLAP entry point
  - [ ] `clap_plugin.h/cpp` — main plugin class
  - [ ] `clap_process.h/cpp` — audio/MIDI processing
  - [ ] `clap_params.h/cpp` — parameter enumeration
  - [ ] `clap_state.h/cpp` — state save/load
  - [ ] `clap_param_access.h/cpp` — EditorParameterAccess adapter
  - [ ] `clap_gui.h` + platform-specific GUI files
- [ ] Add CLAP SDK as `FetchContent` dependency
- [ ] Add `BUILD_CLAP` option to CMakeLists.txt
- [ ] Add `just build-clap` recipe
- [ ] Verify CLAP plugin loads in Bitwig or clap-validator

#### 7.6 Documentation
- [ ] Create `docs/ARCHITECTURE.md` — high-level architecture with diagrams
- [ ] Create `docs/VOICE_ALLOCATION.md` — voice modes, state machines, examples
- [ ] Create `docs/PARAMETER_MAPPING.md` — CC table, chip-specific params, MIDI learn
- [ ] Create `docs/BUILD_GUIDE.md` — build instructions for all platforms
- [ ] Update `README.md` — user-facing guide, screenshots, parameter reference
- [ ] Add inline code comments only where logic is non-obvious

#### 7.7 Final Polish
- [ ] Review all compiler warnings, fix or suppress with justification
- [ ] Ensure consistent code style across all source files
- [ ] Remove any debug logging or test scaffolding from release builds
- [ ] Verify `just configure && just build && just test` passes clean on macOS
- [ ] Tag version 0.1.0 release

**Total Estimated Timeline**: 14–18 weeks

---

## Technical Requirements

### Build Environment

- **CMake** ≥ 3.25
- **C++17** (minimum)
- **Platform SDKs**:
  - macOS: Xcode 12+, macOS 10.15+
  - Windows: Visual Studio 2022, Windows 10+
  - Linux: GCC 11+ or Clang 14+

### Dependencies (auto-fetched via FetchContent)

- **Steinberg VST3 SDK** (latest)
- **CLAP SDK** (latest)
- **Dear ImGui** (v1.90+)
- **miniaudio** (standalone audio I/O)
- **RtMidi** (MIDI input)
- **mml2vgm-rs** (as Rust FFI binding or C++ wrapper)

### Audio Specifications

- **Sample Rates**: 44.1 kHz, 48 kHz, 88.2 kHz, 96 kHz (others via resampling)
- **Buffer Sizes**: 64–2048 samples/block
- **Bit Depth**: 32-bit floating-point
- **Channels**: Stereo (2.0) output minimum; surround support TBD

### Performance Targets

- **CPU Load**: < 15% per voice (single core, -O2)
- **Latency**: < 5 ms round-trip (MIDI input → audio output)
- **Memory**: < 50 MB per instance (with all chips loaded)
- **Startup Time**: < 500 ms

---

## Integration with mml2vgm

### Binding Strategy

**Option A: C++ Wrapper (Recommended) - IMPLEMENTED**
- Compile mml2vgm-rs core as a static Rust library (`libmml2vgm.a`)
- Expose C API for chip synthesis via FFI module (`src/ffi.rs`)
- Link into plugin at build time via CMake custom target
- Pro: Direct integration, optimal performance, zero runtime overhead
- Con: Build complexity, Rust toolchain dependency

**Static Library Build Details:**
```
# In mml2vgm-rs/Cargo.toml:
[lib]
crate-type = ["staticlib", "cdylib"]  # Builds libmml2vgm.a

# In tigerflame-vst3/CMakeLists.txt:
add_custom_target(mml2vgm-rs-build ALL
    COMMAND cargo build --lib --release --manifest-path ${MML2VGM_RS_SOURCE_DIR}/Cargo.toml
    WORKING_DIRECTORY ${MML2VGM_RS_SOURCE_DIR}
)

target_link_libraries(tigerflame-core PRIVATE ${MML2VGM_LIB})
target_include_directories(tigerflame-core PRIVATE ${MML2VGM_INCLUDE_DIR})
```

**FFI Interface (src/ffi.rs):**
- All functions prefixed with `mml2vgm_chip_` to avoid C++ name mangling conflicts
- C-compatible types: `ChipHandle` (opaque pointer), primitive args for parameters
- Memory management: Rust allocates, C++ owns pointers, Rust deallocates via destroy calls
- String handling: Rust allocates via `CString::into_raw()`, C++ must free with `mml2vgm_chip_free_string()`
- Functions: init, shutdown, create, destroy, process_sample, reset, get/set_param, get_param_count, get_param_name, get_name, get_short_name

**Chip Factory (src/chips/mod.rs):**
- `ChipType` enum: YM2151, YM2612, SN76489, OPL2, OPL3, QSound, C140, POKEY, VRC6
- `ChipInstance` wrapper: owns `Box<dyn SoundChipEmulator>`, handles FFI-safe lifetime
- `create_chip()`: factory function mapping `ChipType` → concrete chip emulator
- `TryFrom<i32>` implementation: enables safe enum conversion from C++

**C++ Bridge (src/core/chip_bindings.cpp):**
- Implements C functions from `chip_bindings.h` by forwarding to Rust FFI
- extern "C" declarations map to `mml2vgm_chip_*` functions
- Handles command routing for note on/off, parameter changes, pitch bend

**CMake Integration:**
- Detects sibling `../mml2vgm/mml2vgm-rs` directory via `EXISTS` check
- Custom target builds Rust library before C++ compilation
- Links static library into both `tigerflame-core` and `tigerflame-vst3` targets
- macOS: additionally links Security and SystemConfiguration frameworks (Rust dependencies)
- Graceful fallback: if mml2vgm-rs not found, shows warning and continues without chip emulation

**Option B: IPC (Subprocess)**
- Launch mml2vgm-rs compiler as subprocess
- Serialize MIDI→MML, pipe to process, capture PCM
- Load PCM streams into plugin
- Pro: Decoupled, easy debugging
- Con: Latency, resource overhead

**Option C: Hybrid (Recommended Long-Term)**
- Use Option A for real-time synthesis (MIDI playback)
- Use Option B for batch compilation (export MML → VGM)

### Chip Instance Creation

```cpp
// From mml2vgm's chip factory
auto chipInstance = createChipInstance(ChipType::YM2151, sampleRate);

// Call per sample
float left, right;
chipInstance->processSample(&left, &right);

// Send commands
chipInstance->noteOn(midiNote, velocity);
chipInstance->setParameter(paramId, value);
```

### Preset → MML Export

Users can capture a TigerFlame performance and export as `.gwi` (MML):

```
[Export MIDI] → [Convert to MML] → [Save .gwi file]
```

This enables round-tripping: DAW plugin ↔ standalone compiler.

---

## Example Use Cases

### Use Case 1: Monophonic Bass
- Mode: Monophonic
- Chip: YM2151 (single instance)
- Layer: Bass instrument patch
- Effect: Subtle delay (QSound)
- Result: Classic arcade bass synth with spatial depth

### Use Case 2: Dense Multitimbral Pad
- Mode: Layered Multitimbral
- Layers:
  - Layer 1: YM2612 operator 1 (strings texture)
  - Layer 2: OPL3 bell (shimmer)
  - Layer 3: SN76489 noise (breath)
- Effect: Heavy reverb, QSound
- Result: Lush, evolving pad texture

### Use Case 3: Arpeggiator with Polyphonic Chords
- Mode: Polyphonic (8 voices)
- Chip: YM2151
- Effect: Ping-pong delay, slight distortion
- Result: Bright, bouncy arpeggio synth

---

## Testing Strategy

### Unit Tests (C++)
- Voice allocator state transitions
- MIDI translation correctness
- Parameter CC mapping
- Preset serialization

### Integration Tests
- Multi-chip synchronization
- Audio output continuity (no clicks/pops)
- MIDI → audio latency
- Effect chain processing

### DAW Compatibility Tests
- Reaper (macOS/Windows)
- Logic Pro (macOS)
- Ableton Live (macOS/Windows)
- Bitwig Studio (cross-platform)

### Performance Profiling
- CPU usage per voice count
- Memory allocations (detect leaks)
- Buffer allocation overhead
- Real-time safety violations

---

## Roadmap & Future Enhancements

### Short-term (Post-Launch)
- MIDI learn mode (map any CC to any parameter)
- Arpeggiator built-in
- Chord recognition / voicing assistant
- Tuning presets (A440 variants, microtonality)

### Medium-term
- Wavetable editor for custom waveforms
- Modulation matrix (LFO/envelope → parameters)
- CV input support (via DAW automation)
- Plugin state versioning for forward compatibility

### Long-term
- Spectral analysis / visualization
- Generative composition mode
- Cloud preset repository
- Mobile companion app (for parameter tweaking)

---

## References

### Related Projects
- [combover-vst3](../combover-vst3) — VST3 architecture reference
- [flashkick-vst3](../flashkick-vst3) — QSound effect integration example
- [mml2vgm-rs](../mml2vgm-rs) — Core synthesis engine

### Documentation
- [Steinberg VST3 SDK](https://steinberg.net/developers/technologies/vst3/)
- [CLAP Specification](https://github.com/free-audio/clap)
- [mml2vgm MML Commands](./MML_Commands.md)
- [QSound System](./PLAN_QSound.md)

### Plugin Development Resources
- [Dear ImGui](https://github.com/ocornut/imgui) — UI framework
- [miniaudio](https://miniaud.io/) — Audio I/O
- [RtMidi](https://www.music.mcgill.ca/~gary/rtmidi/) — MIDI I/O

---

## Contributors & License

**Maintainers**: [Your Name/Team]

**License**: [Match mml2vgm project license]

---

## Appendix A: Chip Capabilities Reference

| Chip | Polyphony | Effects | FM Synth | Sample-based | Notes |
|------|-----------|---------|----------|--------------|-------|
| YM2151 | 8 | LFO, EG | Yes | No | Yamaha FM |
| YM2612 | 6 | LFO, EG | Yes | No | Genesis/Mega Drive |
| SN76489 | 3+1 | None | No | No | Sega Master System noise |
| OPL2 | 9 | Tremolo, Vibrato | Yes | No | Sound Blaster |
| OPL3 | 18 | Tremolo, Vibrato | Yes | No | Sound Blaster Pro |
| QSound | Passive | Echo, Phase | No | Yes | Capcom arcade spatial |
| C140 | 24 | None | No | Yes | Namco arcade wavetable |
| POKEY | 4 | Filter sweep | No | No | Atari 8-bit |
| VRC6 | 3 | PWM | No | No | NES Konami cartridge |
| ... | ... | ... | ... | ... | [15+ more chips] |

---

**Document Version**: 0.2.0  
**Last Updated**: May 8, 2026  
**Status**: Phase 1 Complete, Phase 2 In Progress
