extern "C" {
#include <unity.h>
}
#include <freertos/FreeRTOS.h>

#include "ESP32.hpp"
#include "EventLoop.hpp"

using namespace esp;

TEST_CASE("Create and destroy", "[EventLoop]") {
    esp_err_t err = ESP_OK;
    EventLoopPtr loop = EventLoop::eventLoop(EventLoopConfig{.queueSize = 10,
                                                             .taskInfo =
                                                                 TaskInfo{
                                                                     .name = "TestEventLoopTask",
                                                                 }},
                                             err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(loop);
}

TEST_CASE("Default event loop", "[EventLoop]") {
    EventLoopPtr loop1 = EventLoop::defaultEventLoop();
    TEST_ASSERT_NOT_NULL(loop1);
    EventLoopPtr loop2 = EventLoop::defaultEventLoop();
    TEST_ASSERT_NOT_NULL(loop2);
    TEST_ASSERT_EQUAL(loop1.get(), loop2.get());
}

TEST_CASE("Run event loop", "[EventLoop]") {
    esp_err_t err = ESP_OK;
    EventLoopPtr loop = EventLoop::eventLoop(EventLoopConfig{.queueSize = 10,
                                                             .taskInfo =
                                                                 TaskInfo{
                                                                     .name = "TestEventLoopTask",
                                                                 }},
                                             err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(loop);

    loop->run(100 / portTICK_PERIOD_MS, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
}

TEST_CASE("Register and unregister event handler", "[EventLoop]") {
    esp_err_t err = ESP_OK;
    EventLoopPtr loop = EventLoop::eventLoop(EventLoopConfig{.queueSize = 10,
                                                             .taskInfo =
                                                                 TaskInfo{
                                                                     .name = "TestEventLoopTask",
                                                                 }},
                                             err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(loop);
    {
        EventHandlerInstancePtr handlerInstance = loop->registerHandler(
            "TEST_EVENT_BASE", 1,
            [](esp_event_base_t eventBase, int32_t eventId, void* eventData) {
                (void)eventBase;
                (void)eventId;
                (void)eventData;
            },
            err);
        TEST_ASSERT_EQUAL(err, ESP_OK);
        TEST_ASSERT_NOT_NULL(handlerInstance);
    }
}

TEST_CASE("Register handler on default event loop", "[EventLoop]") {
    esp_err_t err = ESP_OK;
    EventLoopPtr loop = EventLoop::defaultEventLoop();
    TEST_ASSERT_NOT_NULL(loop);

    {
        EventHandlerInstancePtr handlerInstance = loop->registerHandler(
            "TEST_EVENT_BASE_DEFAULT", 1,
            [](esp_event_base_t eventBase, int32_t eventId, void* eventData) {
                (void)eventBase;
                (void)eventId;
                (void)eventData;
            },
            err);
        TEST_ASSERT_EQUAL(err, ESP_OK);
        TEST_ASSERT_NOT_NULL(handlerInstance);
    }
}

TEST_CASE("Post event to event loop", "[EventLoop]") {
    esp_err_t err = ESP_OK;
    EventLoopPtr loop = EventLoop::eventLoop(EventLoopConfig{.queueSize = 10,
                                                             .taskInfo =
                                                                 TaskInfo{
                                                                     .name = "TestEventLoopTask",
                                                                 }},
                                             err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(loop);

    volatile bool eventHandled = false;
    {
        EventHandlerInstancePtr handlerInstance = loop->registerHandler(
            "TEST_EVENT_BASE_POST", 1,
            [&eventHandled](esp_event_base_t eventBase, int32_t eventId, void* eventData) {
                (void)eventBase;
                (void)eventId;
                (void)eventData;
                eventHandled = true;
            },
            err);
        TEST_ASSERT_EQUAL(err, ESP_OK);
        TEST_ASSERT_NOT_NULL(handlerInstance);

        loop->postEvent("TEST_EVENT_BASE_POST", 1, nullptr, portMAX_DELAY, err);
        TEST_ASSERT_EQUAL(err, ESP_OK);

        // Allow some time for the event to be handled
        vTaskDelay(10 / portTICK_PERIOD_MS);

        TEST_ASSERT_EQUAL(true, eventHandled);
    }

    eventHandled = false;
    loop->postEvent("TEST_EVENT_BASE_POST", 1, nullptr, portMAX_DELAY, err);
    TEST_ASSERT_EQUAL(err, ESP_OK);

    // Allow some time for the event to be handled
    vTaskDelay(10 / portTICK_PERIOD_MS);

    TEST_ASSERT_EQUAL(false, eventHandled);
}

TEST_CASE("Post event from ISR to event loop", "[EventLoop]") {
    esp_err_t err = ESP_OK;
    EventLoopPtr loop = EventLoop::eventLoop(EventLoopConfig{.queueSize = 10,
                                                             .taskInfo =
                                                                 TaskInfo{
                                                                     .name = "TestEventLoopTask",
                                                                 }},
                                             err);
    TEST_ASSERT_EQUAL(err, ESP_OK);
    TEST_ASSERT_NOT_NULL(loop);

    volatile bool eventHandled = false;
    {
        EventHandlerInstancePtr handlerInstance = loop->registerHandler(
            "TEST_EVENT_BASE_ISR", 1,
            [&eventHandled](esp_event_base_t eventBase, int32_t eventId, void* eventData) {
                (void)eventBase;
                (void)eventId;
                (void)eventData;
                eventHandled = true;
            },
            err);
        TEST_ASSERT_EQUAL(err, ESP_OK);
        TEST_ASSERT_NOT_NULL(handlerInstance);

        loop->postEventFromISR("TEST_EVENT_BASE_ISR", 1, nullptr, err);
        TEST_ASSERT_EQUAL(err, ESP_OK);

        // Allow some time for the event to be handled
        vTaskDelay(10 / portTICK_PERIOD_MS);

        TEST_ASSERT_EQUAL(true, eventHandled);
    }
}
