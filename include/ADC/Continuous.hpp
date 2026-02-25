#pragma once

#include "ADC/Types.hpp"
#include "Interrupt.hpp"

#include <esp_adc/adc_continuous.h>

#include <functional>
#include <memory>
#include <ranges>
#include <span>
#include <vector>

namespace esp {
    class ESP32;

    namespace adc {
        class ADCContinuous;
        using ADCContinuousPtr = std::shared_ptr<ADCContinuous>;

        using ADCContinuousConversionCallback = InterruptResult(*)(const uint8_t* conversionData, size_t count, void* userInfo);
        using ADCContinuousPoolOverflowCallback = InterruptResult(*)(void* userInfo);

        struct ADCContinuousEventCallbacks {
            ADCContinuousConversionCallback onConversionComplete = nullptr;
            ADCContinuousPoolOverflowCallback onPoolOverflow = nullptr;
        };

        struct ADCContinuousChannelConfig {
            adc_unit_t unit;
            adc_channel_t channel;
            Attenuation attenuation;
            BitWidth bitwidth;
        };

        enum class ConversionMode : uint8_t {
            SingleUnit1 = ADC_CONV_SINGLE_UNIT_1,
            SingleUnit2 = ADC_CONV_SINGLE_UNIT_2,
            BothUnit = ADC_CONV_BOTH_UNIT,
            AlterUnit = ADC_CONV_ALTER_UNIT,
        };

        enum class OutputFormat : uint8_t {
            Type1 = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
            Type2 = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
        };

        struct ADCContinuousConfig {
            size_t maximumStoredValues;
            size_t numberOfValuesPerConversionFrame;
            bool flushWhenFull = true;

            std::vector<ADCContinuousChannelConfig> channels{};
            uint32_t samplingFrequencyHz;
            ConversionMode conversionMode = ConversionMode::SingleUnit1;
            OutputFormat outputFormat = OutputFormat::Type1;
        };

        bool _onConversionComplete(adc_continuous_handle_t handle, const adc_continuous_evt_data_t* data, void* userData);
        bool _onPoolOverflow(adc_continuous_handle_t handle, const adc_continuous_evt_data_t* data, void* userData);

        class ADCContinuous : public std::enable_shared_from_this<ADCContinuous> {
        public:
            ~ADCContinuous();

            esp_err_t setEventCallbacks(const ADCContinuousEventCallbacks& callbacks, void* userInfo);

            esp_err_t start();
            esp_err_t stop();

            template <size_t bufferSize>
            std::vector<uint8_t> read(uint32_t timeoutMs, esp_err_t& err);
            template <size_t maxSamples>
            std::vector<adc_continuous_data_t> readParsed(uint32_t timeoutMs, esp_err_t& err);
            template <std::ranges::viewable_range R>
            std::vector<adc_continuous_data_t> parse(const R& rawData, esp_err_t& err) const;
            void parse(const uint8_t* rawData, size_t count, adc_continuous_data_t* parsedData, esp_err_t& err) const;

            esp_err_t flush();

        private:
            ADCContinuous(const ADCContinuousConfig& config, esp_err_t& err);

            adc_continuous_handle_t _handle;
            ADCContinuousEventCallbacks _callbacks;
            std::pair<ADCContinuous*, void*> _userInfo;
            uint32_t _maxStoreBufSize = 1024;
            uint32_t _convFrameSize = 512;
            bool _started = false;

            static constexpr char _loggingTag[] = "esp::ADCContinuous";

            friend class esp::ESP32;
            friend bool _onConversionComplete(adc_continuous_handle_t handle, const adc_continuous_evt_data_t* data, void* userData);
            friend bool _onPoolOverflow(adc_continuous_handle_t handle, const adc_continuous_evt_data_t* data, void* userData);
        };

        //
        // IMPLEMENTATION
        //
        template <size_t bufferSize>
        std::vector<uint8_t> ADCContinuous::read(uint32_t timeoutMs, esp_err_t& err) {
            uint8_t buffer[bufferSize];
            uint32_t bytesRead = 0;
            err = adc_continuous_read(_handle, buffer, bufferSize, &bytesRead, timeoutMs);
            std::vector<uint8_t> rawData(buffer, buffer + bytesRead);
            if (err != ESP_OK) {
                return {};
            }
            return rawData;
        }

        template <size_t maxSamples>
        std::vector<adc_continuous_data_t> ADCContinuous::readParsed(uint32_t timeoutMs, esp_err_t& err) {
            std::vector<adc_continuous_data_t> parsedData(maxSamples);
            uint32_t samplesRead = 0;
            err = adc_continuous_read_parse(_handle, parsedData.data(), maxSamples, &samplesRead, timeoutMs);
            if (err != ESP_OK) {
                return {};
            }
            parsedData.resize(samplesRead);
            return parsedData;
        }

        template <std::ranges::viewable_range R>
        std::vector<adc_continuous_data_t> ADCContinuous::parse(const R& rawData, esp_err_t& err) const {
            std::vector<adc_continuous_data_t> parsedData(rawData.size());
            uint32_t numParsedSamples = 0;
            err = adc_continuous_parse_data(_handle, rawData.data(), rawData.size(), parsedData.data(), &numParsedSamples);
            if (err != ESP_OK) {
                return {};
            }
            parsedData.resize(numParsedSamples);
            return parsedData;
        }

        inline void ADCContinuous::parse(const uint8_t* rawData, size_t count, adc_continuous_data_t* parsedData, esp_err_t& err) const {
            uint32_t numParsedSamples = 0;
            err = adc_continuous_parse_data(_handle, rawData, count, parsedData, &numParsedSamples);
            if (err != ESP_OK) {
                return;
            }
        }
    }  // namespace adc
}  // namespace esp
