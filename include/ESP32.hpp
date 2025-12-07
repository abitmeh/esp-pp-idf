/*
 * ESP32.hpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#pragma once

#include "ADCOneshot.hpp"
#include "GPIO.hpp"
#include "mcpwm/MCPWM.hpp"

#include <memory>

namespace esp {
    class ESP32;
    using ESP32Ptr = std::shared_ptr<ESP32>;

#if defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3
    class ESP32S3;
#endif

    class ESP32 {
    public:
#if defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3
        using specialisation = ::esp::ESP32S3;
#endif
        using specialisationPtr = std::shared_ptr<specialisation>;

        static specialisationPtr sharedESP32();

        virtual size_t numADCs() = 0;
        virtual size_t numGPIOs() = 0;

        ADCOneshotPtr<Uncalibrated> adcOneshot(adc_unit_t unit, esp_err_t& err);
        ADCOneshotPtr<Calibrated> adcOneshot(ADCCalibrationPtr calibration, esp_err_t& err);
        GPIOPtr gpio(GPIOConfig gpioConfig, esp_err_t& err);

        mcpwm::MCPWM& mcpwm() { return _mcpwm; }

    protected:
        ESP32();

        std::vector<std::weak_ptr<ADCOneshot<Uncalibrated>>> _uncalibratedAdcs;
        std::vector<std::weak_ptr<ADCOneshot<Calibrated>>> _calibratedAdcs;
        std::vector<std::weak_ptr<GPIO>> _gpios;

        mcpwm::MCPWM _mcpwm;

    private:
        static constexpr char _loggingTag[] = "esp::ESP32";
    };
}  // namespace esp

#if defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3
#include "ESP32S3.hpp"
#endif
