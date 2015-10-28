#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

#include <SDL.h>

#include "Bog.h"
#include "Camera.h"
#include "Display.h"
#include "EntityManager.h"
#include "Event.h"
#include "GeometryManager.h"
#include "Input.h"
#include "Physics.h"
#include "ShaderManager.h"
#include "State.h"
#include "TextureManager.h"

using namespace std;

int main(int, char**) {
  const unsigned int SCREEN_WIDTH = 1024;
  const unsigned int SCREEN_HEIGHT = 768;
  const unsigned int SCREEN_BPP = 32;

  Display display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP);

  std::unique_ptr<MapProgram> tileProgram = MapProgram::Make();
  std::unique_ptr<TextureProgram> textureProgram = TextureProgram::Make();
  std::unique_ptr<ColorProgram> colorProgram = ColorProgram::Make();

  GeometryManager geometryManager = GeometryManager();
  TextureManager textureManager = TextureManager();

  TextureRef tileSetRef =
      textureManager.LoadTexture("resources/tileset.png", 0);
  TextureRef collisionSetRef =
      textureManager.LoadTexture("resources/collision.png", 0);
  TextureRef dogRef =
      textureManager.LoadTexture("resources/dog_tilesheet.png", 0);
  TextureRef bgRef =
      textureManager.LoadTexture("resources/bg.png", 0);

  auto jump_state_system = MakeJumpStateSystem();
  auto lr_state_system = MakeLRStateSystem();
  Camera camera;
  BoundingBoxGraphicsSystem bb_graphics(&geometryManager, colorProgram.get());
  SubSpriteGraphicsSystem ss_graphics(&geometryManager, textureProgram.get(),
                                      &textureManager);
  EntityManager em;
  vector<Entity> bogs;
  for (int x = 0; x < 1; x += 1) {
    EntityId id = em.CreateEntity();
    bogs.emplace_back(id);
    bogs.back().AddComponent(std::unique_ptr<Transform>(new Transform()));
    bogs.back().AddComponent(std::unique_ptr<Body>(
        new Body(true, {{2, 2}, 0.9, 0.75}, {1, (double)0 / 2.0})));
    bogs.back().AddComponent(std::unique_ptr<JumpStateComponent>(
        new JumpStateComponent(JumpState::STANDING)));
    bogs.back().AddComponent(std::unique_ptr<LRStateComponent>(
        new LRStateComponent(LRState::STILL)));
    bogs.back().AddComponent(std::unique_ptr<Sprite>(new Sprite(dogRef, 0)));
  }

  Map level;
  if (level.LoadTmx("resources/test.tmx")) {
    cout << "Error loading test.tmx" << endl;
  }
  const TileMap* collision_map = level.GetLayer("Collision");
  assert(collision_map);
  Physics physics(collision_map);

  const TileMap* tilemap = level.GetLayer("Tiles");
  assert(tilemap);
  TextureRef tileMapRef = textureManager.LoadTilemapTexture(*tilemap);
  TextureRef collisionMapRef = textureManager.LoadTilemapTexture(*collision_map);

  bool running = true;
  bool paused = true;
  bool debug = false;

  int frames = 0;

  double t = 0;
  double delta = 0;

  double time_scale = 1;

  int last_ticks = SDL_GetTicks();

  while (running) {
    for (ButtonEvent event : GetButtonEvents()) {
      if (event.button_state() == ButtonState::RELEASED) {
        switch (event.button()) {
          case Button::QUIT:
            running = false;
            continue;
          case Button::PAUSE:
            paused = !paused;
            continue;
          case Button::DEBUG:
            debug = !debug;
            continue;
          default:
            break;
        }
      } else if (event.button_state() == ButtonState::PRESSED) {
        switch (event.button()) {
          case Button::PLUS:
            time_scale *= 1.2;
            break;
          case Button::MINUS:
            time_scale /= 1.2;
            break;
          default:
            break;
        }
      }
      // May want to prevent input from triggering two immediate state
      // changes?
      jump_state_system->HandleEvent(&event, bogs);
      lr_state_system->HandleEvent(&event, bogs);
    }

    double dt = (double)(SDL_GetTicks() - last_ticks) / (time_scale * 1000.0);
    if (!paused) {
      jump_state_system->Update(dt, bogs);
      lr_state_system->Update(dt, bogs);
      vector<std::unique_ptr<Event>> events = physics.Update(dt, bogs);
      for (const auto& event : events) {
        auto* collision = static_cast<CollisionEvent*>(event.get());
        jump_state_system->HandleEvent(event.get(), bogs);
        lr_state_system->HandleEvent(event.get(), bogs);
      }
      delta += 8*dt;
      // Interpolate camera to Bog.
      vec2f bog_pos = bogs.at(0).GetComponent<Body>()->bbox.lowerLeft;
      camera.center(bog_pos*0.2 + camera.center()*0.8);
      /* for (const Collision& c : collisions) {
        cout << "a " << c.first << " b " << c.second << " @ (" << c.fix.x << ","
             << c.fix.y << ")" << endl;
      } */
    }
    t += dt;
    frames++;
    last_ticks = SDL_GetTicks();

    // Begin drawing
    display.Clear();

    glDisable(GL_DEPTH_TEST);

    textureManager.BindTexture(bgRef, 0);
    textureManager.BindTexture(-1, 1);
    textureProgram->Use();
    textureProgram->Setup();
    geometryManager.DrawSubTexture(0,0,8,6,-1,-1,2,2);

    textureManager.BindTexture(tileSetRef, 0);
    textureManager.BindTexture(tileMapRef, 1);
    tileProgram->Use();
    tileProgram->map_offset(camera.center() - vec2f{16,12});
    tileProgram->Setup();
    geometryManager.DrawTileMap(camera);

    if (debug) {
      textureManager.BindTexture(collisionSetRef, 0);
      textureManager.BindTexture(collisionMapRef, 1);
      tileProgram->Use();
      tileProgram->map_offset(camera.center() - vec2f{16,12});
      tileProgram->Setup();
      geometryManager.DrawTileMap(camera);
    }

    textureManager.BindTexture(dogRef, 0);
    textureManager.BindTexture(-1, 1);
    textureProgram->Use();
    textureProgram->Setup();

    if (debug) {
      bb_graphics.Update(0 /* unused */, camera, bogs);
    }
    ss_graphics.Update(0 /* unused */, camera, bogs);

    glEnable(GL_DEPTH_TEST);

    display.Swap();

    if (frames % 100 == 0) {
      cout << (float)frames / t << endl;
    }
  }

  return 0;
}
