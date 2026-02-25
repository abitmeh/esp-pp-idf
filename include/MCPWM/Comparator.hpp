/*
 * Comparator.hpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#pragma once

#include "Interrupt.hpp"
#include "MCPWM/Types.hpp"

#include <driver/mcpwm_cmpr.h>

#include <memory>
#include <functional>

namespace esp {
    namespace mcpwm {
        struct ComparatorConfig {
            InterruptPriority interruptPriority = Default;

            bool updateComparatorOnTimerZero = false;
            bool updateComparatorOnTimerPeak = false;
            bool updateComparatorOnSync = false;
        };

        using ComparatorCallback = InterruptResult(*)(uint32_t compareValue, TimerDirection direction, void* userInfo);

        class Comparator;
        using ComparatorPtr = std::shared_ptr<Comparator>;

        class Operator;
        using OperatorPtr = std::shared_ptr<Operator>;

        bool _onCompare(mcpwm_cmpr_handle_t handle, const mcpwm_compare_event_data_t* data, void* userData);

        class Comparator {
        public:
            ~Comparator();

            uint32_t compareValue() const;
            void setCompareValue(uint32_t compareValue, esp_err_t& err);

            esp_err_t setCallback(const ComparatorCallback& callback, void* userInfo);

        private:
            Comparator(OperatorPtr oper, const ComparatorConfig& config, esp_err_t& err);

            mcpwm_cmpr_handle_t _comparator;
            uint32_t _compareValue = 0;

            ComparatorCallback _callback = nullptr;
            std::pair<Comparator*, void*> _userInfo;

            std::weak_ptr<Operator> _operator;

            static constexpr char _loggingTag[] = "esp::mcpwm::Comparator";

            friend class Operator;
            friend class Generator;
            friend bool _onCompare(mcpwm_cmpr_handle_t handle, const mcpwm_compare_event_data_t* data, void* userData);
        };
    }  // namespace mcpwm
}  // namespace esp
