/*
 * Interrupt.hpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#pragma once

#include <cstdint>

namespace esp {
    enum InterruptPriority : uint8_t {
        Default = 0,
        Low,
        MediumLow,
        MediumHigh,
        High,
        __ESP_IDF_Debug_High,
        NonMaskable,
        __ESP_IDF_Debug_Exception
    };

    enum class InterruptResult : uint8_t {
        NoHighPriorityTaskWoken = false,
        HighPriorityTaskWoken = true
    };
}  // namespace esp
