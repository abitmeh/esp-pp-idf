#pragma once

#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_log.h>

#include <memory>

namespace esp {
    enum ADCCalibrationState : bool {
        Uncalibrated = false,
        Calibrated
    };

    class ESP32;

    template <ADCCalibrationState calibration>
    class ADCOneshot;

    template <ADCCalibrationState calibration>
    class ADCOneshotChannel;

    class ADCCalibration;
    using ADCCalibrationPtr = std::shared_ptr<ADCCalibration>;

    class ADCCalibration {
    public:
        ADCCalibration(adc_unit_t unit, adc_atten_t attenuation, adc_bitwidth_t bitwidth, esp_err_t& err);

    private:
        adc_unit_t _unit;
        adc_atten_t _attenuation;
        adc_bitwidth_t _bitwidth;

        adc_cali_handle_t _calibration;

        template <ADCCalibrationState calibration>
        friend class ADCOneshot;
        template <ADCCalibrationState calibration>
        friend class ADCOneshotChannel;
        friend class ESP32;

        static constexpr char _loggingTag[] = "esp::ADCCalibration";
    };

    //
    // IMPLEMENTATION
    //

    inline ADCCalibration::ADCCalibration(adc_unit_t unit, adc_atten_t attenuation, adc_bitwidth_t bitwidth, esp_err_t& err)
        : _unit(unit), _attenuation(attenuation), _bitwidth(bitwidth) {
#if defined(ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED) && ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        adc_cali_curve_fitting_config_t calibrationConfig = {
            .unit_id = unit,
            .chan = ADC_CHANNEL_0,  // Ignored, but here to suppress warning about missing field.
            .atten = attenuation,
            .bitwidth = bitwidth,
        };
        err = adc_cali_create_scheme_curve_fitting(&calibrationConfig, &_calibration);
#else
        adc_cali_line_fitting_config_t calibrationConfig = {
            .unit_id = unit,
            .atten = attenuation,
            .bitwidth = bitwidth,
        };
        err = adc_cali_create_scheme_line_fitting(&calibrationConfig, &_calibration);
#endif
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_cali_create_scheme_curve_fitting failed: %s", esp_err_to_name(err));
        }
    }
}  // namespace esp
