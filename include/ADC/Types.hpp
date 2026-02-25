#pragma once

#include <hal/adc_types.h>

namespace esp {
    namespace adc {
        enum class Attenuation : uint8_t {
            None = ADC_ATTEN_DB_0,
            Decibels2_5 = ADC_ATTEN_DB_2_5,
            Decibels6 = ADC_ATTEN_DB_6,
            Decibels12 = ADC_ATTEN_DB_12,
        };

        enum class BitWidth : uint8_t {
            Default = ADC_BITWIDTH_DEFAULT,
            Bits9 = ADC_BITWIDTH_9,
            Bits10 = ADC_BITWIDTH_10,
            Bits11 = ADC_BITWIDTH_11,
            Bits12 = ADC_BITWIDTH_12,
            Bits13 = ADC_BITWIDTH_13,
        };
    }
}