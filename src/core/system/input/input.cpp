#include "input.h"

void InputSystem::update(float dt) 
{
    m_dispatcher.checkAndDispatch();
}

void InputSystem::addInputEventHandler(const InputEvent& event, const std::function<void(const InputEvent&)>& handler) 
{
    m_dispatcher.registerEventHandler(event, handler);
}