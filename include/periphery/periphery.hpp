/*!
 * \package         cpp-periphery
 * \file            periphery.hpp
 * \author          Mark Butowski (github.com/mpb27)
 * \date            2018-02-28
 * \license         MIT     (https://stackoverflow.com/questions/12353409)
 *
 * <a href="GitHub"> https://github.com/mpb27/cpp-periphery </a>
 */


#include <stdexcept>


namespace periphery {

    class timeout_exception : public std::runtime_error {
    public:
        timeout_exception() : std::runtime_error("") { }
    };

}
