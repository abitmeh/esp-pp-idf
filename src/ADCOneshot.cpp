/*
 * ADC.cpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#include "ADCOneshot.hpp"

#include <esp_err.h>

using namespace esp;

ADCChannelConfig::ADCChannelConfig() {}

ADCChannelConfig::ADCChannelConfig(adc_channel_t chan, adc_oneshot_chan_cfg_t config) : channel(chan), configuration(config) {}

namespace esp {
    bool operator==(const ADCChannelConfig& a, const ADCChannelConfig& b) {
        return a.channel == b.channel && a.configuration.atten == b.configuration.atten && a.configuration.bitwidth == b.configuration.bitwidth;
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

    uint16_t IRAM_ATTR miliVoltsIsr(ADCOneshotChannel<Calibrated>* channel) {
        int mV = 0;
        adc_oneshot_get_calibrated_result(channel->_handle, channel->_calibrationHandle, channel->_channel, &mV);
        return static_cast<uint16_t>(mV);
    }

}  // namespace esp
