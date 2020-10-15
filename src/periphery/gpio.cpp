/*!
 * \package         cpp-periphery
 * \file            gpio.cpp
 * \author          Mark Butowski (github.com/mpb27)
 * \date            2020-10-13
 * \license         MIT
 *
 * <a href="GitHub"> https://github.com/mpb27/cpp-periphery </a>
 */

// C++11 headers:
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

// POSIX 2008 headers:
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>

// Linux headers:
#include <linux/gpio.h>
#include <sys/ioctl.h>

#include "periphery/gpio.hpp"

namespace periphery {

    auto to_request(GpioPin::Edge edge) -> unsigned long;
    auto to_request(GpioPin::Bias bias) -> unsigned long;
    auto to_request(GpioPin::Drive drive) -> unsigned long;
    auto to_request(GpioPin::Invert invert) -> unsigned long;
    auto to_request(GpioPin::Direction direction) -> unsigned  long;


    GpioChip::GpioChip(const std::string &path)
    {
        // ... open GPIO chip ...
        m_fd = open(path.c_str(), O_RDONLY);
        if (m_fd < 0) {
            throw std::system_error(EFAULT, std::system_category(), "failed to open GPIO chip");
        }

        // ... get chip info for number of lines ...
        struct gpiochip_info chip_info = {};
        if (ioctl(m_fd, GPIO_GET_CHIPINFO_IOCTL, &chip_info) < 0) {
            auto e = std::system_error(EFAULT, std::system_category());
            close(m_fd);
            throw e;
        }
    }

    GpioChip::~GpioChip() {
        close(m_fd);
        m_fd = -1;
    }


    auto GpioPin::get() const -> GpioPin::State
    {
        struct gpiohandle_data data = {};

        auto success = ioctl(m_fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
        if (success < 0)
        {
            //ec.assign(-success, std::system_category());
            //return GpioPin::State::Low;
            throw std::system_error(EFAULT, std::system_category(), "Failed to read pin state.");
        }

        return data.values[0] ? GpioPin::State::High : GpioPin::State::Low;
    }

    void GpioPin::set(State value)
    {
        struct gpiohandle_data data = {};

        data.values[0] = value == State::High ? 1 : 0;

        auto success = ioctl(m_fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
        if (success < 0)
        {
            throw std::system_error(EFAULT, std::system_category(), "Failed to write pin state.");
        }
    }

    void GpioPin::reopen(GpioPin::Direction direction, GpioPin::Edge edge, GpioPin::Bias bias, GpioPin::Drive drive, GpioPin::Invert invert)
    {
        uint32_t flags = to_request(bias) | to_request(drive) | to_request(invert) | to_request(direction);

        if (m_fd >= 0)
        {
            auto success = close(m_fd);
            if (success < 0)
            {
                throw std::system_error(-success, std::system_category(), "Failed to close GPIO line.");
            }
            m_fd = -1;
        }

        if (direction == GpioPin::Direction::In && edge == GpioPin::Edge::None)
        {
            struct gpiohandle_request request = {};

            request.lines = 1;
            request.lineoffsets[0] = m_line;
            request.flags = flags;
            strncpy(request.consumer_label, m_label.c_str(), sizeof(request.consumer_label));

            auto success = ioctl(m_chip->m_fd, GPIO_GET_LINEHANDLE_IOCTL, &request);
            if (success < 0)
            {
                throw std::system_error(-success, std::system_category(), "Failed to open input line handle.");
            }

            m_fd = request.fd;
        }
        else if (direction == GpioPin::Direction::In && edge != GpioPin::Edge::None)
        {
            struct gpioevent_request request = {};

            request.lineoffset = m_line;
            request.handleflags = flags;
            request.eventflags = to_request(edge);
            strncpy(request.consumer_label, m_label.c_str(), sizeof(request.consumer_label) - 1);

            auto success = ioctl(m_chip->m_fd, GPIO_GET_LINEEVENT_IOCTL, &request);
            if (success < 0)
            {
                throw std::system_error(-success, std::system_category(), "Failed to open input event line handle.");
            }

            m_fd = request.fd;
        }
        else
        {
            struct gpiohandle_request request = {};
            bool initial_value = (direction == GpioPin::Direction::High);
            initial_value ^= (invert == GpioPin::Invert::On);

            request.lines = 1;
            request.lineoffsets[0] = m_line;
            request.flags = flags;
            request.default_values[0] = initial_value;
            strncpy(request.consumer_label, m_label.c_str(), sizeof(request.consumer_label) - 1);

            auto success = ioctl(m_chip->m_fd, GPIO_GET_LINEHANDLE_IOCTL, &request);
            if (success < 0)
            {
                throw std::system_error(EFAULT, std::system_category(), "Failed to open output line handle.");
            }

            m_fd = request.fd;
        }

        m_direction = direction;
        m_edge = edge;
        m_bias = bias;
        m_drive = drive;
        m_invert = invert;
    }

    auto GpioPin::direction() const -> GpioPin::Direction { return m_direction; }
    void GpioPin::direction(GpioPin::Direction value)
    {
        if (m_direction != value)
        {
            reopen(value, m_edge, m_bias, m_drive, m_invert);
        }
    }

    auto GpioPin::edge() const -> GpioPin::Edge { return m_edge; }
    void GpioPin::edge(GpioPin::Edge value)
    {
        if (m_edge != value)
        {
            reopen(m_direction, value, m_bias, m_drive, m_invert);
        }
    }

    auto GpioPin::bias() const -> GpioPin::Bias { return m_bias; }
    void GpioPin::bias(GpioPin::Bias value)
    {
        if (m_bias != value)
        {
            reopen(m_direction, m_edge, value, m_drive, m_invert);
        }
    }

    auto GpioPin::drive() const -> GpioPin::Drive { return m_drive; }
    void GpioPin::drive(GpioPin::Drive value)
    {
        if (m_drive != value)
        {
            reopen(m_direction, m_edge, m_bias, value, m_invert);
        }
    }

    auto GpioPin::invert() const -> GpioPin::Invert { return m_invert; }
    void GpioPin::invert(GpioPin::Invert value)
    {
        if (m_invert != value)
        {
            reopen(m_direction, m_edge, m_bias, m_drive, value);
        }
    }

    void GpioPin::reconfigure(GpioPin::Direction direction, GpioPin::Edge edge, GpioPin::Bias bias,
                              GpioPin::Drive drive, GpioPin::Invert invert)
    {
        reopen(direction, edge, bias, drive, invert);
    }

    GpioPin::GpioPin(std::shared_ptr<GpioChip> chip, unsigned int line, const std::string &label,
                     GpioPin::Direction direction, GpioPin::Edge edge, GpioPin::Bias bias, GpioPin::Drive drive,
                     GpioPin::Invert invert)
                     : m_chip(chip), m_fd(-1), m_line(line), m_label(label)
    {
        reopen(direction, edge, bias, drive, invert);
    }

    GpioPin::~GpioPin()
    {
        close(m_fd);
    }


    auto to_request(GpioPin::Edge edge) -> unsigned long {
        switch (edge) {
            case GpioPin::Edge::Rising:  return GPIOEVENT_REQUEST_RISING_EDGE;
            case GpioPin::Edge::Falling: return GPIOEVENT_REQUEST_FALLING_EDGE;
            case GpioPin::Edge::Both:    return GPIOEVENT_REQUEST_BOTH_EDGES;
            case GpioPin::Edge::None:    return 0UL;
        }
        return 0UL;
    }

    auto to_request(GpioPin::Invert invert) -> unsigned long {
        switch (invert) {
            case GpioPin::Invert::On:   return GPIOHANDLE_REQUEST_ACTIVE_LOW;
            case GpioPin::Invert::Off:  return 0UL;
        }
        return 0UL;
    }

    auto to_request(GpioPin::Direction direction) -> unsigned  long {
        switch (direction) {
            case GpioPin::Direction::In:    return GPIOHANDLE_REQUEST_INPUT;
            case GpioPin::Direction::Out:
            case GpioPin::Direction::Low:
            case GpioPin::Direction::High:  return GPIOHANDLE_REQUEST_OUTPUT;
        }
        return 0UL;
    }

#ifdef GPIOHANDLE_REQUEST_BIAS_PULL_UP
    auto to_request(GpioPin::Bias bias) {
        switch (bias) {
            case GpioPin::Bias::PullUp:     return GPIOHANDLE_REQUEST_BIAS_PULL_UP;
            case GpioPin::Bias::PullDown:   return GPIOHANDLE_REQUEST_BIAS_PULL_DOWN;
            case GpioPin::Bias::Disable:    return GPIOHANDLE_REQUEST_DISABLE;
            case GpioPin::Bias::Default:    return 0UL;
        }
        return 0UL;
    }
#else
    auto to_request(GpioPin::Bias bias) -> unsigned long {
        if (bias != GpioPin::Bias::Default) {
            throw std::runtime_error("Kernel version does not support configuring GPIO line bias.");
        }
        return 0UL;
    }
#endif

#ifdef GPIOHANDLE_REQUEST_OPEN_DRAIN
    auto to_request(GpioPin::Drive drive) -> unsigned long {
        switch (drive) {
            case GpioPin::Drive::OpenSource:  return GPIOHANDLE_REQUEST_OPEN_SOURCE;
            case GpioPin::Drive::OpenDrain:   return GPIOHANDLE_REQUEST_OPEN_DRAIN;
            case GpioPin::Drive::Default:     return 0UL;
        }
        return 0UL;
    }
#else
    auto to_request(GpioPin::Drive drive) -> unsigned long {
        if (drive != GpioPin::Drive::Default) {
            throw std::runtime_error("Kernel version does not support configuring GPIO line drive.");
        }
        return 0UL;
    }
#endif




}