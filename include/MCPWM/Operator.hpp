/*
 * Operator.hpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#pragma once

#include "Interrupt.hpp"

#include <driver/mcpwm_oper.h>

#include <memory>
#include <vector>
#include <optional>

namespace esp {
    namespace mcpwm {
        struct OperatorConfig {
            InterruptPriority interruptPriority = Default;

            bool updateGeneratorActionOnTimerZero = false;
            bool updateGeneratorActionOnTimerPeak = false;
            bool updateGeneratorActionOnSync = false;
        };

        class Operator;
        using OperatorPtr = std::shared_ptr<Operator>;

        class ComparatorConfig;
        class Comparator;
        using ComparatorPtr = std::shared_ptr<Comparator>;

        class GeneratorConfig;
        class Generator;
        using GeneratorPtr = std::shared_ptr<Generator>;

        class Timer;
        using TimerPtr = std::shared_ptr<Timer>;

        struct CarrierWaveConfig {
            mcpwm_carrier_clock_source_t clockSource = MCPWM_CARRIER_CLK_SRC_DEFAULT;
            uint32_t frequency;
            uint32_t firstPulseTicks = 0;
            float dutyCycle = 0.0f;
            bool invertBeforeModulation = false;
            bool invertAfterModulation = false;
        };

        class Operator : public std::enable_shared_from_this<Operator> {
        public:
            ~Operator();

            ComparatorPtr addComparator(const ComparatorConfig& config, esp_err_t& err);
            GeneratorPtr addGenerator(const GeneratorConfig& config, esp_err_t& err);

            void removeComparator(const ComparatorPtr& comparator);
            void removeGenerator(const GeneratorPtr& generator);

            void enableCarrierWave(const CarrierWaveConfig& config, esp_err_t& err);
            void setCarrierDutyCycle(float dutyCycle, esp_err_t& err);
            void disableCarrierWave(esp_err_t& err);

        private:
            Operator(TimerPtr timer, const OperatorConfig& config, esp_err_t& err);

            mcpwm_oper_handle_t _operator;
            std::optional<CarrierWaveConfig> _carrierConfig;

            std::weak_ptr<Timer> _timer;
            std::vector<GeneratorPtr> _generators;
            std::vector<ComparatorPtr> _comparators;

            static constexpr char _loggingTag[] = "esp::mcpwm::Operator";

            friend class Timer;
            friend class Comparator;
            friend class Generator;
        };
    }  // namespace mcpwm
}  // namespace esp
