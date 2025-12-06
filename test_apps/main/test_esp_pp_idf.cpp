extern "C" {
#include <unity.h>
}

#include "ADC.hpp"
#include "ESP32.hpp"

using namespace esp;

TEST_CASE("Create and destroy units", "[ADC]") {
    esp_err_t err = ESP_OK;
    ADCOneshotPtr adc1 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_1, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc1);
    ADCOneshotPtr adc2 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_2, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc2);
    TEST_ASSERT_NOT_EQUAL(adc1.get(), adc2.get());
    ADCOneshotPtr adc3 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_1, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc3);
    TEST_ASSERT_EQUAL(adc1.get(), adc3.get());
    adc2 = nullptr;
    adc2 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_2, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc2);
}

extern "C" void app_main(void) {
    unity_run_all_tests();
}
