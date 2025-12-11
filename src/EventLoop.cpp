#include "EventLoop.hpp"

#include <esp_log.h>

using namespace esp;

namespace esp {
    void _handleEvent(void* userInfo, esp_event_base_t base, int32_t id, void* eventData) {
        EventHandlerInstance* eventHandler = static_cast<EventHandlerInstance*>(userInfo);
        if (eventHandler->_handler) {
            eventHandler->_handler(base, id, eventData);
        }
    }
}  // namespace esp

EventHandlerInstance::EventHandlerInstance(std::weak_ptr<EventLoop> eventLoop, EventHandler handler, esp_event_base_t base, int32_t id)
    : _eventLoop(eventLoop), _handler(handler), base(base), id(id) {}

EventHandlerInstance::EventHandlerInstance(esp_event_handler_instance_t instance, std::weak_ptr<EventLoop> eventLoop, EventHandler handler,
                                           esp_event_base_t base, int32_t id)
    : _instance(instance), _eventLoop(eventLoop), _handler(handler), base(base), id(id) {}

EventHandlerInstance::~EventHandlerInstance() {
    if (_instance == nullptr) {
        return;
    }
    EventLoopPtr loop = _eventLoop.lock();
    if (loop == nullptr) {
        return;
    }
    if (loop->_isDefaultLoop) {
        esp_err_t err = esp_event_handler_instance_unregister(base, id, _instance);
        if (err != ESP_OK) {
            ESP_LOGE(_loggingTag, "esp_event_handler_instance_unregister failed: %s", esp_err_to_name(err));
        }
        return;
    }
    esp_err_t err = esp_event_handler_instance_unregister_with(loop->_handle, base, id, _instance);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_event_handler_instance_unregister_with failed: %s", esp_err_to_name(err));
    }
    return;
}

EventLoop::EventLoop() : _isDefaultLoop(true) {
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        return;
    }
}

EventLoopPtr EventLoop::eventLoop(const EventLoopConfig& config, esp_err_t& err) {
    EventLoopPtr loop = std::shared_ptr<EventLoop>(new EventLoop(config, err));
    if (err != ESP_OK) {
        return nullptr;
    }

    return loop;
}

EventLoop::EventLoop(const EventLoopConfig& config, esp_err_t& err) {
    esp_event_loop_args_t loopArgs = {
        .queue_size = config.queueSize,
        .task_name = config.taskInfo.has_value() ? config.taskInfo->name.c_str() : nullptr,
        .task_priority = config.taskInfo.has_value() ? config.taskInfo->priority : tskIDLE_PRIORITY,
        .task_stack_size = config.taskInfo.has_value() ? config.taskInfo->stackSize : CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE,
        .task_core_id = config.taskInfo.has_value() ? config.taskInfo->coreId : tskNO_AFFINITY,
    };

    err = esp_event_loop_create(&loopArgs, &_handle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_event_loop_create failed: %s", esp_err_to_name(err));
        return;
    }
}

EventLoop::~EventLoop() {
    if (_isDefaultLoop) {
        return;
    }

    esp_err_t err = esp_event_loop_delete(_handle);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_event_loop_delete failed: %s", esp_err_to_name(err));
    }
}

EventLoopPtr EventLoop::defaultEventLoop() {
    static EventLoopPtr defaultLoop = std::shared_ptr<EventLoop>(new EventLoop());

    return defaultLoop;
}

void EventLoop::run(TickType_t ticksToRun, esp_err_t& err) {
    if (_isDefaultLoop) {
        err = ESP_ERR_INVALID_STATE;
        ESP_LOGE(_loggingTag, "run can not be used on the default event loop");
        return;
    }

    err = esp_event_loop_run(_handle, ticksToRun);
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_event_loop_run failed: %s", esp_err_to_name(err));
        return;
    }
}

EventHandlerInstancePtr EventLoop::registerHandler(esp_event_base_t eventBase, int32_t eventId, EventHandler handler, esp_err_t& err) {
    esp_event_handler_instance_t instance;
    EventLoopPtr sharedThis = shared_from_this();
    EventHandlerInstancePtr handlerInstance =
        std::shared_ptr<EventHandlerInstance>(new EventHandlerInstance(std::weak_ptr<EventLoop>(sharedThis), handler, eventBase, eventId));
    if (_isDefaultLoop) {
        err = esp_event_handler_instance_register(eventBase, eventId, _handleEvent, handlerInstance.get(), &instance);
    } else {
        err = esp_event_handler_instance_register_with(_handle, eventBase, eventId, _handleEvent, handlerInstance.get(), &instance);
    }
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_event_handler_instance_register failed: %s", esp_err_to_name(err));
        return nullptr;
    }
    handlerInstance->setInstance(instance);

    return handlerInstance;
}

void EventLoop::postEvent(esp_event_base_t eventBase, int32_t eventId, void* eventData, TickType_t ticksToWait, esp_err_t& err) {
    if (_isDefaultLoop) {
        err = esp_event_post(eventBase, eventId, eventData, sizeof(eventData), ticksToWait);
    } else {
        err = esp_event_post_to(_handle, eventBase, eventId, eventData, sizeof(eventData), ticksToWait);
    }
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_event_post failed: %s", esp_err_to_name(err));
        return;
    }
}

BaseType_t EventLoop::postEventFromISR(esp_event_base_t eventBase, int32_t eventId, void* eventData, esp_err_t& err) {
    BaseType_t taskAwoken = pdFALSE;

    if (_isDefaultLoop) {
        err = esp_event_isr_post(eventBase, eventId, eventData, sizeof(eventData), &taskAwoken);
    } else {
        err = esp_event_isr_post_to(_handle, eventBase, eventId, eventData, sizeof(eventData), &taskAwoken);
    }
    if (err != ESP_OK) {
        ESP_LOGE(_loggingTag, "esp_event_isr_post failed: %s", esp_err_to_name(err));
        return pdFALSE;
    }

    return taskAwoken;
}
