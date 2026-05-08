# TigerFlame VST3

A VST3/CLAP plugin for mml2vgm-compatible sound chip instruments.

## Features

- VST3 and CLAP plugin formats
- Sound chip emulation compatible with mml2vgm
- Real-time audio processing

## Requirements

- C++20 compatible compiler
- CMake 3.25+

## Building

```bash
# Clone the repository
git clone https://github.com/rjungemann/tigerflame-vst3.git
cd tigerflame-vst3

# Create and enter build directory
mkdir -p build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release
```

## Installation

After building, the VST3 plugin bundle will be located in:
- macOS: `build/tigerflame.vst3/`
- Windows: `build/tigerflame.vst3/`
- Linux: `build/tigerflame.vst3/`

Copy the bundle to your DAW's VST3 plugin directory.

## License

MIT License - see LICENSE file for details.
