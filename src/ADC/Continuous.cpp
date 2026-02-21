#include <ADC/Continuous.hpp>

#include <esp_log.h>

using namespace esp;
using namespace esp::adc;

namespace esp {
    namespace adc {
        bool _onConversionComplete(adc_continuous_handle_t handle, const adc_continuous_evt_data_t* data, void* userData) {
            ADCContinuousPtr adc = static_cast<ADCContinuous*>(userData)->shared_from_this();

            const std::span<uint8_t> conversionData(data->conv_frame_buffer, data->size);

            if (adc->_callbacks.onConversionComplete) {
                adc->_callbacks.onConversionComplete(adc, conversionData);
            }

            if (adc->_callbacks.onParsedConversionComplete) {
                esp_err_t err = ESP_OK;
                std::vector<adc_continuous_data_t> parsedData = adc->parse(conversionData, err);
                if (err != ESP_OK) {
                    ESP_LOGE(adc->_loggingTag, "Failed to parse conversion data in callback: %s", esp_err_to_name(err));
                    return false;
                }
                adc->_callbacks.onParsedConversionComplete(adc, parsedData);
            }
            return false;
        }

        bool _onPoolOverflow(adc_continuous_handle_t handle, const adc_continuous_evt_data_t* data, void* userData) {
            ADCContinuousPtr adc = static_cast<ADCContinuous*>(userData)->shared_from_this();

            if (adc->_callbacks.onPoolOverflow) {
                adc->_callbacks.onPoolOverflow(adc);
            }
            return false;
        }
    }  // namespace adc
}  // namespace esp

ADCContinuous::~ADCContinuous() {
    esp_err_t err = ESP_OK;

    if (_started) {
        err = adc_continuous_stop(_handle);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "adc_continuous_stop failed: %s", esp_err_to_name(err));
        }
    }

    err = adc_continuous_deinit(_handle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_continuous_deinit failed: %s", esp_err_to_name(err));
        return;
    }
}

esp_err_t ADCContinuous::setEventCallbacks(const ADCContinuousEventCallbacks& callbacks) {
    adc_continuous_evt_cbs_t callbackConfig = {
        .on_conv_done = (callbacks.onConversionComplete || callbacks.onParsedConversionComplete) ? _onConversionComplete : nullptr,
        .on_pool_ovf = callbacks.onPoolOverflow ? _onPoolOverflow : nullptr,
    };
    _callbacks = callbacks;
    esp_err_t err = adc_continuous_register_event_callbacks(_handle, &callbackConfig, this);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_continuous_register_event_callbacks failed: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t ADCContinuous::start() {
    esp_err_t err = adc_continuous_start(_handle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_continuous_start failed: %s", esp_err_to_name(err));
    }

    _started = true;

    return err;
}

esp_err_t ADCContinuous::stop() {
    if (!_started) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = adc_continuous_stop(_handle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_continuous_stop failed: %s", esp_err_to_name(err));
    }

    _started = false;

    return err;
}

esp_err_t ADCContinuous::flush() {
    esp_err_t err = adc_continuous_flush_pool(_handle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_continuous_flush_pool failed: %s", esp_err_to_name(err));
    }
    return err;
}

ADCContinuous::ADCContinuous(const ADCContinuousConfig& config, esp_err_t& err) {
    const adc_continuous_handle_cfg_t continuousConfig = {.max_store_buf_size = config.maximumStorageBytes,
                                                          .conv_frame_size = config.conversionFrameBytes,
                                                          .flags{
                                                              .flush_pool = config.flushWhenFull,
                                                          }};

    err = adc_continuous_new_handle(&continuousConfig, &_handle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_continuous_new_handle failed: %s", esp_err_to_name(err));
        return;
    }

    std::vector<adc_digi_pattern_config_t> channelConfigs;
    for (const ADCContinuousChannelConfig& channelConfig : config.channels) {
        channelConfigs.push_back((adc_digi_pattern_config_t){
            .atten = static_cast<uint8_t>(channelConfig.attenuation),
            .channel = channelConfig.channel,
            .unit = channelConfig.unit,
            .bit_width = static_cast<uint8_t>(channelConfig.bitwidth),
        });
    }
    const adc_continuous_config_t driverConfig = {
        .pattern_num = static_cast<uint8_t>(config.channels.size()),
        .adc_pattern = channelConfigs.data(),
        .sample_freq_hz = config.samplingFrequencyHz,
        .conv_mode = static_cast<adc_digi_convert_mode_t>(config.conversionMode),
        .format = static_cast<adc_digi_output_format_t>(config.outputFormat),
    };

    err = adc_continuous_config(_handle, &driverConfig);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "adc_continuous_config failed: %s", esp_err_to_name(err));
        adc_continuous_deinit(_handle);
        return;
    }
}
