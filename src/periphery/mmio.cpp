
// C++11 includes:
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <system_error>

// POSIX 2008 Headers:
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "periphery/mmio.hpp"

namespace periphery {

    Mmio::Mmio(uintptr_t base, size_t size) {
        m_base = base;
        m_size = size;
        m_aligned_base = base - (base % sysconf(_SC_PAGESIZE));
        m_aligned_size = size + (base - m_aligned_base);

        // ... open memory ...
        int fd = open("/dev/mem", O_RDWR, O_SYNC);
        if (fd < 0) {
            throw std::system_error(EFAULT, std::system_category());
        }

        // ... map memory ...
        m_ptr = static_cast<uint8_t*>(mmap(0, m_aligned_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, m_aligned_base));
        if (m_ptr  == MAP_FAILED) {
            auto e = std::system_error(EFAULT, std::system_category());
            close(fd);
            throw e;
        }

        // ... close memory ...
        int error = close(fd);
        if (error) {
            auto e = std::system_error(EFAULT, std::system_category());
            munmap(m_ptr, m_aligned_size);
            throw e;
        }
    }

    Mmio::~Mmio() {
        // ... unmap memory...
        if (munmap(m_ptr, m_aligned_size) < 0) {
            // ... never throw from destructor ...
        }
    }

    template<typename T> inline T Mmio::read(size_t offset) const {
        offset += (m_base - m_aligned_base);
        if ((offset + sizeof(T)) > m_aligned_size) {
            throw std::invalid_argument("offset out of bounds");
        }
        return *((volatile T*)(m_ptr + offset));
    }

    template<typename T> inline void Mmio::write(size_t offset, T value) const {
        offset += (m_base - m_aligned_base);
        if ((offset + sizeof(T)) > m_aligned_size) {
            throw std::invalid_argument("offset out of bounds");
        }
        *((volatile T*)(m_ptr + offset)) = value;
    }

    // TODO: Use either std::span (C++17?) or two pointers cbegin cend in C++ fashion.
    //       If using begin/end, consider using std::copy
    void Mmio::read(size_t offset, uint8_t* buf, size_t len) const {
        offset += (m_base - m_aligned_base);
        if ((offset + len) > m_aligned_size) {
            throw std::invalid_argument("read out of bounds");
        }
        memcpy(buf, m_ptr + offset, len);
    }

    void Mmio::write(size_t offset, const uint8_t* buf, size_t len) const {
        offset += (m_base - m_aligned_base);
        if ((offset + len) > m_aligned_size) {
            throw std::invalid_argument("write out of bounds");
        }
        memcpy(m_ptr + offset, buf, len);
    }


}
