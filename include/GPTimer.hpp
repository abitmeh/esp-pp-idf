/*
 * GPTimer.hpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#pragma once

#include "Interrupt.hpp"

#include <driver/gptimer.h>

#include <esp_attr.h>

#include <functional>
#include <memory>

namespace esp {
    IRAM_ATTR bool _onAlarm(gptimer_handle_t timerHandle, const gptimer_alarm_event_data_t* eventData, void* userInfo);

    class GPTimer;
    using GPTimerPtr = std::shared_ptr<GPTimer>;

    using GPTimerCallback = InterruptResult(*)(GPTimer&, const gptimer_alarm_event_data_t&, void* userInfo);

    struct GPTimerConfig {
        uint32_t durationMicroseconds;
        GPTimerCallback callback;
    };

    class GPTimer {
    public:
        GPTimer(const GPTimerConfig& config, void* userInfo, esp_err_t& err);
        ~GPTimer();

        void start(esp_err_t& err);
        void stop(esp_err_t& err);

    private:
        gptimer_handle_t _timer;

        IRAM_ATTR bool onAlarm(const gptimer_alarm_event_data_t& eventData, void* userInfo);

        GPTimerCallback _callback;
        std::pair<GPTimer*, void*> _userInfo;

        static constexpr char _loggingTag[] = "esp::GPTimer";

        friend bool _onAlarm(gptimer_handle_t, const gptimer_alarm_event_data_t*, void*);
    };
}  // namespace esp
