/*
 * ESP32.cpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#include "ESP32.hpp"

#include "mcpwm/MCPWM.hpp"

#include <esp_log.h>

using namespace esp;

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
    if (adc == nullptr) {
        ADCOneshotPtr<Calibrated> calibratedAdc = _calibratedAdcs[unitNum].lock();
        if (calibratedAdc != nullptr) {
            err = ESP_ERR_INVALID_STATE;
            return nullptr;
        }
        adc = std::shared_ptr<ADCOneshot<Uncalibrated>>(new ADCOneshot<Uncalibrated>(unit, err));
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "ESP32::adcOneshot failed: %s", esp_err_to_name(err));
            return nullptr;
        }
        _uncalibratedAdcs[unitNum] = adc;
    }

    return adc;
}

ADCOneshotPtr<Calibrated> ESP32::adcOneshot(ADCCalibrationPtr calibration, esp_err_t& err) {
    size_t unitNum = static_cast<size_t>(calibration->_unit);
    if (numADCs() < unitNum) {
        err = ESP_ERR_INVALID_ARG;
        return nullptr;
    }

    ADCOneshotPtr<Calibrated> adc = _calibratedAdcs[unitNum].lock();
    if (adc == nullptr) {
        ADCOneshotPtr<Uncalibrated> uncalibratedAdc = _uncalibratedAdcs[unitNum].lock();
        if (uncalibratedAdc != nullptr) {
            err = ESP_ERR_INVALID_STATE;
            return nullptr;
        }
        adc = std::shared_ptr<ADCOneshot<Calibrated>>(new ADCOneshot<Calibrated>(calibration, err));
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "ESP32::adcOneshot failed: %s", esp_err_to_name(err));
            return nullptr;
        }
        _calibratedAdcs[unitNum] = adc;
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

