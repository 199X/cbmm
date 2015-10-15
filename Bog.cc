#include "Bog.h"
#include "Entity.h"
#include "State.h"

#include <iostream>
using namespace std;

// TODO: Pull out common update code into a utility function.

namespace {
class Standing : public StateBehavior<JumpStateComponent> {
 public:
  void Enter(JumpStateComponent*, const Entity*) const override {
    // cout << "Enter Standing" << endl;
  }

  void Exit(JumpStateComponent*, const Entity*) const override {
    // cout << "Exit Standing" << endl;
  }

  JumpState Update(JumpStateComponent*, const Entity* entity,
                   const Seconds dt) const override {
    // movement
    Body* body = entity->GetComponent<Body>();
    assert(body);
    Body new_body = *body;
    new_body.vel = body->vel - vec2f{0, 9} * dt;
    new_body.bbox.lowerLeft = new_body.bbox.lowerLeft + body->vel * dt;

    *body = new_body;

    return state();
  }

  JumpState HandleInput(JumpStateComponent*, const Entity* entity,
                        const ButtonEvent* event) const override {
    Body* body = entity->GetComponent<Body>();
    if (event->button() == Button::JUMP &&
        event->button_state() == ButtonState::PRESSED) {
      return JumpState::JUMPING;
    } else if (event->button() == Button::LEFT &&
               event->button_state() == ButtonState::PRESSED) {
      // TODO: Hmm... How to deal with people pressing left while holding right?
      // How do we know how to stop moving when a button is released?
      body->vel.x = -1.0;
    } else if (event->button() == Button::RIGHT &&
               event->button_state() == ButtonState::PRESSED) {
      body->vel.x = 1.0;
    }
    return state();
  }

  JumpState HandleCollision(JumpStateComponent*, const Entity* entity,
                            const CollisionEvent* collision) const override {
    if (collision->second == MAP_BODY_ID) {
      Body* body = entity->GetComponent<Body>();
      assert(body);
      Body new_body = *body;
      new_body.bbox.lowerLeft += collision->fix;
      if (collision->fix.x != 0) new_body.vel.x = 0;
      if (collision->fix.y != 0) new_body.vel.y = 0;
      *body = new_body;
    }
    return state();
  }

  JumpState state() const override { return JumpState::STANDING; }
};

class Jumping : public StateBehavior<JumpStateComponent> {
 public:
  void Enter(JumpStateComponent*, const Entity* entity) const override {
    cout << "Enter Jumping" << endl;
    Body* body = entity->GetComponent<Body>();
    assert(body);
    body->vel.y = 5;
  }
  void Exit(JumpStateComponent*, const Entity*) const override {
    cout << "Exit Jumping" << endl;
  }
  JumpState Update(JumpStateComponent*, const Entity* entity,
                   const Seconds dt) const override {
    // movement
    Body* body = entity->GetComponent<Body>();
    assert(body);
    Body new_body = *body;
    new_body.vel = body->vel - vec2f{0, 9} * dt;
    new_body.bbox.lowerLeft = new_body.bbox.lowerLeft + body->vel * dt;

    *body = new_body;

    return state();
  }
  JumpState HandleInput(JumpStateComponent*, const Entity*,
                        const ButtonEvent*) const override {
    return state();
  }
  JumpState HandleCollision(JumpStateComponent*, const Entity*,
                            const CollisionEvent* collision) const override {
    if (collision->second == MAP_BODY_ID) {
      return JumpState::STANDING;
    }
    return state();
  }
  JumpState state() const override { return JumpState::JUMPING; }
};
}  // namespace

std::unique_ptr<StateMachineSystem<JumpStateComponent>> MakeJumpStateSystem() {
  std::unique_ptr<StateMachineSystem<JumpStateComponent>> system(
      new StateMachineSystem<JumpStateComponent>());
  system->RegisterStateBehavior(std::unique_ptr<Standing>(new Standing()));
  system->RegisterStateBehavior(std::unique_ptr<Jumping>(new Jumping()));

  return system;
}

namespace {
class Left : public StateBehavior<LRStateComponent> {
 public:
  void Enter(LRStateComponent*, const Entity* entity) const override {
    cout << "Enter Left" << endl;
  }

  void Exit(LRStateComponent*, const Entity*) const override {
    cout << "Exit Left" << endl;
  }

  LRState Update(LRStateComponent*, const Entity* entity,
                 const Seconds dt) const override {
    // movement
    Body* body = entity->GetComponent<Body>();
    assert(body);
    Body new_body = *body;
    new_body.vel.x -= 16.0 * dt;
    new_body.vel.x = new_body.vel.x < -4.0 ? -4.0 : new_body.vel.x;
    new_body.bbox.lowerLeft.x = new_body.bbox.lowerLeft.x + body->vel.x * dt;

    *body = new_body;

    return state();
  }

  LRState HandleInput(LRStateComponent*, const Entity*,
                      const ButtonEvent* event) const override {
    if (event->button() == Button::LEFT &&
        event->button_state() == ButtonState::RELEASED) {
      return LRState::STILL;
    } else if (event->button() == Button::RIGHT &&
               event->button_state() == ButtonState::PRESSED) {
      return LRState::RIGHT;
    }
    return state();
  }

  LRState state() const override { return LRState::LEFT; }
};

class Right : public StateBehavior<LRStateComponent> {
 public:
  void Enter(LRStateComponent*, const Entity* entity) const override {
    cout << "Enter Right" << endl;
  }

  void Exit(LRStateComponent*, const Entity*) const override {
    cout << "Exit Right" << endl;
  }

  LRState Update(LRStateComponent*, const Entity* entity,
                 const Seconds dt) const override {
    // movement
    Body* body = entity->GetComponent<Body>();
    assert(body);
    Body new_body = *body;
    new_body.vel.x += 16.0 * dt;
    new_body.vel.x = new_body.vel.x > 4.0 ? 4.0 : new_body.vel.x;
    new_body.bbox.lowerLeft.x = new_body.bbox.lowerLeft.x + body->vel.x * dt;

    *body = new_body;

    return state();
  }

  LRState HandleInput(LRStateComponent*, const Entity*,
                      const ButtonEvent* event) const override {
    if (event->button() == Button::RIGHT &&
        event->button_state() == ButtonState::RELEASED) {
      return LRState::STILL;
    } else if (event->button() == Button::LEFT &&
               event->button_state() == ButtonState::PRESSED) {
      return LRState::LEFT;
    }
    return state();
  }

  LRState state() const override { return LRState::RIGHT; }
};

class Still : public StateBehavior<LRStateComponent> {
 public:
  void Enter(LRStateComponent*, const Entity* entity) const override {
    cout << "Enter Still" << endl;
    Body* body = entity->GetComponent<Body>();
    assert(body);
    body->vel.x = 0;
  }
  void Exit(LRStateComponent*, const Entity*) const override {
    cout << "Exit Still" << endl;
  }
  LRState HandleInput(LRStateComponent*, const Entity*,
                      const ButtonEvent* event) const override {
    if (event->button() == Button::RIGHT &&
        event->button_state() == ButtonState::PRESSED) {
      return LRState::RIGHT;
    } else if (event->button() == Button::LEFT &&
               event->button_state() == ButtonState::PRESSED) {
      return LRState::LEFT;
    }
    return state();
  }
  LRState state() const override { return LRState::STILL; }
};
}  // namespace

std::unique_ptr<StateMachineSystem<LRStateComponent>> MakeLRStateSystem() {
  std::unique_ptr<StateMachineSystem<LRStateComponent>> system(
      new StateMachineSystem<LRStateComponent>());
  system->RegisterStateBehavior(std::unique_ptr<Left>(new Left()));
  system->RegisterStateBehavior(std::unique_ptr<Right>(new Right()));
  system->RegisterStateBehavior(std::unique_ptr<Still>(new Still()));

  return system;
}
