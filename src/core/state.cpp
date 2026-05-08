// TigerFlame - Core State Management Implementation

#include "state.h"
#include <algorithm>

namespace TigerFlame {

namespace StateUtil {

// Write a chunk header
void writeChunkHeader(IStream* stream, uint32_t magic, uint32_t version, uint32_t size) {
    stream->write(&magic, sizeof(magic));
    stream->write(&version, sizeof(version));
    stream->write(&size, sizeof(size));
}

// Read a chunk header
bool readChunkHeader(IStream* stream, uint32_t& magic, uint32_t& version, uint32_t& size) {
    if (stream->read(&magic, sizeof(magic)) != StreamResult::kOk) {
        return false;
    }
    if (stream->read(&version, sizeof(version)) != StreamResult::kOk) {
        return false;
    }
    if (stream->read(&size, sizeof(size)) != StreamResult::kOk) {
        return false;
    }
    return true;
}

// Skip a chunk
void skipChunk(IStream* stream, uint32_t size) {
    uint8_t buffer[256];
    while (size > 0) {
        uint32_t toRead = std::min(size, uint32_t(sizeof(buffer)));
        if (stream->read(buffer, toRead) != StreamResult::kOk) {
            break;
        }
        size -= toRead;
    }
}

} // namespace StateUtil

} // namespace TigerFlame
