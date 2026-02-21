/*
 * MCPWM.hpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#pragma once

#include "MCPWM/Comparator.hpp"
#include "MCPWM/GPIOFault.hpp"
#include "MCPWM/Generator.hpp"
#include "MCPWM/Operator.hpp"
#include "MCPWM/Timer.hpp"

namespace esp {
    class ESP32;

    namespace mcpwm {
        class MCPWM {
        public:
            TimerPtr timer(const TimerConfig& timerConfig, esp_err_t& err);
            GPIOFaultPtr gpioFault(const GPIOFaultConfig& faultConfig, esp_err_t& err);

        private:
            MCPWM() {}

            static constexpr char _loggingTag[] = "esp::mcpwm::MCPWM";

            friend class esp::ESP32;
        };
    }  // namespace mcpwm
}  // namespace esp
