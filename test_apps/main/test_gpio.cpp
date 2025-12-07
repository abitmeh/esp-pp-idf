extern "C" {
#include <unity.h>
}
#include <freertos/FreeRTOS.h>

#include "ESP32.hpp"
#include "GPIO.hpp"

using namespace esp;

TEST_CASE("Create and destroy", "[GPIO]") {
    esp_err_t err = ESP_OK;
    GPIOPtr gpio2 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_2, GPIOModeInputOutput, PullUp::Disable, PullDown::Disable), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio2);
    GPIOPtr gpio4 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_4, GPIOModeInputOutput, PullUp::Disable, PullDown::Disable), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio4);
    TEST_ASSERT_NOT_EQUAL(gpio2.get(), gpio4.get());
    GPIOPtr gpio2Again = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_2, GPIOModeInputOutput, PullUp::Disable, PullDown::Disable), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio2Again);
    TEST_ASSERT_EQUAL(gpio2.get(), gpio2Again.get());
}

TEST_CASE("Set and get level", "[GPIO]") {
    esp_err_t err = ESP_OK;
    GPIOPtr gpio2 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_2, GPIOModeInputOutput, PullUp::Disable, PullDown::Disable), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio2);
    gpio2->setLevel(true, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    bool level = gpio2->level();
    TEST_ASSERT_EQUAL(true, level);
    gpio2->setLevel(false, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    level = gpio2->level();
    TEST_ASSERT_EQUAL(false, level);
}

TEST_CASE("GPIO already in use", "[GPIO]") {
    esp_err_t err = ESP_OK;
    GPIOPtr gpio2 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_2, GPIOModeInputOutput, PullUp::Disable, PullDown::Disable), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio2);
    GPIOPtr gpio2Conflict = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_2, GPIOModeInput, PullUp::Disable, PullDown::Disable), err);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, err);
    TEST_ASSERT_NULL(gpio2Conflict);
}

TEST_CASE("GPIO config equality", "[GPIO]") {
    GPIOConfig config1 = GPIOConfig(GPIO_NUM_2, GPIOModeInput, PullUp::Enable, PullDown::Disable);
    GPIOConfig config2 = GPIOConfig(GPIO_NUM_2, GPIOModeInput, PullUp::Enable, PullDown::Disable);
    GPIOConfig config3 = GPIOConfig(GPIO_NUM_2, GPIOModeOutput, PullUp::Enable, PullDown::Disable);
    TEST_ASSERT(config1 == config2);
    TEST_ASSERT(config1 != config3);
}

TEST_CASE("GPIO config factory methods", "[GPIO]") {
    GPIOConfig inputConfig = GPIOConfig(GPIO_NUM_2, GPIOModeInput, PullUp::Enable, PullDown::Disable);
    TEST_ASSERT_EQUAL(GPIO_NUM_2, inputConfig.gpioNum);
    TEST_ASSERT(inputConfig.mode == GPIOModeInput);
    TEST_ASSERT_EQUAL(PullUp::Enable, inputConfig.pullUp);
    TEST_ASSERT_EQUAL(PullDown::Disable, inputConfig.pullDown);

    GPIOConfig outputConfig = GPIOConfig(GPIO_NUM_3, GPIOModeOutputOpenDrain, PullUp::Disable, PullDown::Enable);
    TEST_ASSERT_EQUAL(GPIO_NUM_3, outputConfig.gpioNum);
    TEST_ASSERT(outputConfig.mode == GPIOModeOutputOpenDrain);
    TEST_ASSERT_EQUAL(PullUp::Disable, outputConfig.pullUp);
    TEST_ASSERT_EQUAL(PullDown::Enable, outputConfig.pullDown);
}

TEST_CASE("GPIO open-drain config", "[GPIO]") {
    esp_err_t err = ESP_OK;
    GPIOPtr gpio17 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_17, GPIOModeOutputOpenDrain, PullUp::Enable, PullDown::Disable), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio17);
    gpio17->setLevel(false, err);  // should be able to pull low
    TEST_ASSERT_EQUAL(ESP_OK, err);
    bool level = gpio17->level();
    TEST_ASSERT_EQUAL(level, false);
    gpio17->setLevel(true, err);  // should release to high impedance
    TEST_ASSERT_EQUAL(ESP_OK, err);
}

TEST_CASE("GPIO destructor", "[GPIO]") {
    esp_err_t err = ESP_OK;
    {
        GPIOPtr gpio2 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_2, GPIOModeInputOutput), err);
        TEST_ASSERT_EQUAL(ESP_OK, err);
        TEST_ASSERT_NOT_NULL(gpio2);
    }  // gpio27 goes out of scope here
    GPIOPtr gpio2Again = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_2, GPIOModeInputOutput), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio2Again);
}

TEST_CASE("Set GPIO interrupt", "[GPIO]") {
    esp_err_t err = ESP_OK;
    GPIOPtr gpio40 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_40, GPIOModeInputOutput, PullUp::Enable), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio40);

    gpio40->setLevel(true, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);

    volatile bool interruptCalled = false;
    gpio40->setInterrupt(
        GPIOInteruptType::NegativeEdge,
        [&interruptCalled]() {
            (void)0;
            interruptCalled = true;
        },
        err);
    TEST_ASSERT_EQUAL(ESP_OK, err);

    // Simulate a falling edge by toggling the level
    gpio40->setLevel(false, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);

    // Allow some time for the interrupt to be handled
    vTaskDelay(10 / portTICK_PERIOD_MS);

    TEST_ASSERT_EQUAL(true, interruptCalled);
}

TEST_CASE("Change GPIO pull-up/down", "[GPIO]") {
    esp_err_t err = ESP_OK;
    GPIOPtr gpio5 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_5, GPIOModeInput), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio5);

    gpio5->setPullUp(PullUp::Enable, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_EQUAL(PullUp::Enable, gpio5->pullUp());

    gpio5->setPullDown(PullDown::Enable, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_EQUAL(PullDown::Enable, gpio5->pullDown());

    gpio5->setPullUp(PullUp::Disable, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_EQUAL(PullUp::Disable, gpio5->pullUp());

    gpio5->setPullDown(PullDown::Disable, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_EQUAL(PullDown::Disable, gpio5->pullDown());
}

TEST_CASE("Set and get GPIO mode", "[GPIO]") {
    esp_err_t err = ESP_OK;
    GPIOPtr gpio2 = ESP32::sharedESP32()->gpio(GPIOConfig(GPIO_NUM_2, GPIOModeInput), err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT_NOT_NULL(gpio2);

    gpio2->setMode(GPIOModeInputOutput, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT(gpio2->mode() == GPIOModeInputOutput);

    gpio2->setMode(GPIOModeOutputOpenDrain, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT(gpio2->mode() == GPIOModeOutputOpenDrain);

    gpio2->setMode(GPIOModeInput, err);
    TEST_ASSERT_EQUAL(ESP_OK, err);
    TEST_ASSERT(gpio2->mode() == GPIOModeInput);
}
