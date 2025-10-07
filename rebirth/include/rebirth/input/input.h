#pragma once

#include <SDL3/SDL_events.h>
#include <EASTL/unordered_map.h>

#include <rebirth/input/keyboard.h>
#include <rebirth/input/mouse.h>

// handles key and mouse button presses
class Input
{
public:
    void processEvent(SDL_Event *event);
    bool isKeyPressed(KeyboardKey key);
    bool isMouseButtonPressed(MouseButton button);

    Input() = default;
    Input(Input const &) = delete;
    void operator=(Input const &) = delete;

private:
    SDL_Keycode getSDLKey(KeyboardKey key);
    KeyboardKey getKeyFromSDL(SDL_Keycode key);

    eastl::unordered_map<SDL_Keycode, bool> keys;
    bool mouseRight = false;
    bool mouseLeft = false;
    bool mouseMiddle = false;
};

extern Input g_input;