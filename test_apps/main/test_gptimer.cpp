extern "C" {
#include <unity.h>
}

#include "GPTimer.hpp"

using namespace esp;

InterruptResult gpTimerCallback(GPTimer& timer, const gptimer_alarm_event_data_t& eventData, void* userInfo) {
    return InterruptResult::NoHighPriorityTaskWoken;
}

TEST_CASE("Create and destroy", "[GPTimer]") {
    esp_err_t err = ESP_OK;
    const GPTimerConfig timerConfig = {
        .durationMicroseconds = 1'000,
        .callback = gpTimerCallback
    };
    GPTimerPtr timer = std::make_shared<GPTimer>(timerConfig, nullptr, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(timer);
}
