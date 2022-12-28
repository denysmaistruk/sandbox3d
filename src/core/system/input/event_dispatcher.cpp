#include "event_dispatcher.h"

#include "raylib.h"

static constexpr bool operator<(const InputEvent& left, const InputEvent& right)
{
    return static_cast<int>(left.type) * left.signal < static_cast<int>(right.type) * right.signal;
}

void InputEventDispatcher::registerEventHandler(const InputEvent& event, const std::function<void(const InputEvent&)>& handler) 
{
    assert(m_handlers.find(event) == m_handlers.end());
    m_handlers[event] = handler;
}

void InputEventDispatcher::dispatchEvent(const InputEvent& event) 
{
    auto it = m_handlers.find(event);
    assert(it != m_handlers.end());
    it->second(event);
}

void InputEventDispatcher::checkAndDispatch() 
{
    for (auto& [event, handler] : m_handlers) 
    {
        if (event.type == InputEventType::KeyPressed && IsKeyPressed(event.signal)) 
        {
            dispatchEvent(event);
        }
        if (event.type == InputEventType::KeyDown && IsKeyDown(event.signal)) 
        {
            dispatchEvent(event);
        }
        if (event.type == InputEventType::KeyReleased && IsKeyReleased(event.signal)) 
        {
            dispatchEvent(event);
        }
        if (event.type == InputEventType::KeyUp && IsKeyUp(event.signal)) 
        {
            dispatchEvent(event);
        }
        if (event.type == InputEventType::MouseButtonPressed && IsMouseButtonPressed(event.signal)) 
        {
            dispatchEvent(event);
        }
        if (event.type == InputEventType::MouseButtonDown && IsMouseButtonDown(event.signal)) 
        {
            dispatchEvent(event);
        }
        if (event.type == InputEventType::MouseButtonReleased && IsMouseButtonReleased(event.signal)) 
        {
            dispatchEvent(event);
        }
        if (event.type == InputEventType::MouseButtonUp && IsMouseButtonUp(event.signal)) 
        {
            dispatchEvent(event);
        }
    }
}