/*!
 * \package         cpp-periphery
 * \file            gpio.hpp
 * \author          Mark Butowski (github.com/mpb27)
 * \date            2020-10-13
 * \license         MIT
 *
 * <a href="GitHub"> https://github.com/mpb27/cpp-periphery </a>
 */

#ifndef PERIPHERY_GPIO_HPP
#define PERIPHERY_GPIO_HPP

// C++11 includes:
#include <cstddef>
#include <cstdint>
#include <memory>

namespace periphery {

    class GpioChip {
    public:
        explicit GpioChip(const std::string &path);
        ~GpioChip();

    private:
        int m_fd;
        friend class GpioPin;
    };

    class GpioPin {
    public:
        enum class Direction { In, Out, Low, High };
        enum class Edge      { None, Rising, Falling, Both };
        enum class Bias      { Default, PullUp, PullDown, Disable };
        enum class Drive     { Default, OpenDrain, OpenSource };
        enum class Invert    { Off, On };
        enum class State     { Low, High };

        GpioPin(std::shared_ptr<GpioChip> chip, unsigned int line, const std::string& label,
                Direction direction,
                Edge edge = GpioPin::Edge::None,
                Bias bias = GpioPin::Bias::Default,
                Drive drive = GpioPin::Drive::Default,
                Invert invert = GpioPin::Invert::Off);
        //GpioPin(const GpioPin&) = delete;
        //GpioPin& operator=(const GpioPin&) = delete;
        ~GpioPin();


        auto get() const -> State;
        void set(State value);

        void reconfigure(GpioPin::Direction direction, GpioPin::Edge edge, GpioPin::Bias bias,
                         GpioPin::Drive drive, GpioPin::Invert invert);

        // ... getters ...
        auto direction() const -> GpioPin::Direction;
        auto edge()      const -> GpioPin::Edge;
        auto bias()      const -> GpioPin::Bias;
        auto drive()     const -> GpioPin::Drive;
        auto invert()    const -> GpioPin::Invert;

        // ... setters ...
        void direction(GpioPin::Direction value);
        void edge(GpioPin::Edge value);
        void bias(GpioPin::Bias value);
        void drive(GpioPin::Drive value);
        void invert(GpioPin::Invert value);

    private:
        std::shared_ptr<GpioChip> m_chip;
        int             m_fd;
        unsigned int    m_line;
        std::string     m_label;
        Direction m_direction;
        Edge m_edge;
        Bias m_bias;
        Drive m_drive;
        Invert m_invert;

        void reopen(GpioPin::Direction direction, GpioPin::Edge edge, GpioPin::Bias bias, GpioPin::Drive drive, GpioPin::Invert invert);
    };
}

#endif