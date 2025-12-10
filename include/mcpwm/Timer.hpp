/*
 * Timer.hpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#pragma once

#include "Interupt.hpp"

#include <driver/mcpwm_timer.h>

#include <functional>
#include <memory>
#include <vector>

namespace esp {
    namespace mcpwm {
        struct TimerConfig {
            uint8_t groupId;
            mcpwm_timer_clock_source_t clockSource = MCPWM_TIMER_CLK_SRC_DEFAULT;
            uint32_t frequency;
            uint32_t period;
            mcpwm_timer_count_mode_t countMode;

            bool updatePeriodOnEmpty = false;
            bool updatePeriodOnSync = false;
            bool allowPowerDown = false;

            InteruptPriority interuptPriority = Default;
        };

        class Timer;
        using TimerPtr = std::shared_ptr<Timer>;

        class OperatorConfig;
        class Operator;
        using OperatorPtr = std::shared_ptr<Operator>;

        bool _onFull(mcpwm_timer_handle_t, const mcpwm_timer_event_data_t*, void*);
        bool _onEmpty(mcpwm_timer_handle_t, const mcpwm_timer_event_data_t*, void*);
        bool _onStop(mcpwm_timer_handle_t, const mcpwm_timer_event_data_t*, void*);

        class Timer : public std::enable_shared_from_this<Timer> {
        public:
            using EventCallback = std::function<bool(const mcpwm_timer_event_data_t& eventData)>;

            struct EventCallbacks {
                EventCallback onFull;
                EventCallback onEmpty;
                EventCallback onStop;
            };

            enum class StartCommand : uint8_t {
                NoStop = 0,
                StopEmpty,
                StopFull
            };

            enum class StopCommand : uint8_t {
                StopEmpty = 0,
                StopFull
            };

            ~Timer();

            OperatorPtr addOperator(const OperatorConfig& config, esp_err_t& err);

            void removeOperator(const OperatorPtr& oper);

            void setEventCallbacks(const EventCallbacks& eventCallbacks, esp_err_t& err);

            void enable(esp_err_t& err);
            void disable(esp_err_t& err);

            void start(StartCommand cmd, esp_err_t& err);
            void stop(StopCommand cmd, esp_err_t& err);

            uint32_t period() const;
            void setPeriod(uint32_t periodTicks, esp_err_t& err);

        private:
            Timer(const TimerConfig& config, esp_err_t& err);

            TimerConfig _config;
            mcpwm_timer_handle_t _timer;

            std::vector<OperatorPtr> _operators;

            bool _enabled = false;

            EventCallbacks _callbacks;

            static constexpr char _loggingTag[] = "esp::mcpwm::Timer";

            friend class Operator;
            friend class MCPWM;

            friend bool _onFull(mcpwm_timer_handle_t, const mcpwm_timer_event_data_t*, void*);
            friend bool _onEmpty(mcpwm_timer_handle_t, const mcpwm_timer_event_data_t*, void*);
            friend bool _onStop(mcpwm_timer_handle_t, const mcpwm_timer_event_data_t*, void*);
        };

        enum class TimerDirection : uint8_t {
            Up = MCPWM_TIMER_DIRECTION_UP,
            Down = MCPWM_TIMER_DIRECTION_DOWN,
        };

        enum class TimerEvent : uint8_t {
            Full = MCPWM_TIMER_EVENT_FULL,
            Empty = MCPWM_TIMER_EVENT_EMPTY,
            Invalid = MCPWM_TIMER_EVENT_INVALID
        };
    }  // namespace mcpwm
}  // namespace esp

