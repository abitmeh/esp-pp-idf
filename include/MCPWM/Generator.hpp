/*
 * Generator.hpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#pragma once

#include "MCPWM/Timer.hpp"
#include "Enums.hpp"

#include <driver/gpio.h>
#include <driver/mcpwm_gen.h>

#include <initializer_list>
#include <memory>

namespace esp {
    namespace mcpwm {
        struct GeneratorConfig {
            gpio_num_t gpioNum;

            bool invertPwm = false;
            bool ioLoopBack = false;
            bool ioOpenDrain = false;
            bool pullUp = false;
            bool pullDown = false;
        };

        class Generator;
        using GeneratorPtr = std::shared_ptr<Generator>;

        class Comparator;
        using ComparatorPtr = std::shared_ptr<Comparator>;

        class Operator;
        using OperatorPtr = std::shared_ptr<Operator>;

        enum class GeneratorAction : uint8_t {
            None = MCPWM_GEN_ACTION_KEEP,
            Low = MCPWM_GEN_ACTION_LOW,
            High = MCPWM_GEN_ACTION_HIGH,
            Toggle = MCPWM_GEN_ACTION_TOGGLE
        };

        class Generator {
        public:
            struct TimerEventAction {
                TimerDirection direction;
                TimerEvent event;
                GeneratorAction action;
            };

            struct CompareEventAction {
                TimerDirection direction;
                ComparatorPtr comparator;
                GeneratorAction action;
            };

            ~Generator();

            void setActionOnTimerEvent(const TimerEventAction& action, esp_err_t& err);
            void setActionsOnTimerEvent(std::initializer_list<TimerEventAction> actions, esp_err_t& err);

            void setActionOnCompareEvent(const CompareEventAction& action, esp_err_t& err);
            void setActionsOnCompareEvent(std::initializer_list<CompareEventAction> actions, esp_err_t& err);

            void setLevel(Level level, bool overrideGeneratorActions, esp_err_t& err);

        private:
            Generator(OperatorPtr oper, const GeneratorConfig& config, esp_err_t& err);

            mcpwm_gen_handle_t _generator;

            std::weak_ptr<Operator> _operator;

            static constexpr char _loggingTag[] = "esp::mcpwm::Generator";

            friend class Operator;
        };
    }  // namespace mcpwm
}  // namespace esp
