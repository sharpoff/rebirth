#include <rebirth/input/input.h>

namespace rebirth
{

Input &Input::getInstance()
{
    static Input instance;

    return instance;
}

void Input::processEvent(SDL_Event *event)
{
    if (!event)
        return;

    bool pressed = event->type != SDL_EVENT_KEY_UP;
    SDL_Keycode key = event->key.key;

    keys[key] = pressed;

    bool mousePressed = event->button.type != SDL_EVENT_MOUSE_BUTTON_UP;
    if (event->button.button == SDL_BUTTON_LEFT) {
        mouseLeft = mousePressed;
    }
    if (event->button.button == SDL_BUTTON_RIGHT) {
        mouseRight = mousePressed;
    }
    if (event->button.button == SDL_BUTTON_MIDDLE) {
        mouseMiddle = mousePressed;
    }
}

bool Input::isKeyPressed(KeyboardKey key)
{
    SDL_Keycode sdlKey = getSDLKey(key);
    if (keys.find(sdlKey) != keys.end())
        return keys[sdlKey];

    return false;
}

bool Input::isMouseButtonPressed(MouseButton button)
{
    switch (button) {
        case MouseButton::RIGHT:
            return mouseRight;
        case MouseButton::LEFT:
            return mouseLeft;
        case MouseButton::MIDDLE:
            return mouseMiddle;
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

KeyboardKey Input::getKeyFromSDL(SDL_Keycode key)
{
    switch (key) {
        case SDLK_A:
            return KeyboardKey::A;
        case SDLK_B:
            return KeyboardKey::B;
        case SDLK_C:
            return KeyboardKey::C;
        case SDLK_D:
            return KeyboardKey::D;
        case SDLK_E:
            return KeyboardKey::E;
        case SDLK_F:
            return KeyboardKey::F;
        case SDLK_G:
            return KeyboardKey::G;
        case SDLK_H:
            return KeyboardKey::H;
        case SDLK_I:
            return KeyboardKey::I;
        case SDLK_J:
            return KeyboardKey::J;
        case SDLK_K:
            return KeyboardKey::K;
        case SDLK_L:
            return KeyboardKey::L;
        case SDLK_M:
            return KeyboardKey::M;
        case SDLK_N:
            return KeyboardKey::N;
        case SDLK_O:
            return KeyboardKey::O;
        case SDLK_P:
            return KeyboardKey::P;
        case SDLK_Q:
            return KeyboardKey::Q;
        case SDLK_R:
            return KeyboardKey::R;
        case SDLK_S:
            return KeyboardKey::S;
        case SDLK_T:
            return KeyboardKey::T;
        case SDLK_U:
            return KeyboardKey::U;
        case SDLK_V:
            return KeyboardKey::V;
        case SDLK_W:
            return KeyboardKey::W;
        case SDLK_X:
            return KeyboardKey::X;
        case SDLK_Y:
            return KeyboardKey::Y;
        case SDLK_Z:
            return KeyboardKey::Z;

        case SDLK_0:
            return KeyboardKey::NUM0;
        case SDLK_1:
            return KeyboardKey::NUM1;
        case SDLK_2:
            return KeyboardKey::NUM2;
        case SDLK_3:
            return KeyboardKey::NUM3;
        case SDLK_4:
            return KeyboardKey::NUM4;
        case SDLK_5:
            return KeyboardKey::NUM5;
        case SDLK_6:
            return KeyboardKey::NUM6;
        case SDLK_7:
            return KeyboardKey::NUM7;
        case SDLK_8:
            return KeyboardKey::NUM8;
        case SDLK_9:
            return KeyboardKey::NUM9;

        case SDLK_F1:
            return KeyboardKey::F1;
        case SDLK_F2:
            return KeyboardKey::F2;
        case SDLK_F3:
            return KeyboardKey::F3;
        case SDLK_F4:
            return KeyboardKey::F4;
        case SDLK_F5:
            return KeyboardKey::F5;
        case SDLK_F6:
            return KeyboardKey::F6;
        case SDLK_F7:
            return KeyboardKey::F7;
        case SDLK_F8:
            return KeyboardKey::F8;
        case SDLK_F9:
            return KeyboardKey::F9;
        case SDLK_F10:
            return KeyboardKey::F10;
        case SDLK_F11:
            return KeyboardKey::F11;
        case SDLK_F12:
            return KeyboardKey::F12;

        case SDLK_LCTRL:
            return KeyboardKey::LCTRL;
        case SDLK_RCTRL:
            return KeyboardKey::RCTRL;
        case SDLK_LSHIFT:
            return KeyboardKey::LSHIFT;
        case SDLK_RSHIFT:
            return KeyboardKey::RSHIFT;
        case SDLK_LALT:
            return KeyboardKey::LALT;
        case SDLK_RALT:
            return KeyboardKey::RALT;
        case SDLK_CAPSLOCK:
            return KeyboardKey::CAPSLOCK;

        case SDLK_RETURN:
            return KeyboardKey::RETURN;
        case SDLK_ESCAPE:
            return KeyboardKey::ESCAPE;
        case SDLK_BACKSPACE:
            return KeyboardKey::BACKSPACE;
        case SDLK_TAB:
            return KeyboardKey::TAB;
        case SDLK_SPACE:
            return KeyboardKey::SPACE;

        case SDLK_KP_0:
            return KeyboardKey::KP0;
        case SDLK_KP_1:
            return KeyboardKey::KP1;
        case SDLK_KP_2:
            return KeyboardKey::KP2;
        case SDLK_KP_3:
            return KeyboardKey::KP3;
        case SDLK_KP_4:
            return KeyboardKey::KP4;
        case SDLK_KP_5:
            return KeyboardKey::KP5;
        case SDLK_KP_6:
            return KeyboardKey::KP6;
        case SDLK_KP_7:
            return KeyboardKey::KP7;
        case SDLK_KP_8:
            return KeyboardKey::KP8;
        case SDLK_KP_9:
            return KeyboardKey::KP9;
        case SDLK_KP_PLUS:
            return KeyboardKey::KPPLUS;
        case SDLK_KP_MINUS:
            return KeyboardKey::KPMINUS;
        case SDLK_KP_MULTIPLY:
            return KeyboardKey::KPMULTIPLY;
        case SDLK_KP_DIVIDE:
            return KeyboardKey::KPDIVIDE;
        case SDLK_KP_ENTER:
            return KeyboardKey::KPENTER;
        case SDLK_KP_PERIOD:
            return KeyboardKey::KPPERIOD;

        case SDLK_UP:
            return KeyboardKey::UP;
        case SDLK_DOWN:
            return KeyboardKey::DOWN;
        case SDLK_LEFT:
            return KeyboardKey::LEFT;
        case SDLK_RIGHT:
            return KeyboardKey::RIGHT;
        case SDLK_HOME:
            return KeyboardKey::HOME;
        case SDLK_END:
            return KeyboardKey::END;
        case SDLK_PAGEUP:
            return KeyboardKey::PAGEUP;
        case SDLK_PAGEDOWN:
            return KeyboardKey::PAGEDOWN;
        case SDLK_INSERT:
            return KeyboardKey::INSERT;
        case SDLK_DELETE:
            return KeyboardKey::DELETE;

        case SDLK_COMMA:
            return KeyboardKey::COMMA;
        case SDLK_PERIOD:
            return KeyboardKey::PERIOD;
        case SDLK_SEMICOLON:
            return KeyboardKey::SEMICOLON;
        case SDLK_APOSTROPHE:
            return KeyboardKey::QUOTE;
        case SDLK_GRAVE:
            return KeyboardKey::BACKQUOTE;
        case SDLK_LEFTBRACKET:
            return KeyboardKey::LEFTBRACKET;
        case SDLK_RIGHTBRACKET:
            return KeyboardKey::RIGHTBRACKET;
        case SDLK_BACKSLASH:
            return KeyboardKey::BACKSLASH;
        case SDLK_SLASH:
            return KeyboardKey::SLASH;
        case SDLK_MINUS:
            return KeyboardKey::MINUS;
        case SDLK_EQUALS:
            return KeyboardKey::EQUALS;

        case SDLK_PRINTSCREEN:
            return KeyboardKey::PRINTSCREEN;
        case SDLK_SCROLLLOCK:
            return KeyboardKey::SCROLLLOCK;
        case SDLK_PAUSE:
            return KeyboardKey::PAUSE;
        case SDLK_MENU:
            return KeyboardKey::MENU;

        case SDLK_VOLUMEUP:
            return KeyboardKey::VOLUMEUP;
        case SDLK_VOLUMEDOWN:
            return KeyboardKey::VOLUMEDOWN;

        default:
            break;
    }

    return KeyboardKey::UNDEFINED;
}

} // namespace rebirth