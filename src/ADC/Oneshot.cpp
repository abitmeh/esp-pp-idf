/*
 * ADC.cpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#include "ADC/Oneshot.hpp"

#include <esp_err.h>

using namespace esp;
using namespace esp::adc;

ADCChannelConfig::ADCChannelConfig() {}

ADCChannelConfig::ADCChannelConfig(adc_channel_t chan, Attenuation attenuation, BitWidth bitwidth) : channel(chan), attenuation(attenuation), bitwidth(bitwidth) {}

namespace esp {
    namespace adc {
        bool operator==(const ADCChannelConfig& a, const ADCChannelConfig& b) {
            return a.channel == b.channel && a.attenuation == b.attenuation && a.bitwidth == b.bitwidth;
        }

        uint16_t IRAM_ATTR readIsr(ADCOneshotChannel<Uncalibrated>* channel) {
            int rawValue = 0;
            adc_oneshot_read_isr(channel->_handle, channel->_channel, &rawValue);
            return static_cast<uint16_t>(rawValue);
        }

        uint16_t IRAM_ATTR readIsr(ADCOneshotChannel<Calibrated>* channel) {
            int rawValue = 0;
            adc_oneshot_read_isr(channel->_handle, channel->_channel, &rawValue);
            return static_cast<uint16_t>(rawValue);
        }
    }
}  // namespace esp
