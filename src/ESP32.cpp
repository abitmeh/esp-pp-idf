/*
 * ESP32.cpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#include "ESP32.hpp"

#include "MCPWM/MCPWM.hpp"

#include <esp_log.h>
#include <algorithm>

using namespace esp;
using namespace esp::adc;

namespace esp {
    static ESP32::specialisationPtr _sharedESP32;
}

ESP32::specialisationPtr ESP32::sharedESP32() {
    if (_sharedESP32 == nullptr) {
        _sharedESP32 = std::make_shared<ESP32::specialisation>();
    }

    return _sharedESP32;
}

ESP32::ESP32() : _mcpwm() {}

ADCOneshotPtr<Uncalibrated> ESP32::adcOneshot(adc_unit_t unit, esp_err_t& err) {
    size_t unitNum = static_cast<size_t>(unit);
    if (numADCs() < unitNum) {
        err = ESP_ERR_INVALID_ARG;
        return nullptr;
    }

    ADCOneshotPtr<Uncalibrated> adc = _uncalibratedAdcs[unitNum].lock();
    if (adc != nullptr) {
        return adc;
    }

    ADCOneshotPtr<Calibrated> calibratedAdc = _calibratedAdcs[unitNum].lock();
    ADCContinuousPtr continuousAdc = _continuousAdcs[unitNum].lock();
    if (calibratedAdc != nullptr || continuousAdc != nullptr) {
        err = ESP_ERR_INVALID_STATE;
        return nullptr;
    }

    adc = std::shared_ptr<ADCOneshot<Uncalibrated>>(new ADCOneshot<Uncalibrated>(unit, err));
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "ESP32::adcOneshot failed: %s", esp_err_to_name(err));
        return nullptr;
    }
    _uncalibratedAdcs[unitNum] = adc;

    return adc;
}

ADCOneshotPtr<Calibrated> ESP32::adcOneshot(ADCCalibrationPtr calibration, esp_err_t& err) {
    size_t unitNum = static_cast<size_t>(calibration->_unit);
    if (numADCs() < unitNum) {
        err = ESP_ERR_INVALID_ARG;
        return nullptr;
    }

    ADCOneshotPtr<Calibrated> adc = _calibratedAdcs[unitNum].lock();
    if (adc != nullptr) {
        return adc;
    }

    ADCOneshotPtr<Uncalibrated> uncalibratedAdc = _uncalibratedAdcs[unitNum].lock();
    ADCContinuousPtr continuousAdc = _continuousAdcs[unitNum].lock();
    if (uncalibratedAdc != nullptr || continuousAdc != nullptr) {
        err = ESP_ERR_INVALID_STATE;
        return nullptr;
    }
    adc = std::shared_ptr<ADCOneshot<Calibrated>>(new ADCOneshot<Calibrated>(calibration, err));
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "ESP32::adcOneshot failed: %s", esp_err_to_name(err));
        return nullptr;
    }
    _calibratedAdcs[unitNum] = adc;

    return adc;
}

std::unordered_set<size_t> ESP32::_unitsForADCContinuousConfig(ADCContinuousConfig config) {
    std::unordered_set<size_t> units;
    for (const ADCContinuousChannelConfig& channelConfig : config.channels) {
        const size_t unitNum = static_cast<size_t>(channelConfig.unit);
        if (std::find(units.begin(), units.end(), unitNum) == units.end()) {
            units.insert(unitNum);
        }
    }
    return units;
}

ADCContinuousPtr ESP32::adcContinuous(const ADCContinuousConfig& config, esp_err_t& err) {
    const std::unordered_set<size_t> unitsNeeded = _unitsForADCContinuousConfig(config);
    for (size_t unitNum : unitsNeeded) {
        if (numADCs() < unitNum) {
            err = ESP_ERR_INVALID_ARG;
            return nullptr;
        }

        ADCOneshotPtr<Uncalibrated> uncalibratedAdc = _uncalibratedAdcs[unitNum].lock();
        ADCOneshotPtr<Calibrated> calibratedAdc = _calibratedAdcs[unitNum].lock();
        ADCContinuousPtr continuousAdc = _continuousAdcs[unitNum].lock();
        if (uncalibratedAdc != nullptr || calibratedAdc != nullptr || continuousAdc != nullptr) {
            err = ESP_ERR_INVALID_STATE;
            return nullptr;
        }
    }

    ADCContinuousPtr adc = std::shared_ptr<ADCContinuous>(new ADCContinuous(config, err));
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "ADCContinuous constructor failed: %s", esp_err_to_name(err));
        return nullptr;
    }
    for (size_t unitNum : unitsNeeded) {
        _continuousAdcs[unitNum] = adc;
    }

    return adc;
}

GPIOPtr ESP32::gpio(GPIOConfig gpioConfig, esp_err_t& err) {
    if (numGPIOs() < gpioConfig.gpioNum) {
        err = ESP_ERR_INVALID_ARG;
        return nullptr;
    }

    GPIOPtr gpio = _gpios[gpioConfig.gpioNum].lock();
    if (gpio == nullptr) {
        gpio = std::shared_ptr<GPIO>(new GPIO(gpioConfig, err));
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "ESP32::gpio failed: %s", esp_err_to_name(err));
            return nullptr;
        }
        _gpios[gpioConfig.gpioNum] = gpio;
    } else if (gpio->config() != gpioConfig) {
        err = ESP_ERR_INVALID_STATE;
        return nullptr;
    }

    return gpio;
}

std::expected<TimerPtr, esp_err_t> ESP32::timer(const TimerConfig& config) {
    esp_err_t err = ESP_OK;
    TimerPtr timer = std::shared_ptr<Timer>(new Timer(config, err));
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "ESP32::timer failed: %s", esp_err_to_name(err));
        return std::unexpected(err);
    }

    return timer;
}

std::pair<adc_unit_t, adc_channel_t> ESP32::adcChannelForGPIO(gpio_num_t gpio, esp_err_t& err) {
    adc_unit_t unit = ADC_UNIT_1;
    adc_channel_t channel = ADC_CHANNEL_0;
    err = adc_oneshot_io_to_channel(gpio, &unit, &channel);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_oneshot_io_to_channel failed: %s", esp_err_to_name(err));
    }
    return std::pair<adc_unit_t, adc_channel_t>(unit, channel);
}

gpio_num_t ESP32::gpioForAdcChannel(adc_unit_t unit, adc_channel_t channel, esp_err_t& err) {
    gpio_num_t gpio = GPIO_NUM_0;
    err = adc_oneshot_channel_to_io(unit, channel, reinterpret_cast<int*>(&gpio));
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_channel_to_io failed: %s", esp_err_to_name(err));
    }

    return gpio;
}
