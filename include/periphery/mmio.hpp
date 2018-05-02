/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 *
 * Notes:
 *   1) Why use size_t for offset :
 *          https://stackoverflow.com/questions/10634629/
 *   2) Which exception should be throw :
 *          https://stackoverflow.com/questions/12171377/
 *   3) MMAP info:
 *          http://man7.org/linux/man-pages/man2/mmap.2.html
 *          https://stackoverflow.com/questions/45972/mmap-vs-reading-blocks
 *   4) Include headers:
 *          https://stackoverflow.com/a/2029106/953414
 */

#ifndef PERIPHERY_MMIO_HPP
#define PERIPHERY_MMIO_HPP

// C++11 includes:
#include <cstddef>
#include <cstdint>

namespace periphery {

class Mmio {
public:
    // ... constructor / destructor ...
    Mmio(uintptr_t base, size_t size);
    ~Mmio();
    // ... disable copy-constructor and copy assignment ...
    Mmio(const Mmio&) = delete;
    Mmio& operator=(const Mmio&) = delete;

    // ... reads ...
    uint32_t read32(size_t offset) const;
    uint16_t read16(size_t offset) const;
    uint8_t  read8 (size_t offset) const;
    void     read(size_t offset, uint8_t* buf, size_t len) const;
    // ... writes ...
    void     write32(size_t offset, uint32_t value) const;
    void     write16(size_t offset, uint16_t value) const;
    void     write8(size_t offset, uint8_t value) const;
    void     write(size_t offset, const uint8_t* buf, size_t len) const;
    // ... clear and set (non-atomic) ...
    void     clear32(size_t offset, uint32_t mask) const;
    void     clear16(size_t offset, uint16_t mask) const;
    void     clear8 (size_t offset, uint8_t  mask) const;
    void     set32(size_t offset, uint32_t mask) const;
    void     set16(size_t offset, uint16_t mask) const;
    void     set8 (size_t offset, uint8_t  mask) const;

    // ... pointer, should not be used after Mmio is destroyed, ensure using? ...
    void* ptr() const { return static_cast<void*>(m_ptr + (m_base - m_aligned_base)); }

private:
    uintptr_t m_base;
    uintptr_t m_aligned_base;
    size_t    m_size;
    size_t    m_aligned_size;
    uint8_t*  m_ptr;
    // ... read/write templates, private for now, but could be made public
    //     with some static_asserts to make sure it is being used properly ...
    template<typename T> T    read (size_t offset) const;
    template<typename T> void write(size_t offset, T value) const;
};



} // ... namespace periphery ...

#endif
