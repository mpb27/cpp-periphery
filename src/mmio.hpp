/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 *
 * Notes:
 *   1) Why use size_t for offset : https://stackoverflow.com/questions/10634629/
 */

#ifndef PERIPHERY_MMIO_HPP
#define PERIPHERY_MMIO_HPP

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <system_error>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

namespace periphery {

class mmio {
public:
    // .. Constructor / Destructor ...
    mmio(uintptr_t base, size_t size);
    ~mmio();
    // ... disable copy-constructor and copy assignment ...
    mmio(const mmio&) = delete;
    mmio& operator=(const mmio&) = delete;

    // ... reads ...
    uint32_t read32(size_t offset) const { return read<uint32_t>(offset); }
    uint16_t read16(size_t offset) const { return read<uint16_t>(offset); }
    uint8_t  read8 (size_t offset) const { return read<uint8_t >(offset); }
    void     read(size_t offset, uint8_t* buf, size_t len) const;
    // ... writes ...
    void     write32(size_t offset, uint32_t value) const { write<uint32_t>(offset, value); }
    void     write16(size_t offset, uint16_t value) const  { write<uint16_t>(offset, value); }
    void     write8(size_t offset, uint8_t value) const  { write<uint8_t>(offset, value); }
    void     write(size_t offset, const uint8_t* buf, size_t len) const;
    // ... clear and set (non-atomic) ...
    void     clear32(size_t offset, uint32_t mask) const { write32(offset, read32(offset) & ~mask); }
    void     clear16(size_t offset, uint16_t mask) const { write16(offset, read16(offset) & ~mask); }
    void     clear8 (size_t offset, uint8_t  mask) const { write8(offset, read8(offset) & ~mask); }
    void     set32(size_t offset, uint32_t mask) const { write32(offset, read32(offset) | mask); }
    void     set16(size_t offset, uint16_t mask) const { write16(offset, read16(offset) | mask); }
    void     set8 (size_t offset, uint8_t  mask) const { write8(offset, read8(offset) | mask); }

private:
    uintptr_t base;
    uintptr_t aligned_base;
    size_t size;
    size_t aligned_size;
    uint8_t* ptr;
    // ... read/write templates, private for now, but could be made public
    //     with some static_asserts to make sure it is being used properly ...
    template<typename T> T    read (size_t offset) const;
    template<typename T> void write(size_t offset, T value) const;
};

mmio::mmio(uintptr_t base, size_t size) {
    int fd;
    this->base = base;
    this->size = size;
    this->aligned_base = base - (base % sysconf(_SC_PAGESIZE));
    this->aligned_size = size + (base - this->aligned_base);

    // ... open memory ...
    if ((fd = open("/dev/mem", O_RDWR, O_SYNC)) < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }

    // ... map memory ...
    this->ptr = static_cast<uint8_t*>(mmap(0, this->aligned_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, this->aligned_base));
    if (this->ptr  == MAP_FAILED) {
        auto e = std::system_error(EFAULT, std::system_category());
        close(fd);
        throw e;
    }

    // ... close memory ...
    if (close(fd) < 0) {
        auto e = std::system_error(EFAULT, std::system_category());
        munmap(this->ptr, this->aligned_size);
        throw e;
    }
}

mmio::~mmio() {
    // ... unmap memory...
    if (munmap(this->ptr, this->aligned_size) < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }
}

template<typename T> inline T mmio::read(size_t offset) const {
    offset += (this->base - this->aligned_base);
    if ((offset + sizeof(T)) > this->aligned_size) {
        throw std::invalid_argument("offset out of bounds");
    }
    return *((volatile T*)(this->ptr + offset));
}

template<typename T> inline void mmio::write(size_t offset, T value) const {
    offset += (this->base - this->aligned_base);
    if ((offset + sizeof(T)) > this->aligned_size) {
        throw std::invalid_argument("offset out of bounds");
    }
    *((volatile T*)(this->ptr + offset)) = value;
}

void mmio::read(size_t offset, uint8_t* buf, size_t len) const {
    offset += (this->base - this->aligned_base);
    if ((offset + len) > this->aligned_size) {
        throw std::invalid_argument("read out of bounds");
    }
    memcpy(buf, this->ptr + offset, len);
}

void mmio::write(size_t offset, const uint8_t* buf, size_t len) const {
    offset += (this->base - this->aligned_base);
    if ((offset + len) > this->aligned_size) {
        throw std::invalid_argument("write out of bounds");
    }
    memcpy(this->ptr + offset, buf, len);
}


} // ... namespace periphery ...

#endif
