/*
 * GPIOFault.hpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#pragma once

#include "Interrupt.hpp"

#include <driver/gpio.h>
#include <driver/mcpwm_fault.h>

#include <functional>
#include <memory>

namespace esp {
    namespace mcpwm {
        struct GPIOFaultConfig {
            uint8_t groupId;
            InterruptPriority interruptPriority = Default;
            gpio_num_t gpioNum;

            bool activeHigh = false;
        };

        class GPIOFault;
        using GPIOFaultPtr = std::shared_ptr<GPIOFault>;

        bool _onFaultEnter(mcpwm_fault_handle_t fault, const mcpwm_fault_event_data_t* faultData, void* userInfo);
        bool _onFaultExit(mcpwm_fault_handle_t fault, const mcpwm_fault_event_data_t* faultData, void* userInfo);

        class GPIOFault {
        public:
            using Callback = InterruptResult(*)(const mcpwm_fault_event_data_t&, void* userInfo);

            struct Callbacks {
                Callback onFaultEnter = nullptr;
                Callback onFaultExit = nullptr;
            };

            ~GPIOFault();

            void setCallbacks(const Callbacks& callbacks, void* userInfo, esp_err_t& err);

        private:
            GPIOFault(const GPIOFaultConfig& config, esp_err_t& err);

            Callbacks _callbacks;
            std::pair<GPIOFault*, void*> _userInfo;

            mcpwm_fault_handle_t _fault;

            static constexpr char _loggingTag[] = "esp::mcpwm::GPIOFault";

            friend class MCPWM;

            friend bool _onFaultEnter(mcpwm_fault_handle_t fault, const mcpwm_fault_event_data_t* faultData, void* userInfo);
            friend bool _onFaultExit(mcpwm_fault_handle_t, const mcpwm_fault_event_data_t*, void* userINfo);
        };
    }  // namespace mcpwm
}  // namespace esp

