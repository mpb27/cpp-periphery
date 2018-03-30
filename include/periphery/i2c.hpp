/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 *
 * Notes:
 *   1) Don't throw from destructor:
 *          https://stackoverflow.com/questions/130117
 *   2) RAII example :
 *          https://www.tomdalling.com/blog/software-design/resource-acquisition-is-initialisation-raii-explained/
 *   3) Deleteing copy constructor in C++11:
 *          https://stackoverflow.com/questions/33776697
 */

#ifndef PERIPHERY_I2C_HPP
#define PERIPHERY_I2C_HPP


#include <array>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <system_error>
#include <vector>

namespace periphery {

class I2C {
public:
    class Message;  // ... forward define ...

    // ... constructor / destructor ...
    I2C(const std::string& path);
    ~I2C();
    // ... disable copy-constructor and copy assignment ...
    I2C(const I2C&) = delete;
    I2C& operator=(const I2C&) = delete;

    // ... primary functions ...
    template <class... Messages>
    void transfer(uint16_t addr, Messages&&... messages) const;

    template <typename ForwardIt>
    void transfer(uint16_t addr, ForwardIt first, ForwardIt last, typename std::iterator_traits<ForwardIt>::iterator_category* = nullptr) const;

    void transfer(uint16_t addr, std::initializer_list<std::reference_wrapper<Message>> messages) const;

    // ... extra ...
    std::string toString() const;

    class Message {
    public:

        static Message write(std::initializer_list<uint8_t> tx_data);  // has to be int to detect bad values
        static Message write(const std::vector<uint8_t>& tx_data);
        static Message read(size_t len);

        std::vector<uint8_t> data;

    private:
        uint16_t flags;
        Message() { };
        friend class I2C;
    };
private:
    int m_fd;
    std::string m_path;
    template <typename ForwardIt>
    void transfer_impl(uint16_t addr, ForwardIt first, ForwardIt last) const;
    void transfer(uint16_t addr, std::vector<std::reference_wrapper<Message>>& messages) const;
};


template <class... Messages>
inline void I2C::transfer(uint16_t addr, Messages&&... messages) const
{
    transfer(addr, { messages... });
}


template <typename ForwardIt>
inline void I2C::transfer(uint16_t addr, ForwardIt first, ForwardIt last,
    typename std::iterator_traits<ForwardIt>::iterator_category*) const
{
    // traverse the container and get a vector of references to messages
    std::vector<std::reference_wrapper<Message>> v;
    for (auto it = first; it != last; ++it) {
        v.push_back(std::ref(*it));
    }

    transfer(addr, v);
}


} // ... namespace periphery ...


#endif  // PERIPHERY_I2C_HPP
