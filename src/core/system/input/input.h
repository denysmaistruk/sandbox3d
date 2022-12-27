#pragma once
#include "core/system/system_base.h"
#include "event_dispatcher.h"

template<typename System>
struct SystemDebugger;

class InputSystem : public SystemBase<InputSystem>
{
    friend SystemBase<InputSystem>;
    friend struct SystemDebugger<InputSystem>;
public:
    void update(float dt);

    void addInputEventHandler(const InputEvent& event, const std::function<void(const InputEvent&)>& handler);

protected:
    InputSystem();
private:
    InputEventDispatcher m_dispatcher;
};