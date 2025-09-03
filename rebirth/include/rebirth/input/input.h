#pragma once

#include <SDL3/SDL_events.h>
#include <unordered_map>

#include <rebirth/input/keyboard.h>
#include <rebirth/input/mouse.h>

namespace rebirth
{

// handles key and mouse button presses
class Input
{
public:
    static Input &getInstance();

    void processEvent(SDL_Event *event);
    bool isKeyPressed(KeyboardKey key);
    bool isMouseButtonPressed(MouseButton button);

    Input(Input const &) = delete;
    void operator=(Input const &) = delete;

private:
    Input() = default;

    SDL_Keycode getSDLKey(KeyboardKey key);
    KeyboardKey getKeyFromSDL(SDL_Keycode key);

    std::unordered_map<SDL_Keycode, bool> keys;
    bool mouseRight = false;
    bool mouseLeft = false;
    bool mouseMiddle = false;
};

} // namespace rebirth