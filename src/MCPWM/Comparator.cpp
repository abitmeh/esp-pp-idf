/*
 * Comparator.cpp
 *
 * (c) Tom Davie 03/12/2025 
 * 
 */

#include "MCPWM/Comparator.hpp"

#include "MCPWM/Operator.hpp"

#include <esp_log.h>

using namespace esp;
using namespace esp::mcpwm;

namespace esp {
    namespace mcpwm {
        bool _onCompare(mcpwm_cmpr_handle_t handle, const mcpwm_compare_event_data_t* data, void* userData) {
            auto& [comparator, userInfo] = *reinterpret_cast<std::pair<Comparator*, void*>*>(userData);

            bool highPriorityTaskWoken = false;
            if (comparator->_callback) {
                highPriorityTaskWoken = static_cast<bool>(comparator->_callback(data->compare_ticks, static_cast<TimerDirection>(data->direction), userInfo));
            }
            return highPriorityTaskWoken;
        }
    }
}

Comparator::Comparator(OperatorPtr oper, const ComparatorConfig& config, esp_err_t& err) : _operator(oper) {
    const mcpwm_comparator_config_t comparatorConfig = {.intr_priority = config.interruptPriority,
                                                        .flags = {
                                                            .update_cmp_on_tez = config.updateComparatorOnTimerZero,
                                                            .update_cmp_on_tep = config.updateComparatorOnTimerPeak,
                                                            .update_cmp_on_sync = config.updateComparatorOnSync,
                                                        }};

    err = mcpwm_new_comparator(oper->_operator, &comparatorConfig, &_comparator);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_new_comparator failed: %s", esp_err_to_name(err));
        return;
    }
    setCompareValue(0, err);
}

Comparator::~Comparator() {
    esp_err_t err = ESP_OK;
    err = mcpwm_del_comparator(_comparator);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_del_comparator failed: %s", esp_err_to_name(err));
        return;
    }
}

uint32_t Comparator::compareValue() const {
    return _compareValue;
}

void Comparator::setCompareValue(uint32_t compareValue, esp_err_t& err) {
    err = mcpwm_comparator_set_compare_value(_comparator, compareValue);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_comparator_set_compare_value failed: %s", esp_err_to_name(err));
        return;
    }
}

esp_err_t Comparator::setCallback(const ComparatorCallback& callback, void* userInfo) {
    _callback = callback;
    _userInfo = std::make_pair(this, userInfo);

    mcpwm_comparator_event_callbacks_t callbacks = {
        .on_reach = callback ? _onCompare : nullptr,
    };

    esp_err_t err = mcpwm_comparator_register_event_callbacks(_comparator, &callbacks, &_userInfo);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "mcpwm_comparator_register_event_callbacks failed: %s", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}