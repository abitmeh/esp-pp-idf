#pragma once

#include <esp_timer.h>

#include <chrono>
#include <expected>

namespace esp {
    class ESP32;

    class Timer;
    using TimerPtr = std::shared_ptr<Timer>;

    using TimerCallback = void(*)(Timer& timer, void* userInfo);

    enum class TimerDispatchMethod : uint8_t {
        Task = ESP_TIMER_TASK,
#if CONFIG_ESP_TIMER_SUPPORTS_ISR_DISPATCH_METHOD
        ISR = ESP_TIMER_ISR
#endif
    };

    struct TimerConfig {
        TimerCallback callback;
        void* userInfo;
        TimerDispatchMethod dispatchMethod;
        std::string name;
        bool skipUnhandledEvents;
    };

    namespace priv {
        void timerCallback(void* userData);
    }

    class Timer {
    public:
        static std::chrono::microseconds now();
        static std::chrono::microseconds nextTimerFire();
        static std::chrono::microseconds nextWakingTimerFire();

        static void requestISRYield();

        Timer(const Timer& other) = delete;
        Timer(Timer&& other);

        ~Timer();

        Timer& operator=(const Timer& other) = delete;
        Timer& operator=(Timer&& other);

        esp_err_t startOneshot(std::chrono::microseconds duration);
        esp_err_t startPeriodic(std::chrono::microseconds duration);

        esp_err_t restart(std::chrono::microseconds duration);

        esp_err_t stop();

        std::expected<std::chrono::microseconds, esp_err_t> duration() const;
        std::expected<std::chrono::microseconds, esp_err_t> expiry() const;

        bool isActive() const;

    private:
        Timer(const TimerConfig& config, esp_err_t& err);

        void _timerCallback(void* userInfo);

        void _teardown();

        TimerCallback _callback;
        esp_timer_handle_t _timerHandle = nullptr;
        std::pair<Timer*, void*> _userInfo;

        static constexpr char _loggingTag[] = "esp::Timer";

        friend void priv::timerCallback(void* userData);
        friend class ESP32;
    };
}
