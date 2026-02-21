/*
 * ESP32.hpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#pragma once

#include "ADC/Continuous.hpp"
#include "ADC/Oneshot.hpp"
#include "GPIO.hpp"
#include "MCPWM/MCPWM.hpp"

#include <memory>
#include <unordered_set>

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

        adc::ADCOneshotPtr<adc::Uncalibrated> adcOneshot(adc_unit_t unit, esp_err_t& err);
        adc::ADCOneshotPtr<adc::Calibrated> adcOneshot(adc::ADCCalibrationPtr calibration, esp_err_t& err);
        adc::ADCContinuousPtr adcContinuous(const adc::ADCContinuousConfig& config, esp_err_t& err);

        GPIOPtr gpio(GPIOConfig gpioConfig, esp_err_t& err);

        mcpwm::MCPWM& mcpwm() { return _mcpwm; }

        static std::pair<adc_unit_t, adc_channel_t> adcChannelForGPIO(gpio_num_t gpio, esp_err_t& err);
        static gpio_num_t gpioForAdcChannel(adc_unit_t unit, adc_channel_t channel, esp_err_t& err);

    protected:
        ESP32();

        static std::unordered_set<size_t> _unitsForADCContinuousConfig(adc::ADCContinuousConfig config);

        std::vector<std::weak_ptr<adc::ADCOneshot<adc::Uncalibrated>>> _uncalibratedAdcs;
        std::vector<std::weak_ptr<adc::ADCOneshot<adc::Calibrated>>> _calibratedAdcs;
        std::vector<std::weak_ptr<adc::ADCContinuous>> _continuousAdcs;
        std::vector<std::weak_ptr<GPIO>> _gpios;

        mcpwm::MCPWM _mcpwm;

    private:
        static constexpr char _loggingTag[] = "esp::ESP32";
    };
}  // namespace esp

#if defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3
#include "ESP32S3.hpp"
#endif
