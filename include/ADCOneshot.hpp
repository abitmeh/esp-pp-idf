/*
 * ADC.hpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#pragma once

#include "ADCCalibration.hpp"

#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>
#include <esp_private/adc_private.h>

#include <array>
#include <memory>
#include <vector>

namespace esp {
    struct ADCChannelConfig {
        ADCChannelConfig();
        ADCChannelConfig(adc_channel_t channel, adc_oneshot_chan_cfg_t configuration);

        adc_channel_t channel;
        adc_oneshot_chan_cfg_t configuration;
    };

    bool operator==(const ADCChannelConfig& a, const ADCChannelConfig& b);

    template <ADCCalibrationState calibration>
    class ADCOneshotChannel;

    template <ADCCalibrationState calibration>
    using ADCOneshotChannelPtr = std::shared_ptr<ADCOneshotChannel<calibration>>;

    template <ADCCalibrationState calibration>
    class ADCOneshot;

    template <ADCCalibrationState calibration>
    using ADCOneshotPtr = std::shared_ptr<ADCOneshot<calibration>>;

    uint16_t readIsr(ADCOneshotChannel<Uncalibrated>* channel);
    uint16_t readIsr(ADCOneshotChannel<Calibrated>* channel);
    uint16_t miliVoltsIsr(ADCOneshotChannel<Calibrated>* channel);

    template <ADCCalibrationState calibration>
    class ADCOneshotChannel : private std::enable_shared_from_this<ADCOneshotChannel<calibration>> {
    public:
        // Construct an ADCOneshotChannel by requesting one from an ADCOneshot.

        uint16_t read();

        template <ADCCalibrationState C = calibration, typename = std::enable_if_t<C>>
        uint16_t miliVolts();

    private:
        template <ADCCalibrationState C = calibration, typename = std::enable_if_t<!C>>
        ADCOneshotChannel(ADCOneshotPtr<calibration> adc, const ADCChannelConfig& config, esp_err_t& err);
        template <ADCCalibrationState C = calibration, typename = std::enable_if_t<C>>
        ADCOneshotChannel(ADCOneshotPtr<calibration> adc, adc_channel_t config, esp_err_t& err);

        ADCOneshotPtr<calibration> _adc;
        adc_oneshot_unit_handle_t _handle;  // Keep a copy here for ISR use, since we can't dereference shared_ptrs in ISR.
        adc_channel_t _channel;
        adc_cali_handle_t _calibrationHandle;
        ADCChannelConfig _config;

        friend class ADCOneshot<calibration>;
        friend uint16_t readIsr(ADCOneshotChannel<Uncalibrated>* channel);
        friend uint16_t readIsr(ADCOneshotChannel<Calibrated>* channel);
        friend uint16_t miliVoltsIsr(ADCOneshotChannel<Calibrated>* channel);

        static constexpr char _loggingTag[] = "esp::ADCOneshotChannel";
    };

    static constexpr size_t kADCUnitCount = 2;
    static constexpr size_t kADCChannelCount = 10;

    template <ADCCalibrationState calibrationState>
    class ADCOneshot : public std::enable_shared_from_this<ADCOneshot<calibrationState>> {
    public:
        // Construct ADCOneshot by requesting one from an ESP32.
        ~ADCOneshot();

        template <ADCCalibrationState C = calibrationState, typename = std::enable_if_t<!C>>
        ADCOneshotChannelPtr<calibrationState> channel(const ADCChannelConfig& config, esp_err_t& err);

        template <ADCCalibrationState C = calibrationState, typename = std::enable_if_t<C>>
        ADCOneshotChannelPtr<calibrationState> channel(adc_channel_t channel, esp_err_t& err);

        static std::pair<adc_unit_t, adc_channel_t> channelForGPIO(gpio_num_t gpio, esp_err_t& err);
        static gpio_num_t gpioForChannel(adc_unit_t unit, adc_channel_t channel, esp_err_t& err);

    private:
        template <ADCCalibrationState C = calibrationState, typename = std::enable_if_t<!C>>
        ADCOneshot(adc_unit_t unitId, esp_err_t& err);
        template <ADCCalibrationState C = calibrationState, typename = std::enable_if_t<C>>
        ADCOneshot(ADCCalibrationPtr calibration, esp_err_t& err);

        adc_oneshot_unit_handle_t _handle;
        std::optional<ADCCalibrationPtr> _calibration;

        std::array<std::weak_ptr<ADCOneshotChannel<calibrationState>>, kADCChannelCount> _channels;

        static constexpr char _loggingTag[] = "esp::ADCOneshot";

        friend class ESP32;
        friend class ADCOneshotChannel<calibrationState>;
    };

    //
    // IMPLEMENTATION
    //

    template <ADCCalibrationState calibration>
    uint16_t ADCOneshotChannel<calibration>::read() {
        int rawValue = 0;
        adc_oneshot_read(_adc->_handle, _config.channel, &rawValue);
        return static_cast<uint16_t>(rawValue);
    }

    template <ADCCalibrationState calibration>
    template <ADCCalibrationState C, typename>
    uint16_t ADCOneshotChannel<calibration>::miliVolts() {
        int mV = 0;
        adc_oneshot_get_calibrated_result(_adc->_handle, _adc->_calibration.value()->_calibration, _config.channel, &mV);
        return static_cast<uint16_t>(mV);
    }

    template <ADCCalibrationState calibration>
    template <ADCCalibrationState C, typename>
    ADCOneshotChannel<calibration>::ADCOneshotChannel(ADCOneshotPtr<calibration> adc, const ADCChannelConfig& config, esp_err_t& err)
        : _adc(adc), _config(config) {
        _handle = adc->_handle;
        _channel = config.channel;
        err = adc_oneshot_config_channel(_adc->_handle, config.channel, &config.configuration);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_oneshot_config_channel failed: %s", esp_err_to_name(err));
        }
    }

    template <ADCCalibrationState calibration>
    template <ADCCalibrationState C, typename>
    ADCOneshotChannel<calibration>::ADCOneshotChannel(ADCOneshotPtr<calibration> adc, adc_channel_t channel, esp_err_t& err) : _adc(adc) {
        _config.channel = channel;
        _config.configuration.atten = adc->_calibration.value()->_attenuation;
        _config.configuration.bitwidth = adc->_calibration.value()->_bitwidth;
        _handle = adc->_handle;
        _channel = channel;
        _calibrationHandle = adc->_calibration.value()->_calibration;
        err = adc_oneshot_config_channel(_adc->_handle, channel, &_config.configuration);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_oneshot_config_channel failed: %s", esp_err_to_name(err));
        }
    }

    template <ADCCalibrationState calibration>
    template <ADCCalibrationState C, typename>
    ADCOneshotChannelPtr<calibration> ADCOneshot<calibration>::channel(const ADCChannelConfig& config, esp_err_t& err) {
        ADCOneshotChannelPtr<calibration> chan = _channels[config.channel].lock();
        if (chan == nullptr) {
            chan = std::shared_ptr<ADCOneshotChannel<calibration>>(new ADCOneshotChannel<calibration>(this->shared_from_this(), config, err));
            _channels[config.channel] = std::weak_ptr<ADCOneshotChannel<calibration>>(chan);
            return chan;
        }

        if (chan->_config == config) {
            return chan;
        }

        err = ESP_ERR_INVALID_STATE;

        return nullptr;
    }

    template <ADCCalibrationState calibration>
    template <ADCCalibrationState C, typename>
    ADCOneshotChannelPtr<calibration> ADCOneshot<calibration>::channel(adc_channel_t channel, esp_err_t& err) {
        ADCOneshotChannelPtr<calibration> chan = _channels[channel].lock();
        if (chan == nullptr) {
            chan = std::shared_ptr<ADCOneshotChannel<calibration>>(new ADCOneshotChannel<calibration>(this->shared_from_this(), channel, err));
            _channels[channel] = std::weak_ptr<ADCOneshotChannel<calibration>>(chan);
            return chan;
        }

        if (chan->_config.configuration.atten == _calibration.value()->_attenuation &&
            chan->_config.configuration.bitwidth == _calibration.value()->_bitwidth) {
            return chan;
        }

        err = ESP_ERR_INVALID_STATE;

        return nullptr;
    }

    template <ADCCalibrationState calibrationState>
    template <ADCCalibrationState C, typename>
    ADCOneshot<calibrationState>::ADCOneshot(adc_unit_t unitId, esp_err_t& err) {
        adc_oneshot_unit_init_cfg_t config = {.unit_id = unitId, .clk_src = ADC_RTC_CLK_SRC_DEFAULT, .ulp_mode = ADC_ULP_MODE_DISABLE};
        err = adc_oneshot_new_unit(&config, &_handle);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_oneshot_new_unit failed: %s", esp_err_to_name(err));
        }
    }

    template <ADCCalibrationState calibrationStatus>
    template <ADCCalibrationState C, typename>
    ADCOneshot<calibrationStatus>::ADCOneshot(ADCCalibrationPtr calibration, esp_err_t& err) {
        adc_oneshot_unit_init_cfg_t config = {.unit_id = calibration->_unit, .clk_src = ADC_RTC_CLK_SRC_DEFAULT, .ulp_mode = ADC_ULP_MODE_DISABLE};
        err = adc_oneshot_new_unit(&config, &_handle);
        _calibration = calibration;
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_oneshot_new_unit failed: %s", esp_err_to_name(err));
        }
    }

    template <ADCCalibrationState calibration>
    ADCOneshot<calibration>::~ADCOneshot() {
        esp_err_t err = ESP_OK;
        err = adc_oneshot_del_unit(_handle);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_oneshot_del_unit failed: %s", esp_err_to_name(err));
        }
    }

    template <ADCCalibrationState calibration>
    std::pair<adc_unit_t, adc_channel_t> ADCOneshot<calibration>::channelForGPIO(gpio_num_t gpio, esp_err_t& err) {
        adc_unit_t unit = ADC_UNIT_1;
        adc_channel_t channel = ADC_CHANNEL_0;
        err = adc_oneshot_io_to_channel(gpio, &unit, &channel);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_oneshot_io_to_channel failed: %s", esp_err_to_name(err));
        }
        return std::pair<adc_unit_t, adc_channel_t>(unit, channel);
    }

    template <ADCCalibrationState calibration>
    gpio_num_t ADCOneshot<calibration>::gpioForChannel(adc_unit_t unit, adc_channel_t channel, esp_err_t& err) {
        gpio_num_t gpio = GPIO_NUM_0;
        err = adc_oneshot_channel_to_io(unit, channel, reinterpret_cast<int*>(&gpio));
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_channel_to_io failed: %s", esp_err_to_name(err));
        }

        return gpio;
    }
}  // namespace esp
