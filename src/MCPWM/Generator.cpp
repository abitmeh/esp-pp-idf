/*
 * Generator.cpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#include "MCPWM/Generator.hpp"

#include "MCPWM/Comparator.hpp"
#include "MCPWM/Operator.hpp"

#include <esp_log.h>

using namespace esp;
using namespace mcpwm;

Generator::Generator(OperatorPtr oper, const GeneratorConfig& config, esp_err_t& err) : _operator(oper) {
    mcpwm_generator_config_t generatorConfig = {.gen_gpio_num = config.gpioNum,
                                                .flags{
                                                    .invert_pwm = false,
                                                }};
    err = mcpwm_new_generator(oper->_operator, &generatorConfig, &_generator);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_new_generator failed: %s", esp_err_to_name(err));
        return;
    }
}

Generator::~Generator() {
    esp_err_t err = ESP_OK;
    err = mcpwm_del_generator(_generator);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_del_generator failed: %s", esp_err_to_name(err));
        return;
    }
}

void Generator::setActionOnTimerEvent(const TimerEventAction& action, esp_err_t& err) {
    mcpwm_gen_timer_event_action_t mcpwmAction = {
        .direction = static_cast<mcpwm_timer_direction_t>(action.direction),
        .event = static_cast<mcpwm_timer_event_t>(action.event),
        .action = static_cast<mcpwm_generator_action_t>(action.action)
    };
    err = mcpwm_generator_set_action_on_timer_event(_generator, mcpwmAction);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_generator_set_action_on_timer_event failed: %s", esp_err_to_name(err));
        return;
    }
}

void Generator::setActionsOnTimerEvent(std::initializer_list<TimerEventAction> actions, esp_err_t& err) {
    for (const TimerEventAction& action : actions) {
        setActionOnTimerEvent(action, err);
        if (err != ESP_OK) {
            return;
        }
    }
}

void Generator::setActionOnCompareEvent(const CompareEventAction& action, esp_err_t& err) {
    mcpwm_gen_compare_event_action_t mcpwmAction = {
        .direction = static_cast<mcpwm_timer_direction_t>(action.direction),
        .comparator = action.comparator->_comparator,
        .action = static_cast<mcpwm_generator_action_t>(action.action)
    };
    err = mcpwm_generator_set_action_on_compare_event(_generator, mcpwmAction);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_generator_set_action_on_compare_event failed: %s", esp_err_to_name(err));
        return;
    }
}

void Generator::setActionsOnCompareEvent(std::initializer_list<CompareEventAction> actions, esp_err_t& err) {
    for (const CompareEventAction& action : actions) {
        setActionOnCompareEvent(action, err);
        if (err != ESP_OK) {
            return;
        }
    }
}

void Generator::setLevel(Level level, bool overrideGeneratorActions, esp_err_t& err) {
    err = mcpwm_generator_set_force_level(_generator, static_cast<int>(level), overrideGeneratorActions);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_generator_set_force_level failed: %s", esp_err_to_name(err));
        return;
    }
}

void Generator::clearLevel(esp_err_t& err) {
    err = mcpwm_generator_set_force_level(_generator, -1, true);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_generator_set_force_level failed: %s", esp_err_to_name(err));
        return;
    }
}
