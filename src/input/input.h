#pragma once

#include "input/keyboard.h"
#include "input/mouse.h"

#include "EASTL/unordered_map.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_mouse.h"
#include "math/math.h"

enum class InputAction
{
    Pressed,
    Released,
    JustPressed,
    JustReleased,
};

// handles key and mouse button presses
class Input
{
public:
    Input() = default;
    ~Input() = default;

    void processEvent(const SDL_Event &event);

    bool getKey(KeyboardKey key, InputAction action);
    bool getMouseButton(MouseButton button, InputAction action);
    vec2 getMousePosition() const { return mousePosition; };
    vec2 getMouseRelativeMotion() const { return mouseRelativeMotion; };

private:
    SDL_Keycode getSDLKey(KeyboardKey key);
    KeyboardKey getKeyFromSDL(SDL_Keycode key);
    SDL_MouseButtonFlags getSDLMouseButton(MouseButton button);

    eastl::unordered_map<SDL_Keycode, bool> previousKeys;
    eastl::unordered_map<SDL_Keycode, bool> currentKeys;

    eastl::unordered_map<SDL_MouseButtonFlags, bool> previousMouseButtons;
    eastl::unordered_map<SDL_MouseButtonFlags, bool> currentMouseButtons;

    vec2 mousePosition = vec2(0.f, 0.f);
    vec2 mouseRelativeMotion = vec2(0.f, 0.f);
};