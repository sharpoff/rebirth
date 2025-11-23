#include "input/input.h"
#include "SDL3/SDL_events.h"

// FIXME: JustReleased is buggy, probably something wrong with processEvent

void Input::processEvent(const SDL_Event &event)
{
    bool        keyPressed = event.type != SDL_EVENT_KEY_UP;
    SDL_Keycode key = event.key.key;
    currentKeys[key] = false;
    previousKeys[key] = currentKeys[key];
    currentKeys[key] = keyPressed;

    bool                 mousePressed = event.button.type != SDL_EVENT_MOUSE_BUTTON_UP;
    SDL_MouseButtonFlags mouseButton = event.button.button;
    previousMouseButtons[mouseButton] = currentMouseButtons[mouseButton];
    currentMouseButtons[mouseButton] = mousePressed;

    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mousePosition = vec2(event.motion.x, event.motion.y);
        mouseRelativeMotion = vec2(event.motion.xrel, event.motion.yrel);
    }

}

bool Input::getKey(KeyboardKey key, InputAction action)
{
    SDL_Keycode sdlKey = getSDLKey(key);
    switch (action) {
        case InputAction::Pressed:
            return currentKeys[sdlKey];
        case InputAction::Released:
            return !currentKeys[sdlKey];
        case InputAction::JustPressed:
            return currentKeys[sdlKey] && !previousKeys[sdlKey];
        case InputAction::JustReleased:
            return !currentKeys[sdlKey] && previousKeys[sdlKey];
    }

    return false;
}

bool Input::getMouseButton(MouseButton button, InputAction action)
{
    SDL_MouseButtonFlags sdlButton = getSDLMouseButton(button);
    switch (action) {
        case InputAction::Pressed:
            return currentMouseButtons[sdlButton];
        case InputAction::Released:
            return !currentMouseButtons[sdlButton];
        case InputAction::JustPressed:
            return currentMouseButtons[sdlButton] && !previousMouseButtons[sdlButton];
        case InputAction::JustReleased:
            return !currentMouseButtons[sdlButton] && previousMouseButtons[sdlButton];
    }

    return false;
}

SDL_Keycode Input::getSDLKey(KeyboardKey key)
{
    switch (key) {
        case KeyboardKey::A:
            return SDLK_A;
        case KeyboardKey::B:
            return SDLK_B;
        case KeyboardKey::C:
            return SDLK_C;
        case KeyboardKey::D:
            return SDLK_D;
        case KeyboardKey::E:
            return SDLK_E;
        case KeyboardKey::F:
            return SDLK_F;
        case KeyboardKey::G:
            return SDLK_G;
        case KeyboardKey::H:
            return SDLK_H;
        case KeyboardKey::I:
            return SDLK_I;
        case KeyboardKey::J:
            return SDLK_J;
        case KeyboardKey::K:
            return SDLK_K;
        case KeyboardKey::L:
            return SDLK_L;
        case KeyboardKey::M:
            return SDLK_M;
        case KeyboardKey::N:
            return SDLK_N;
        case KeyboardKey::O:
            return SDLK_O;
        case KeyboardKey::P:
            return SDLK_P;
        case KeyboardKey::Q:
            return SDLK_Q;
        case KeyboardKey::R:
            return SDLK_R;
        case KeyboardKey::S:
            return SDLK_S;
        case KeyboardKey::T:
            return SDLK_T;
        case KeyboardKey::U:
            return SDLK_U;
        case KeyboardKey::V:
            return SDLK_V;
        case KeyboardKey::W:
            return SDLK_W;
        case KeyboardKey::X:
            return SDLK_X;
        case KeyboardKey::Y:
            return SDLK_Y;
        case KeyboardKey::Z:
            return SDLK_Z;

        case KeyboardKey::NUM0:
            return SDLK_0;
        case KeyboardKey::NUM1:
            return SDLK_1;
        case KeyboardKey::NUM2:
            return SDLK_2;
        case KeyboardKey::NUM3:
            return SDLK_3;
        case KeyboardKey::NUM4:
            return SDLK_4;
        case KeyboardKey::NUM5:
            return SDLK_5;
        case KeyboardKey::NUM6:
            return SDLK_6;
        case KeyboardKey::NUM7:
            return SDLK_7;
        case KeyboardKey::NUM8:
            return SDLK_8;
        case KeyboardKey::NUM9:
            return SDLK_9;

        case KeyboardKey::F1:
            return SDLK_F1;
        case KeyboardKey::F2:
            return SDLK_F2;
        case KeyboardKey::F3:
            return SDLK_F3;
        case KeyboardKey::F4:
            return SDLK_F4;
        case KeyboardKey::F5:
            return SDLK_F5;
        case KeyboardKey::F6:
            return SDLK_F6;
        case KeyboardKey::F7:
            return SDLK_F7;
        case KeyboardKey::F8:
            return SDLK_F8;
        case KeyboardKey::F9:
            return SDLK_F9;
        case KeyboardKey::F10:
            return SDLK_F10;
        case KeyboardKey::F11:
            return SDLK_F11;
        case KeyboardKey::F12:
            return SDLK_F12;

        case KeyboardKey::LCTRL:
            return SDLK_LCTRL;
        case KeyboardKey::RCTRL:
            return SDLK_RCTRL;
        case KeyboardKey::LSHIFT:
            return SDLK_LSHIFT;
        case KeyboardKey::RSHIFT:
            return SDLK_RSHIFT;
        case KeyboardKey::LALT:
            return SDLK_LALT;
        case KeyboardKey::RALT:
            return SDLK_RALT;
        case KeyboardKey::CAPSLOCK:
            return SDLK_CAPSLOCK;

        case KeyboardKey::RETURN:
            return SDLK_RETURN;
        case KeyboardKey::ESCAPE:
            return SDLK_ESCAPE;
        case KeyboardKey::BACKSPACE:
            return SDLK_BACKSPACE;
        case KeyboardKey::TAB:
            return SDLK_TAB;
        case KeyboardKey::SPACE:
            return SDLK_SPACE;

        case KeyboardKey::KP0:
            return SDLK_KP_0;
        case KeyboardKey::KP1:
            return SDLK_KP_1;
        case KeyboardKey::KP2:
            return SDLK_KP_2;
        case KeyboardKey::KP3:
            return SDLK_KP_3;
        case KeyboardKey::KP4:
            return SDLK_KP_4;
        case KeyboardKey::KP5:
            return SDLK_KP_5;
        case KeyboardKey::KP6:
            return SDLK_KP_6;
        case KeyboardKey::KP7:
            return SDLK_KP_7;
        case KeyboardKey::KP8:
            return SDLK_KP_8;
        case KeyboardKey::KP9:
            return SDLK_KP_9;
        case KeyboardKey::KPPLUS:
            return SDLK_KP_PLUS;
        case KeyboardKey::KPMINUS:
            return SDLK_KP_MINUS;
        case KeyboardKey::KPMULTIPLY:
            return SDLK_KP_MULTIPLY;
        case KeyboardKey::KPDIVIDE:
            return SDLK_KP_DIVIDE;
        case KeyboardKey::KPENTER:
            return SDLK_KP_ENTER;
        case KeyboardKey::KPPERIOD:
            return SDLK_KP_PERIOD;

        case KeyboardKey::UP:
            return SDLK_UP;
        case KeyboardKey::DOWN:
            return SDLK_DOWN;
        case KeyboardKey::LEFT:
            return SDLK_LEFT;
        case KeyboardKey::RIGHT:
            return SDLK_RIGHT;
        case KeyboardKey::HOME:
            return SDLK_HOME;
        case KeyboardKey::END:
            return SDLK_END;
        case KeyboardKey::PAGEUP:
            return SDLK_PAGEUP;
        case KeyboardKey::PAGEDOWN:
            return SDLK_PAGEDOWN;
        case KeyboardKey::INSERT:
            return SDLK_INSERT;
        case KeyboardKey::DELETE:
            return SDLK_DELETE;

        case KeyboardKey::COMMA:
            return SDLK_COMMA;
        case KeyboardKey::PERIOD:
            return SDLK_PERIOD;
        case KeyboardKey::SEMICOLON:
            return SDLK_SEMICOLON;
        case KeyboardKey::QUOTE:
            return SDLK_APOSTROPHE;
        case KeyboardKey::BACKQUOTE:
            return SDLK_GRAVE;
        case KeyboardKey::LEFTBRACKET:
            return SDLK_LEFTBRACKET;
        case KeyboardKey::RIGHTBRACKET:
            return SDLK_RIGHTBRACKET;
        case KeyboardKey::BACKSLASH:
            return SDLK_BACKSLASH;
        case KeyboardKey::SLASH:
            return SDLK_SLASH;
        case KeyboardKey::MINUS:
            return SDLK_MINUS;
        case KeyboardKey::EQUALS:
            return SDLK_EQUALS;

        case KeyboardKey::PRINTSCREEN:
            return SDLK_PRINTSCREEN;
        case KeyboardKey::SCROLLLOCK:
            return SDLK_SCROLLLOCK;
        case KeyboardKey::PAUSE:
            return SDLK_PAUSE;
        case KeyboardKey::MENU:
            return SDLK_MENU;

        case KeyboardKey::VOLUMEUP:
            return SDLK_VOLUMEUP;
        case KeyboardKey::VOLUMEDOWN:
            return SDLK_VOLUMEDOWN;

        default:
            break;
    }

    return SDLK_UNKNOWN;
}

SDL_MouseButtonFlags Input::getSDLMouseButton(MouseButton button)
{
    switch (button) {
        case MouseButton::RIGHT:
            return SDL_BUTTON_RIGHT;
        case MouseButton::LEFT:
            return SDL_BUTTON_LEFT;
        case MouseButton::MIDDLE:
            return SDL_BUTTON_MIDDLE;
    }
}