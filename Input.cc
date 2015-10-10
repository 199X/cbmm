#include "Input.h"

std::vector<ButtonEvent> GetButtonEvents() {
  std::vector<ButtonEvent> button_events;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        button_events.emplace_back(Button::QUIT, ButtonState::RELEASED);
        break;
      case SDL_KEYUP:
      case SDL_KEYDOWN:
      {
        ButtonState state = (event.type == SDL_KEYUP ? ButtonState::RELEASED
                                                     : ButtonState::PRESSED);
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            button_events.emplace_back(Button::QUIT, state);
            break;
          case SDLK_p:
            button_events.emplace_back(Button::PAUSE, state);
            break;
          case SDLK_z:
            button_events.emplace_back(Button::JUMP, state);
            break;
          case SDLK_UP:
            button_events.emplace_back(Button::UP, state);
            break;
          case SDLK_DOWN:
            button_events.emplace_back(Button::DOWN, state);
            break;
          case SDLK_LEFT:
            button_events.emplace_back(Button::LEFT, state);
            break;
          case SDLK_RIGHT:
            button_events.emplace_back(Button::RIGHT, state);
            break;
          default:
            break;
        }
        break;
      }
      default:
        break;
    }
  }
  return button_events;
}
