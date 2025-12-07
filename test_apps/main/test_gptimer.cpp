extern "C" {
#include <unity.h>
}

#include "GPTimer.hpp"

using namespace esp;

TEST_CASE("Create and destroy", "[GPTimer]") {
    esp_err_t err = ESP_OK;
    const GPTimerConfig timerConfig = {
        .durationMicroseconds = 1'000,
        .callback = [](GPTimer& timer, const gptimer_alarm_event_data_t& eventData) -> bool { return false; },
    };
    GPTimerPtr timer = std::make_shared<GPTimer>(timerConfig, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(timer);
}
