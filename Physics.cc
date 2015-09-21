#include <cassert>
#include <cmath>

#include "Physics.h"

namespace {
bool PointMapBelow(const TileMap& tile_map, const vec2f& contact_pt,
                   vec2f* fix) {
  float map_y = floor(contact_pt.y);

  int tile_type = tile_map.At(contact_pt.x, contact_pt.y);
  if (tile_type == TILE_BLOCK) {
    fix->y = (map_y + 1) - contact_pt.y;

    return true;
  }

  return false;
}

// TODO: Velocity after @fix should be parallel to the slope so jittering
// doesn't occur.
bool PointMapSlope(const TileMap& tile_map, const vec2f& contact_pt,
                   vec2f* fix) {
  float map_x = floor(contact_pt.x);
  float map_y = floor(contact_pt.y);

  int tile_type = tile_map.At(contact_pt.x, contact_pt.y);
  if (tile_type == TILE_SLOPE_01) {
    float dist_from_slope = (contact_pt.y - map_y) - (contact_pt.x - map_x);
    if (dist_from_slope < 0) {
      fix->y = -dist_from_slope;
      return true;
    }
    return false;
  } else if (tile_type == TILE_SLOPE_10) {
    float dist_from_slope =
        (contact_pt.y - map_y) - (1 - (contact_pt.x - map_x));
    if (dist_from_slope < 0) {
      fix->y = -dist_from_slope;
      return true;
    }
    return false;
  }

  return false;
}

bool PointMapSide(const TileMap& tile_map, const vec2f& contact_pt,
                  vec2f* fix) {
  int tile_type = tile_map.At(contact_pt.x, contact_pt.y);
  if (tile_type == TILE_BLOCK) {
    // Might want to split this up into two functions...
    fix->x = round(contact_pt.x) - contact_pt.x;
    return true;
  }

  return false;
}

// Returns true if a and b overlap: A [  { ]  } B
// @fix is set to the distance A must move to not overlap B.
bool AxisCheck(double a1, double a2, double b1, double b2, double* fix) {
  if (b1 < a1) {
    // Flip so a1 is always left of b1
    if (AxisCheck(b1, b2, a1, a2, fix)) {
      *fix = -*fix;
      return true;
    }
  } else {
    *fix = b1 - a2;
    if (*fix < 0) {
      return true;
    }
  }
  return false;
}
}  // namespace

// Returns true if @first and @second collide. @fix is set to the correction
// that @first must make to no longer collide with @second.
bool Physics::RectRectCollision(const Rect& first, const Rect& second,
                                vec2f* fix) {
  double x_fix, y_fix;
  if (AxisCheck(first.upperLeft.x, first.upperLeft.x + first.w,
                second.upperLeft.x, second.upperLeft.x + second.w, &x_fix) &&
      AxisCheck(first.upperLeft.y - first.h, first.upperLeft.y,
                second.upperLeft.y - second.h, second.upperLeft.y, &y_fix)) {
    if (abs(x_fix) < abs(y_fix)) {
      fix->x = x_fix;
      fix->y = 0;
    } else {
      fix->x = 0;
      fix->y = y_fix;
    }
    return true;
  }
  return false;
}

bool Physics::RectMapCollision(const Rect& rect, vec2f* fix) {
  int tile_type = tile_map.At(rect.upperLeft.x + (rect.w / 2.0),
                                    rect.upperLeft.y - rect.h);
  if (tile_type == TILE_SLOPE_01 || tile_type == TILE_SLOPE_10) {
    return PointMapSlope(tile_map, {rect.upperLeft.x + (rect.w / 2.0),
                                    rect.upperLeft.y - rect.h + fix->y},
                         fix);
  }

  bool collided_below =
      PointMapBelow(tile_map, {rect.upperLeft.x, rect.upperLeft.y - rect.h},
                    fix) ||
      PointMapBelow(tile_map,
                    {rect.upperLeft.x + rect.w, rect.upperLeft.y - rect.h},
                    fix);

  bool collided_side =
      PointMapSide(tile_map,
                   {rect.upperLeft.x, rect.upperLeft.y - rect.h + fix->y},
                   fix) ||
      PointMapSide(tile_map, {rect.upperLeft.x + rect.w,
                              rect.upperLeft.y - rect.h + fix->y},
                   fix) ||
      PointMapSide(tile_map, {rect.upperLeft.x, rect.upperLeft.y + fix->y},
                   fix) ||
      PointMapSide(tile_map,
                   {rect.upperLeft.x + rect.w, rect.upperLeft.y + fix->y}, fix);

  return collided_below || collided_side;
}

vector<std::unique_ptr<Event>> Physics::Update(Seconds,
                                               const vector<Entity>& entities) {
  vector<std::unique_ptr<Event>> collisions;

  for (size_t i = 0; i < entities.size(); ++i) {
    auto* body = entities[i].GetComponent<Body>();
    assert(body);
    if (body->enabled) {
      // tilemap collision
      vec2f fix{0, 0};
      if (RectMapCollision(body->bbox, &fix)) {
        std::unique_ptr<CollisionEvent> collision(new CollisionEvent());
        collision->first = i;
        collision->second = MAP_BODY_ID;
        collision->fix = fix;
        collisions.push_back(std::move(collision));
      }

      // rect rect collisions
      vec2f rect_fix{0, 0};
      auto collide_to = body;
      collide_to++;
      for (size_t target_i = i + 1; target_i < entities.size(); ++target_i) {
        auto* target_body = entities[target_i].GetComponent<Body>();
        assert(target_body);
        if (target_body->enabled &&
            RectRectCollision(body->bbox, target_body->bbox, &rect_fix)) {
          std::unique_ptr<CollisionEvent> collision(new CollisionEvent());
          collision->first = i;
          collision->second = target_i;
          collision->fix = rect_fix;
          collisions.push_back(std::move(collision));
        }
      }
    }
  }

  return collisions;
}
