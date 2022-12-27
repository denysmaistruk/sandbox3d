#pragma once
#include "core/system/system_base.h"
#include "event_dispatcher.h"

template<typename System>
struct SystemDebugger;

class InputSystem : SystemBase<InputSystem>
{
    friend class SystemDebugger<InputSystem>;
public:
    InputSystem();

    void update(float dt);

    void addInputEventHandler(const InputEvent& event, const std::function<void(const InputEvent&)>& handler);

private:
    InputEventDispatcher m_dispatcher;
};