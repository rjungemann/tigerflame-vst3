// TigerFlame - Core Stream Interface
// Abstract stream interface for state serialization
// Can be implemented with VST3 IBStream or standard file streams

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace TigerFlame {

// Result codes
enum class StreamResult : int32_t {
    kOk = 0,
    kError = -1,
    kNotImplemented = -2,
    kInvalidArgument = -3
};

// Abstract stream interface
class IStream {
public:
    virtual ~IStream() = default;
    
    // Write data to stream
    virtual StreamResult write(const void* buffer, size_t size) = 0;
    
    // Read data from stream
    virtual StreamResult read(void* buffer, size_t size) = 0;
    
    // Seek to position
    virtual StreamResult seek(int64_t position) = 0;
    
    // Get current position
    virtual int64_t tell() const = 0;
    
    // Get stream size
    virtual int64_t getSize() const = 0;
};

// Simple memory stream implementation for testing
class MemoryStream : public IStream {
public:
    MemoryStream(uint8_t* data, size_t capacity)
        : data_(data), capacity_(capacity), position_(0) {}
    
    StreamResult write(const void* buffer, size_t size) override {
        if (position_ + size > capacity_) {
            return StreamResult::kError;
        }
        if (buffer) {
            std::memcpy(data_ + position_, buffer, size);
        }
        position_ += size;
        return StreamResult::kOk;
    }
    
    StreamResult read(void* buffer, size_t size) override {
        if (buffer == nullptr && size > 0) {
            return StreamResult::kInvalidArgument;
        }
        if (position_ + size > capacity_) {
            return StreamResult::kError;
        }
        std::memcpy(buffer, data_ + position_, size);
        position_ += size;
        return StreamResult::kOk;
    }
    
    StreamResult seek(int64_t position) override {
        if (position < 0 || static_cast<size_t>(position) > capacity_) {
            return StreamResult::kError;
        }
        position_ = static_cast<size_t>(position);
        return StreamResult::kOk;
    }
    
    int64_t tell() const override {
        return static_cast<int64_t>(position_);
    }
    
    int64_t getSize() const override {
        return static_cast<int64_t>(capacity_);
    }
    
    uint8_t* getData() const { return data_; }
    size_t getCapacity() const { return capacity_; }
    size_t getPosition() const { return position_; }

private:
    uint8_t* data_;
    size_t capacity_;
    size_t position_;
};

} // namespace TigerFlame
