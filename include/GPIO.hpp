/*
 * GPIO.hpp
 *
 * (c) Tom Davie 29/11/2025 
 * 
 */

#pragma once

#include <driver/gpio.h>

#include <functional>
#include <memory>

namespace esp {
    struct GPIOMode {
    public:
        constexpr GPIOMode() : _input(false), _output(false), _openDrain(false) {}

        constexpr GPIOMode(gpio_mode_t mode)
            : _input(mode == GPIO_MODE_INPUT || mode == GPIO_MODE_INPUT_OUTPUT || mode == GPIO_MODE_INPUT_OUTPUT_OD),
              _output(mode == GPIO_MODE_OUTPUT || mode == GPIO_MODE_INPUT_OUTPUT || mode == GPIO_MODE_OUTPUT_OD),
              _openDrain(mode == GPIO_MODE_OUTPUT_OD || mode == GPIO_MODE_INPUT_OUTPUT_OD) {}

        constexpr GPIOMode(bool input, bool output, bool openDrain) : _input(input), _output(output), _openDrain(openDrain) {}

        bool input() const { return _input; }

        void setInput(bool input) { _input = input; }

        bool output() const { return _output; }

        void setOutput(bool output) { _output = output; }

        bool openDrain() const { return _openDrain; }

        void setOpenDrain(bool openDrain) { _openDrain = openDrain; }

        gpio_mode_t gpioMode() const {
            if (_input && _output) {
                return _openDrain ? GPIO_MODE_INPUT_OUTPUT_OD : GPIO_MODE_INPUT_OUTPUT;
            } else if (_input) {
                return GPIO_MODE_INPUT;
            } else if (_output) {
                return _openDrain ? GPIO_MODE_OUTPUT_OD : GPIO_MODE_OUTPUT;
            } else {
                return GPIO_MODE_DISABLE;
            }
        }

    private:
        bool _input : 1;
        bool _output : 1;
        bool _openDrain : 1;
    };

    static constexpr GPIOMode GPIOModeDisable = GPIOMode(false, false, false);
    static constexpr GPIOMode GPIOModeInput = GPIOMode(true, false, false);
    static constexpr GPIOMode GPIOModeOutput = GPIOMode(false, true, false);
    static constexpr GPIOMode GPIOModeInputOutput = GPIOMode(true, true, false);
    static constexpr GPIOMode GPIOModeInputOutputOpenDrain = GPIOMode(true, true, true);
    static constexpr GPIOMode GPIOModeOutputOpenDrain = GPIOMode(false, true, true);

    enum class PullUp : uint8_t {
        Disable = 0,
        Enable
    };

    enum class PullDown : uint8_t {
        Disable = 0,
        Enable
    };

    enum class GPIOInteruptType : uint8_t {
        Disable = GPIO_INTR_DISABLE,
        AnyEdge = GPIO_INTR_ANYEDGE,
        LowLevel = GPIO_INTR_LOW_LEVEL,
        HighLevel = GPIO_INTR_HIGH_LEVEL,
        NegativeEdge = GPIO_INTR_NEGEDGE,
        PositiveEdge = GPIO_INTR_POSEDGE
    };

    int interuptFlags(GPIOInteruptType type);

    void _interruptHandler(void* arg);

    using GPIOInteruptCallback = std::function<void()>;

    struct GPIOConfig {
    public:
        GPIOConfig(gpio_num_t num, GPIOMode m, PullUp pu = PullUp::Disable, PullDown pd = PullDown::Disable)
            : gpioNum(num), mode(m), pullUp(pu), pullDown(pd) {}

        gpio_num_t gpioNum;
        GPIOMode mode = {};
        PullUp pullUp = PullUp::Disable;
        PullDown pullDown = PullDown::Disable;
        GPIOInteruptType interuptType = GPIOInteruptType::Disable;
    };

    bool operator==(const GPIOMode& a, const GPIOMode& b);
    bool operator==(const GPIOConfig& a, const GPIOConfig& b);

    class GPIO {
    public:
        ~GPIO();

        const GPIOConfig& config() const;
        void setConfig(const GPIOConfig& config, esp_err_t& err);

        GPIOMode mode() const;
        void setMode(GPIOMode mode, esp_err_t& err);

        PullUp pullUp() const;
        void setPullUp(PullUp pullUp, esp_err_t& err);

        PullDown pullDown() const;
        void setPullDown(PullDown pullDown, esp_err_t& err);

        GPIOInteruptType interruptType() const;
        void setInterrupt(GPIOInteruptType type, GPIOInteruptCallback callback, esp_err_t& err);

        bool level();
        void setLevel(bool level, esp_err_t& err);

    private:
        GPIO(const GPIOConfig& gpioConfig, esp_err_t& err);

        GPIOConfig _config;
        GPIOInteruptCallback _interuptCallback;

        friend void esp::_interruptHandler(void* arg);

        static constexpr char _loggingTag[] = "esp::GPIO";

        friend class ESP32;
    };

    using GPIOPtr = std::shared_ptr<GPIO>;
}  // namespace esp
