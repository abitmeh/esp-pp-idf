#include "Timer.hpp"

#include <esp_log.h>
#include <esp_timer.h>

using namespace esp;
using namespace std::chrono_literals;

namespace esp {
    namespace priv {
        void timerCallback(void* userData) {
            auto [timer, userInfo] = *reinterpret_cast<std::pair<Timer*, void*>*>(userData);
            timer->_timerCallback(userInfo);
        }
    }
}

std::chrono::microseconds Timer::now() {
    return std::chrono::microseconds(esp_timer_get_time());
}

std::chrono::microseconds Timer::nextTimerFire() {
    return std::chrono::microseconds(esp_timer_get_next_alarm());
}

std::chrono::microseconds Timer::nextWakingTimerFire() {
    return std::chrono::microseconds(esp_timer_get_next_alarm_for_wake_up());
}

void Timer::requestISRYield() {
#if CONFIG_ESP_TIMER_SUPPORTS_ISR_DISPATCH_METHOD
    esp_timer_isr_dispatch_need_yield();
#endif
}

Timer::Timer(const TimerConfig& config, esp_err_t& err) {
    _userInfo = std::make_pair(this, config.userInfo);
    _callback = config.callback;
    esp_timer_create_args_t conf = {
        .callback = esp::priv::timerCallback,
        .arg = &_userInfo,
        .dispatch_method = static_cast<esp_timer_dispatch_t>(config.dispatchMethod),
        .name = config.name.c_str(),
        .skip_unhandled_events = config.skipUnhandledEvents
    };

    err = esp_timer_create(&conf, &_timerHandle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_timer_create failed: %s", esp_err_to_name(err));
        return;
    }
}

Timer::Timer(Timer&& other) {
    _userInfo = std::make_pair(this, other._userInfo.second);
    _callback = other._callback;
    _timerHandle = other._timerHandle;
    other._timerHandle = nullptr;
}

Timer::~Timer() {
    _teardown();
}

Timer& Timer::operator=(Timer&& other) {
    if (&other == this) {
        return *this;
    } 

    _teardown();
    _userInfo = std::make_pair(this, other._userInfo.second);
    _callback = other._callback;
    _timerHandle = other._timerHandle;
    other._timerHandle = nullptr;
    return *this;
}

esp_err_t Timer::startOneshot(std::chrono::microseconds duration) {
    esp_err_t err = ESP_OK;
    err = esp_timer_start_once(_timerHandle, duration.count());
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_timer_start_once failed: %s", esp_err_to_name(err));
        return err;
    }

    return err;
}

esp_err_t Timer::startPeriodic(std::chrono::microseconds duration) {
    esp_err_t err = ESP_OK;
    err = esp_timer_start_periodic(_timerHandle, duration.count());
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_timer_start_periodic failed: %s", esp_err_to_name(err));
        return err;
    }

    return err;
}

esp_err_t Timer::restart(std::chrono::microseconds duration) {
    esp_err_t err = ESP_OK;
    err = esp_timer_restart(_timerHandle, duration.count());
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_timer_restart failed: %s", esp_err_to_name(err));
        return err;
    }

    return err;
}

esp_err_t Timer::stop() {
    esp_err_t err = ESP_OK;
    err = esp_timer_stop(_timerHandle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_timer_stop failed: %s", esp_err_to_name(err));
        return err;
    }

    return err;
}

std::expected<std::chrono::microseconds, esp_err_t> Timer::duration() const  {
    esp_err_t err = ESP_OK;
    uint64_t period;
    err = esp_timer_get_period(_timerHandle, &period);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_timer_get_period failed: %s", esp_err_to_name(err));
        return std::unexpected(err);
    }

    return std::chrono::microseconds(period);
}

std::expected<std::chrono::microseconds, esp_err_t> Timer::expiry() const {
    esp_err_t err = ESP_OK;
    uint64_t expiry;
    err = esp_timer_get_expiry_time(_timerHandle, &expiry);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_timer_get_expiry_time failed: %s", esp_err_to_name(err));
        return std::unexpected(err);
    }

    return std::chrono::microseconds(expiry);
}

bool Timer::isActive() const {
    return esp_timer_is_active(_timerHandle);
}

void Timer::_timerCallback(void* userInfo) {
    if (_callback != nullptr) {
        _callback(*this, userInfo);
    }
}

void Timer::_teardown() {
    if (_timerHandle == nullptr) {
        return;
    }

    esp_err_t err = ESP_OK;
    if (isActive()) {
        err = stop();
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "esp::Timer::stop failed: %s", esp_err_to_name(err));
            return;
        }
    }

    err = esp_timer_delete(_timerHandle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_timer_delete failed: %s", esp_err_to_name(err));
        return;
    }
}
