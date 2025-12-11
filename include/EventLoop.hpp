#include "Testing.hpp"

#include <esp_event.h>
#include <freertos/FreeRTOS.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace esp {
    class EventLoop;
    using EventLoopPtr = std::shared_ptr<EventLoop>;

    struct TaskInfo {
        std::string name;
        UBaseType_t priority = tskIDLE_PRIORITY;
        uint32_t stackSize = CONFIG_ESP_SYSTEM_EVENT_TASK_STACK_SIZE;
        BaseType_t coreId = tskNO_AFFINITY;
    };

    struct EventLoopConfig {
        int32_t queueSize;
        std::optional<TaskInfo> taskInfo;
    };

    class EventHandlerInstance;
    using EventHandlerInstancePtr = std::shared_ptr<EventHandlerInstance>;

    void _handleEvent(void* userInfo, esp_event_base_t base, int32_t id, void* eventData);

    using EventHandler = std::function<void(esp_event_base_t eventBase, int32_t eventId, void* eventData)>;

    class EventHandlerInstance {
    public:
        ~EventHandlerInstance();

        PRIVATE_UNLESS_TESTING
        EventHandlerInstance(esp_event_handler_instance_t instance, std::weak_ptr<EventLoop> eventLoop, EventHandler handler, esp_event_base_t base,
                             int32_t id);

    private:
        EventHandlerInstance(std::weak_ptr<EventLoop> eventLoop, EventHandler handler, esp_event_base_t base, int32_t id);

        void setInstance(esp_event_handler_instance_t instance) { _instance = instance; }

    private:
        esp_event_handler_instance_t _instance;
        std::weak_ptr<EventLoop> _eventLoop;
        EventHandler _handler;
        esp_event_base_t base;
        int32_t id;

        static constexpr char _loggingTag[] = "esp::EventHandlerInstance";

        friend class EventLoop;
        friend void _handleEvent(void* userInfo, esp_event_base_t base, int32_t id, void* eventData);
    };

    class EventLoop : public std::enable_shared_from_this<EventLoop> {
    public:
        ~EventLoop();

        static EventLoopPtr eventLoop(const EventLoopConfig& config, esp_err_t& err);

        static EventLoopPtr defaultEventLoop();

        void run(TickType_t ticksToRun, esp_err_t& err);

        EventHandlerInstancePtr registerHandler(esp_event_base_t eventBase, int32_t eventId, EventHandler handler, esp_err_t& err);

        void postEvent(esp_event_base_t eventBase, int32_t eventId, void* eventData, TickType_t ticksToWait, esp_err_t& err);
        BaseType_t postEventFromISR(esp_event_base_t eventBase, int32_t eventId, void* eventData, esp_err_t& err);

    private:
        EventLoop(const EventLoopConfig& config, esp_err_t& err);
        EventLoop();

        struct EventIdentifier {
            esp_event_base_t eventBase;
            int32_t eventId;

            bool operator==(const EventIdentifier& other) const { return eventBase == other.eventBase && eventId == other.eventId; }
        };

        struct EventIdentifierHash {
            std::size_t operator()(const EventIdentifier& identifier) const {
                return std::hash<esp_event_base_t>()(identifier.eventBase) ^ std::hash<int32_t>()(identifier.eventId);
            }
        };

        bool _isDefaultLoop = false;
        esp_event_loop_handle_t _handle;

        static constexpr char _loggingTag[] = "esp::EventLoop";

        friend class EventHandlerInstance;
        friend void _handleEvent(void* userInfo, esp_event_base_t base, int32_t id, void* eventData);
    };
}  // namespace esp
