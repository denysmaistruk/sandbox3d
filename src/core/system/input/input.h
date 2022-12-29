#pragma once
#include "core/system/system_base.h"
#include "event_dispatcher.h"

template<typename System>
struct SystemDebugger;

class InputSystem : public SystemBase<InputSystem>
{
    SANDBOX3D_SYSTEM_CLASS(InputSystem);
public:
    void update(float dt);

    void addInputEventHandler(const InputEvent& event, const std::function<void(const InputEvent&)>& handler);

protected:
    InputSystem(size_t id);

private:
    InputEventDispatcher m_dispatcher;
};