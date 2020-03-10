#pragma once

#include "Common.h"
#include "Entity.h"

struct Intersect { bool result; Vec2 pos; };

namespace Physics
{
    Vec2 GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
    Vec2 GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
    Intersect LineIntersect(const Vec2 & a, const Vec2 & b, const Vec2 & c, const Vec2 & d);
    bool EntityIntersect(const Vec2 & a, const Vec2 & b, std::shared_ptr<Entity> e);
}