/*
 * Operator.cpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#include "MCPWM/Operator.hpp"

#include "MCPWM/Comparator.hpp"
#include "MCPWM/Generator.hpp"
#include "MCPWM/Timer.hpp"

#include <esp_log.h>

#include <vector>

using namespace esp;
using namespace mcpwm;

Operator::Operator(TimerPtr timer, const OperatorConfig& config, esp_err_t& err) : _timer(timer) {
    const mcpwm_operator_config_t operatorConfig = {.group_id = timer->_config.groupId,
                                                    .intr_priority = config.interruptPriority,
                                                    .flags{
                                                        .update_gen_action_on_tez = config.updateGeneratorActionOnTimerZero,
                                                        .update_gen_action_on_tep = config.updateGeneratorActionOnTimerPeak,
                                                        .update_gen_action_on_sync = config.updateGeneratorActionOnSync,
                                                        .update_dead_time_on_tez = false,
                                                        .update_dead_time_on_tep = false,
                                                        .update_dead_time_on_sync = false,
                                                    }};

    err = mcpwm_new_operator(&operatorConfig, &_operator);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_new_operator failed: %s", esp_err_to_name(err));
        return;
    }
    err = mcpwm_operator_connect_timer(_operator, timer->_timer);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_operator_connect_timer failed: %s", esp_err_to_name(err));
        return;
    }
}

Operator::~Operator() {
    esp_err_t err = ESP_OK;
    err = mcpwm_del_operator(_operator);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_del_operator failed: %s", esp_err_to_name(err));
        return;
    }
}

ComparatorPtr Operator::addComparator(const ComparatorConfig& config, esp_err_t& err) {
    ComparatorPtr comparator = std::shared_ptr<Comparator>(new Comparator(shared_from_this(), config, err));

    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm::Comparator::Comparator failed: %s", esp_err_to_name(err));
        return nullptr;
    }
    if (comparator == nullptr) {
        err = ESP_FAIL;
        ESP_LOGE(_loggingTag, "mcpwm::Comparator::Comparator failed: %s", esp_err_to_name(err));
        return nullptr;
    }
    _comparators.push_back(comparator);

    return comparator;
}

GeneratorPtr Operator::addGenerator(const GeneratorConfig& config, esp_err_t& err) {
    GeneratorPtr generator = std::shared_ptr<Generator>(new Generator(shared_from_this(), config, err));

    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm::Generator::Generator failed: %s", esp_err_to_name(err));
        return nullptr;
    }
    if (generator == nullptr) {
        err = ESP_FAIL;
        ESP_LOGE(_loggingTag, "mcpwm::Generator::Generator failed: %s", esp_err_to_name(err));
        return nullptr;
    }
    _generators.push_back(generator);

    return generator;
}

void Operator::removeComparator(const ComparatorPtr& comparator) {
    std::erase(_comparators, comparator);
}

void Operator::removeGenerator(const GeneratorPtr& generator) {
    std::erase(_generators, generator);
}

void Operator::enableCarrierWave(const CarrierWaveConfig& config, esp_err_t& err) {
    mcpwm_carrier_config_t carrierConfig = {.clk_src = config.clockSource,
                                            .frequency_hz = config.frequency,
                                            .first_pulse_duration_us = config.firstPulseTicks * config.frequency / 1'000'000,
                                            .duty_cycle = config.dutyCycle,
                                            .flags{
                                                .invert_before_modulate = config.invertBeforeModulation,
                                                .invert_after_modulate = config.invertAfterModulation,
                                            }};
    err = mcpwm_operator_apply_carrier(_operator, &carrierConfig);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_operator_enable_carrier_wave failed: %s", esp_err_to_name(err));
        return;
    }

    _carrierConfig = config;
}

void Operator::setCarrierDutyCycle(float dutyCycle, esp_err_t& err) {
    if (!_carrierConfig.has_value()) {
        err = ESP_ERR_INVALID_STATE;
        ESP_LOGE(_loggingTag, "Carrier wave not enabled");
        return;
    }

    mcpwm_carrier_config_t carrierConfig = {.clk_src = _carrierConfig->clockSource,
                                            .frequency_hz = _carrierConfig->frequency,
                                            .first_pulse_duration_us = _carrierConfig->firstPulseTicks * _carrierConfig->frequency / 1'000'000,
                                            .duty_cycle = dutyCycle,
                                            .flags{
                                                .invert_before_modulate = _carrierConfig->invertBeforeModulation,
                                                .invert_after_modulate = _carrierConfig->invertAfterModulation,
                                            }};
    err = mcpwm_operator_apply_carrier(_operator, &carrierConfig);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_operator_set_carrier_duty_cycle failed: %s", esp_err_to_name(err));
        return;
    }

    _carrierConfig->dutyCycle = dutyCycle;
}

void Operator::disableCarrierWave(esp_err_t& err) {
    err = mcpwm_operator_apply_carrier(_operator, nullptr);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_operator_disable_carrier_wave failed: %s", esp_err_to_name(err));
        return;
    }
}