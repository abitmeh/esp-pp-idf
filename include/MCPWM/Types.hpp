#pragma once

#include <driver/mcpwm_timer.h>

namespace esp {
    namespace mcpwm {
        enum class TimerDirection : uint8_t {
            Up = MCPWM_TIMER_DIRECTION_UP,
            Down = MCPWM_TIMER_DIRECTION_DOWN,
        };

        enum class TimerEvent : uint8_t {
            Full = MCPWM_TIMER_EVENT_FULL,
            Empty = MCPWM_TIMER_EVENT_EMPTY,
            Invalid = MCPWM_TIMER_EVENT_INVALID
        };
    }
}
