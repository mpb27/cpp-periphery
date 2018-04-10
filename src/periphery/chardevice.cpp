/*
 * cpp-periphery
 * https://github.com/mpb27/cpp-periphery
 * License: MIT
 */

//http://www.cmrr.umn.edu/~strupp/serial.html

// C++11
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

// POSIX 2008 Headers:
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>

// Linux:
#include <sys/ioctl.h>

#include "periphery/periphery.hpp"
#include "periphery/chardevice.hpp"


namespace periphery {


CharacterDevice::CharacterDevice(const std::string& path, Access access)
{
    int oflag = 0;
    
    // parse access
    switch (access) {
    case Access::ReadOnly  : oflag |= O_RDONLY; break;
    case Access::WriteOnly : oflag |= O_WRONLY; break;
    case Access::ReadWrite : oflag |= O_RDWR;   break;
    default                : throw std::invalid_argument("access invalid");
    }
    
    // set no delay, and no ctty?
    oflag |= O_NOCTTY | O_NDELAY;
    
    // open
    m_fd = ::open(path.c_str(), oflag);
    if (m_fd < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }
}


CharacterDevice::~CharacterDevice()
{
    int error = ::close(m_fd);
    if (error < 0) {
        // ... can't throw in destructor, maybe log an error? ...
    }

}

unsigned int CharacterDevice::input_waiting() const
{
    unsigned int count = 0;
    int error = ::ioctl(m_fd, TIOCINQ, &count);
    if (error < 0) {
        std::cerr << "\n\n\nInput waiting ERROR = " << error << std::endl << std::endl << std::endl;
        throw std::system_error(EFAULT, std::system_category());
    }
    return count;
}


unsigned int CharacterDevice::output_waiting() const
{
    unsigned int count = 0;
    int error = ::ioctl(m_fd, TIOCOUTQ, &count);
    if (error < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }
    return count;
}


bool CharacterDevice::poll(std::chrono::milliseconds timeout) const
{
    struct pollfd fds[1];

    // ... poll ...
    fds[0].fd = m_fd;
    fds[0].events = POLLIN | POLLPRI;
    int ret = ::poll(fds, 1, timeout.count());
    // ... above uses scope resolution operator :: ...
    // https://stackoverflow.com/a/7149954/953414

    // ... ret < 0 is an error, ret > 0 is success, ret == 0 is timeout ...
    if (ret < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }
    return (ret > 0);
}


void CharacterDevice::write(const_buffer buf) const
{
    while (buf.size() > 0) {
        ssize_t ret = ::write(m_fd, buf.data(), buf.size());
        if (ret < 0) {
            throw std::system_error(EFAULT, std::system_category());
        }

        // ... advance the buffer by the number of sent bytes ...
        buf += ret;
    }
}


int CharacterDevice::read(mutable_buffer buf) const
{
    ssize_t ret = ::read(m_fd, buf.data(), buf.size());
    if (ret < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }
    return ret;
}


void CharacterDevice::read_all(mutable_buffer buf) const
{
    while (buf.size() > 0)
    {
        int bytesRead = this->read(buf);
        buf += bytesRead;
    }
}


int CharacterDevice::read_timeout(periphery::mutable_buffer buf, std::chrono::milliseconds timeout) const
{
    ssize_t ret;

    // ... setup timeout ...
    struct timeval tv;
    tv.tv_sec = timeout.count() / 1000;
    tv.tv_usec = (timeout.count() % 1000) * 1000;

    // ... if there is a timeout, first make sure there are bytes available with a timeout that waits for
    //     bytes to be available ...
    if (timeout.count() >= 0) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_fd, &rfds);

        // TODO: Consider using poll() since we are waiting on only one file descriptor and poll takes milliseconds directly.
        ret = ::select(m_fd+1, &rfds, NULL, NULL, &tv);
        if (ret < 0) {
            throw std::system_error(EFAULT, std::system_category());
        }

        // ... timeout if ret is 0 ...
        if (ret == 0) {
            // ... timeout occurred ...
            return 0;
        }
    }

    // ... read what is available ...
    ret = ::read(m_fd, buf.data(), buf.size());
    if (ret < 0) {
        throw std::system_error(EFAULT, std::system_category());
    }

    return ret;
}


int CharacterDevice::read_all_timeout(periphery::mutable_buffer buf, std::chrono::milliseconds timeout) const
{
    ssize_t ret;
    auto original_size = buf.size();

    // ... setup timeout ...
    struct timeval tv;
    tv.tv_sec = timeout.count() / 1000;
    tv.tv_usec = (timeout.count() % 1000) * 1000;

    do {
        if (timeout.count() >= 0) {
            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(m_fd, &rfds);

            ret = ::select(m_fd + 1, &rfds, NULL, NULL, &tv);
            if (ret < 0) {
                throw std::system_error(EFAULT, std::system_category());
            }

            // ... timeout if ret is 0 ...
            if (ret == 0) {
                // ... timeout occurred ...
                break;
            }
        }

        ret = ::read(m_fd, buf.data(), buf.size());
        if (ret < 0) {
            throw std::system_error(EFAULT, std::system_category());
        }

        // ... advance the buffer by the number of received bytes ...
        buf += ret;

    } while (buf.size() > 0);

    return original_size - buf.size();
}


} // ... namespace periphery ...
