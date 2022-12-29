#pragma once

enum class InputEventType
{
    Invalid,
    KeyPressed,
    KeyDown,
    KeyReleased,
    KeyUp,
    MouseButtonPressed,
    MouseButtonDown,
    MouseButtonReleased,
    MouseButtonUp
};

struct InputEvent
{
    InputEventType type;
    int signal;
};

class InputEventDispatcher
{
public:
    void checkAndDispatch();
    
    void registerEventHandler(const InputEvent& event, const std::function<void(const InputEvent&)>& handler);
    void dispatchEvent(const InputEvent& event);
    
private:
    std::map<InputEvent, std::function<void(const InputEvent&)>> m_handlers;
};