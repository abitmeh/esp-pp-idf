extern "C" {
#include <unity.h>
}

#include "ADC/Oneshot.hpp"
#include "ESP32.hpp"

using namespace esp;
using namespace esp::adc;

TEST_CASE("Create and destroy units", "[ADCOneshot]") {
    esp_err_t err = ESP_OK;
    ADCOneshotPtr<Uncalibrated> adc1 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_1, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc1);
    ADCOneshotPtr<Uncalibrated> adc2 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_2, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc2);
    TEST_ASSERT_NOT_EQUAL(adc1.get(), adc2.get());
    ADCOneshotPtr<Uncalibrated> adc3 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_1, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc3);
    TEST_ASSERT_EQUAL(adc1.get(), adc3.get());
    adc2 = nullptr;
    adc2 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_2, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc2);
}

TEST_CASE("Get channels", "[ADCOneshot]") {
    esp_err_t err = ESP_OK;
    ADCOneshotPtr<Uncalibrated> adc1 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_1, err);
    ADCOneshotPtr<Uncalibrated> adc2 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_2, err);
    ADCChannelConfig config(ADC_CHANNEL_0, Attenuation::Decibels12, BitWidth::Bits12);
    ADCOneshotChannelPtr unit1Channel0 = adc1->channel(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(unit1Channel0);
    ADCChannelConfig config2(ADC_CHANNEL_1, Attenuation::Decibels6, BitWidth::Bits12);
    ADCOneshotChannelPtr unit1Channel1 = adc1->channel(config2, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(unit1Channel1);
    ADCOneshotChannelPtr unit2Channel0 = adc2->channel(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(unit2Channel0);
    ADCChannelConfig config3(ADC_CHANNEL_1, Attenuation::Decibels2_5, BitWidth::Bits12);
    ADCOneshotChannelPtr unit2Channel1 = adc2->channel(config3, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(unit2Channel1);
    ADCOneshotChannelPtr unit1Channel0Again = adc1->channel(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_EQUAL(unit1Channel0.get(), unit1Channel0Again.get());
    ADCOneshotChannelPtr unit2Channel1DifferentConfig = adc2->channel(config2, err);
    TEST_ASSERT_EQUAL(err, ESP_ERR_INVALID_STATE);
    TEST_ASSERT_NULL(unit2Channel1DifferentConfig);
}

TEST_CASE("Read channels", "[ADCOneshot]") {
    esp_err_t err = ESP_OK;
    ADCOneshotPtr<Uncalibrated> adc1 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_1, err);
    ADCChannelConfig config(ADC_CHANNEL_0, Attenuation::Decibels12, BitWidth::Bits12);
    ADCOneshotChannelPtr unit1Channel0 = adc1->channel(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(unit1Channel0);
    uint16_t value = unit1Channel0->read();
    TEST_ASSERT(value <= 4095);
}

TEST_CASE("Read channels in ISR", "[ADCOneshot]") {
    esp_err_t err = ESP_OK;
    ADCOneshotPtr<Uncalibrated> adc1 = ESP32::sharedESP32()->adcOneshot(ADC_UNIT_1, err);
    ADCChannelConfig config(ADC_CHANNEL_0, Attenuation::Decibels12, BitWidth::Bits12);
    ADCOneshotChannelPtr unit1Channel0 = adc1->channel(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(unit1Channel0);

    // Simulate ISR by calling readIsr directly
    uint16_t value = readIsr(unit1Channel0.get());
    TEST_ASSERT(value <= 4095);
}

TEST_CASE("Create calibrated ADCOneshot", "[ADCOneshot]") {
    esp_err_t err = ESP_OK;
    ADCCalibrationPtr calibration = std::make_shared<ADCCalibration>(ADC_UNIT_1, Attenuation::Decibels12, BitWidth::Bits12, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(calibration);
    ADCOneshotPtr<Calibrated> calibratedAdc = ESP32::sharedESP32()->adcOneshot(calibration, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(calibratedAdc);
    ADCOneshotChannelPtr calibratedChannel = calibratedAdc->channel(ADC_CHANNEL_0, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(calibratedChannel);
    uint16_t value = calibratedChannel->read();
    TEST_ASSERT(value <= 4095);
}

TEST_CASE("Read calibrated channel in mV", "[ADCOneshot]") {
    esp_err_t err = ESP_OK;
    ADCCalibrationPtr calibration = std::make_shared<ADCCalibration>(ADC_UNIT_1, Attenuation::Decibels12, BitWidth::Bits12, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(calibration);
    ADCOneshotPtr<Calibrated> calibratedAdc = ESP32::sharedESP32()->adcOneshot(calibration, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(calibratedAdc);
    ADCOneshotChannelPtr calibratedChannel = calibratedAdc->channel(ADC_CHANNEL_0, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(calibratedChannel);
    uint16_t mV = calibratedChannel->miliVolts();
    TEST_ASSERT(mV >= 0);
    TEST_ASSERT(mV <= 2450);  // Attenuation of 12dB gives max ~2.45V
}
