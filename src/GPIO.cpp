/*
 * GPIO.cpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#include "GPIO.hpp"

#include <esp_log.h>

using namespace esp;

namespace esp {
    bool operator==(const GPIOMode& a, const GPIOMode& b) {
        return a.input() == b.input() && a.output() == b.output() && a.openDrain() == b.openDrain();
    }

    bool operator==(const GPIOConfig& a, const GPIOConfig& b) {
        return a.gpioNum == b.gpioNum && a.mode == b.mode && a.pullUp == b.pullUp && a.pullDown == b.pullDown && a.interuptType == b.interuptType;
    }

    int interuptFlags(GPIOInteruptType type) {
        switch (type) {
            case GPIOInteruptType::Disable:
                return ESP_INTR_FLAG_INTRDISABLED;
            case GPIOInteruptType::LowLevel:
                return 0;
            case GPIOInteruptType::HighLevel:
                return 0;
            case GPIOInteruptType::NegativeEdge:
                return ESP_INTR_FLAG_EDGE;
            case GPIOInteruptType::PositiveEdge:
                return ESP_INTR_FLAG_EDGE;
            default:
                return GPIO_INTR_DISABLE;
        }
    }

    void _interruptHandler(void* arg) {
        GPIO* gpio = static_cast<GPIO*>(arg);
        if (gpio->_interuptCallback) {
            gpio->_interuptCallback();
        }
    }
}  // namespace esp

GPIO::GPIO(const GPIOConfig& gpioConfig, esp_err_t& err) : _config(gpioConfig) {
    if (!GPIO_IS_VALID_GPIO(gpioConfig.gpioNum)) {
        ESP_LOGE(_loggingTag, "Invalid GPIO: %u", gpioConfig.gpioNum);
        err = ESP_ERR_INVALID_ARG;
        return;
    }

    gpio_config_t config = {
        .pin_bit_mask = (1ull << gpioConfig.gpioNum),
        .mode = gpioConfig.mode.gpioMode(),
        .pull_up_en = static_cast<gpio_pullup_t>(gpioConfig.pullUp),
        .pull_down_en = static_cast<gpio_pulldown_t>(gpioConfig.pullDown),
        .intr_type = static_cast<gpio_int_type_t>(gpioConfig.interuptType),
    };
    err = gpio_config(&config);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_config failed: %s", esp_err_to_name(err));
        return;
    }
    static bool isIsrServiceInstalled = false;
    if (!isIsrServiceInstalled) {
        err = gpio_install_isr_service(ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_EDGE);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "gpio_install_isr_service failed: %s", esp_err_to_name(err));
            return;
        }
        isIsrServiceInstalled = true;
    }
}

GPIO::~GPIO() {
    esp_err_t err = ESP_OK;
    err = gpio_reset_pin(_config.gpioNum);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_reset_pin failed: %s", esp_err_to_name(err));
        return;
    }
}

const GPIOConfig& GPIO::config() const {
    return _config;
}

void GPIO::setConfig(const GPIOConfig& config, esp_err_t& err) {
    gpio_config_t gpioConfig = {
        .pin_bit_mask = (1ull << config.gpioNum),
        .mode = config.mode.gpioMode(),
        .pull_up_en = static_cast<gpio_pullup_t>(config.pullUp),
        .pull_down_en = static_cast<gpio_pulldown_t>(config.pullDown),
        .intr_type = static_cast<gpio_int_type_t>(config.interuptType),
    };
    err = gpio_config(&gpioConfig);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_config failed: %s", esp_err_to_name(err));
        return;
    }

    _config = config;
}

GPIOMode GPIO::mode() const {
    return _config.mode;
}

void GPIO::setMode(GPIOMode mode, esp_err_t& err) {
    gpio_mode_t gpioMode = mode.gpioMode();
    err = gpio_set_direction(_config.gpioNum, gpioMode);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_set_direction failed: %s", esp_err_to_name(err));
        return;
    }

    _config.mode = mode;
}

PullUp GPIO::pullUp() const {
    return _config.pullUp;
}

void GPIO::setPullUp(PullUp pullUp, esp_err_t& err) {
    gpio_pull_mode_t pullMode =
        static_cast<gpio_pull_mode_t>((pullUp == PullUp::Enable ? GPIO_PULLUP_ONLY : 0) | (_config.pullDown == PullDown::Enable ? GPIO_PULLDOWN_ONLY : 0));
    err = gpio_set_pull_mode(_config.gpioNum, pullMode);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_set_pull_mode failed: %s", esp_err_to_name(err));
        return;
    }

    _config.pullUp = pullUp;
}

PullDown GPIO::pullDown() const {
    return _config.pullDown;
}

void GPIO::setPullDown(PullDown pullDown, esp_err_t& err) {
    gpio_pull_mode_t pullMode =
        static_cast<gpio_pull_mode_t>((_config.pullUp == PullUp::Enable ? GPIO_PULLUP_ONLY : 0) | (pullDown == PullDown::Enable ? GPIO_PULLDOWN_ONLY : 0));
    err = gpio_set_pull_mode(_config.gpioNum, pullMode);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_set_pull_mode failed: %s", esp_err_to_name(err));
        return;
    }

    _config.pullDown = pullDown;
}

GPIOInteruptType GPIO::interruptType() const {
    return _config.interuptType;
}

void GPIO::setInterrupt(GPIOInteruptType type, GPIOInteruptCallback callback, esp_err_t& err) {
    if (type == GPIOInteruptType::Disable) {
        err = gpio_isr_handler_remove(_config.gpioNum);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "gpio_isr_del_handler failed: %s", esp_err_to_name(err));
            return;
        }

        err = gpio_intr_disable(_config.gpioNum);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "gpio_intr_disable failed: %s", esp_err_to_name(err));
            return;
        }

        _config.interuptType = type;
        _interuptCallback = nullptr;
        return;
    }

    err = gpio_intr_enable(_config.gpioNum);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_intr_enable failed: %s", esp_err_to_name(err));
        return;
    }

    err = gpio_set_intr_type(_config.gpioNum, static_cast<gpio_int_type_t>(type));
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_set_intr_type failed: %s", esp_err_to_name(err));
        return;
    }

    err = gpio_isr_handler_remove(_config.gpioNum);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_isr_del_handler failed: %s", esp_err_to_name(err));
        return;
    }

    _interuptCallback = callback;
    err = gpio_isr_handler_add(_config.gpioNum, esp::_interruptHandler, this);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_isr_handler_add failed: %s", esp_err_to_name(err));
        return;
    }

    _config.interuptType = type;
}

bool GPIO::level() {
    uint32_t level = gpio_get_level(_config.gpioNum);
    return level != 0;
}

void GPIO::setLevel(bool level, esp_err_t& err) {
    err = gpio_set_level(_config.gpioNum, static_cast<uint32_t>(level));
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "gpio_set_level failed: %s", esp_err_to_name(err));
        return;
    }
}
