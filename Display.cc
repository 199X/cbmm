#include <GL/glew.h>
#include <SDL.h>
#include <iostream>

#include "Display.h"

using namespace std;

Display::Display(unsigned int width, unsigned int height, unsigned int bpp) {
  screen_width = width;
  screen_height = height;
  screen_bpp = bpp;

  SDL_Init(SDL_INIT_EVERYTHING);

  if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) ||
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) ||
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                          SDL_GL_CONTEXT_PROFILE_CORE)) {
    cout << "Error setting up GL." << endl;
  }

  window = SDL_CreateWindow("CBMM!!!!", SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED, screen_width,
                            screen_height, SDL_WINDOW_OPENGL);

  SDL_GL_CreateContext(window);  // probably should delete this

  if (!window) {
    cout << "Error opening surface: " << SDL_GetError() << endl;
  }

  glewExperimental = GL_TRUE;
  GLenum glewErr = glewInit();

  if (glewErr != GLEW_OK) {
    unsigned char const *test = glewGetErrorString(glewErr);
    cout << "Error: " << test << endl;
    return;
  }

  const GLubyte* version = glGetString(GL_SHADING_LANGUAGE_VERSION);
  if (version) {
      cout << "GLSL version: " << version << endl;
  } else {
      cout << "Could not get GLSL version." << endl;
  }

  glClearColor(0, 0, 0, 1);

  glViewport(0, 0, screen_width, screen_height);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
}

void Display::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void Display::Swap() { SDL_GL_SwapWindow(window); }

Display::~Display() { SDL_Quit(); }
