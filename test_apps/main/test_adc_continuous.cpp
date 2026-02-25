extern "C" {
#include <unity.h>
}

#include "ADC/Continuous.hpp"
#include "ESP32.hpp"

using namespace esp;
using namespace esp::adc;

TEST_CASE("Create and destroy units", "[ADCContinuous]") {
    adc::ADCContinuousConfig config1{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_0,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    esp_err_t err = ESP_OK;
    ADCContinuousPtr adc1 = ESP32::sharedESP32()->adcContinuous(config1, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc1);
    ADCContinuousPtr adc2 = ESP32::sharedESP32()->adcContinuous(config1, err);
    TEST_ASSERT_EQUAL(err, ESP_ERR_INVALID_STATE);
    TEST_ASSERT_NULL(adc2);
    adc1 = nullptr;
    adc2 = nullptr;
    adc1 = ESP32::sharedESP32()->adcContinuous(config1, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc1);
}

TEST_CASE("Get channels", "[ADCContinuous]") {
    adc::ADCContinuousConfig config{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_0,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_1,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    esp_err_t err = ESP_OK;
    ADCContinuousPtr adc = ESP32::sharedESP32()->adcContinuous(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc);
}

TEST_CASE("Start and stop", "[ADCContinuous]") {
    adc::ADCContinuousConfig config{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_0,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    esp_err_t err = ESP_OK;
    ADCContinuousPtr adc = ESP32::sharedESP32()->adcContinuous(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc);
    err = adc->start();
    TEST_ASSERT_EQUAL(err, ESP_OK);
    err = adc->stop();
    TEST_ASSERT_EQUAL(err, ESP_OK);
}

TEST_CASE("Read", "[ADCContinuous]") {
    adc::ADCContinuousConfig config{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_0,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    esp_err_t err = ESP_OK;
    ADCContinuousPtr adc = ESP32::sharedESP32()->adcContinuous(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc);
    err = adc->start();
    TEST_ASSERT_EQUAL(err, ESP_OK);
    std::vector<uint8_t> rawData = adc->read<512>(1000, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_FALSE(rawData.empty());
    err = adc->stop();
    TEST_ASSERT_EQUAL(err, ESP_OK);
}

TEST_CASE("Read parsed", "[ADCContinuous]") {
    adc::ADCContinuousConfig config{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_0,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    esp_err_t err = ESP_OK;
    ADCContinuousPtr adc = ESP32::sharedESP32()->adcContinuous(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc);
    err = adc->start();
    TEST_ASSERT_EQUAL(err, ESP_OK);
    std::vector<adc_continuous_data_t> parsedData = adc->readParsed<128>(1000, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_FALSE(parsedData.empty());
}

TEST_CASE("Parse", "[ADCContinuous]") {
    adc::ADCContinuousConfig config{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_0,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    esp_err_t err = ESP_OK;
    ADCContinuousPtr adc = ESP32::sharedESP32()->adcContinuous(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc);
    err = adc->start();
    TEST_ASSERT_EQUAL(err, ESP_OK);
    std::vector<uint8_t> rawData = adc->read<512>(1000, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_FALSE(rawData.empty());
    std::vector<adc_continuous_data_t> parsedData = adc->parse(rawData, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_FALSE(parsedData.empty());
}

TEST_CASE("Flush", "[ADCContinuous]") {
    adc::ADCContinuousConfig config{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_0,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    esp_err_t err = ESP_OK;
    ADCContinuousPtr adc = ESP32::sharedESP32()->adcContinuous(config, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc);
    err = adc->flush();
    TEST_ASSERT_EQUAL(err, ESP_OK);
    err = adc->start();
    TEST_ASSERT_EQUAL(err, ESP_OK);
    err = adc->stop();
    TEST_ASSERT_EQUAL(err, ESP_OK);
    err = adc->flush();
    TEST_ASSERT_EQUAL(err, ESP_OK);
}

TEST_CASE("Unit already in use", "[ADCContinuous]") {
    adc::ADCContinuousConfig config1{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_0,
                    .attenuation = Attenuation::Decibels12,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    adc::ADCContinuousConfig config2{
        .maximumStoredValues = 32,
        .numberOfValuesPerConversionFrame = 16,
        .channels =
            {
                {
                    .unit = ADC_UNIT_1,
                    .channel = ADC_CHANNEL_1,
                    .attenuation = Attenuation::Decibels6,
                    .bitwidth = BitWidth::Bits12,
                },
            },
        .samplingFrequencyHz = 1000,
    };

    esp_err_t err = ESP_OK;
    ADCContinuousPtr adc1 = ESP32::sharedESP32()->adcContinuous(config1, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(adc1);
    ADCContinuousPtr adc2 = ESP32::sharedESP32()->adcContinuous(config2, err);
    TEST_ASSERT_EQUAL(err, ESP_ERR_INVALID_STATE);
    TEST_ASSERT_NULL(adc2);
}
