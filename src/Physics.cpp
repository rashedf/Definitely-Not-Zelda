#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto a_pos = a->getComponent<CTransform>()->pos;
	auto b_pos = b->getComponent<CTransform>()->pos;

	float delta_x = abs(a_pos.x - b_pos.x);
	float delta_y = abs(a_pos.y - b_pos.y);

	float overlap_x = a->getComponent<CBoundingBox>()->halfSize.x + b->getComponent<CBoundingBox>()->halfSize.x - delta_x;
	float overlap_y = a->getComponent<CBoundingBox>()->halfSize.y + b->getComponent<CBoundingBox>()->halfSize.y - delta_y;

	return Vec2(overlap_x, overlap_y);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	auto a_pos = a->getComponent<CTransform>()->pos;
	auto b_pos = b->getComponent<CTransform>()->prevPos;

	float delta_x = abs(a_pos.x - b_pos.x);
	float delta_y = abs(a_pos.y - b_pos.y);

	float overlap_x = a->getComponent<CBoundingBox>()->halfSize.x + b->getComponent<CBoundingBox>()->halfSize.x - delta_x;
	float overlap_y = a->getComponent<CBoundingBox>()->halfSize.y + b->getComponent<CBoundingBox>()->halfSize.y - delta_y;

	return Vec2(overlap_x, overlap_y);
}

Intersect Physics::LineIntersect(const Vec2 & a, const Vec2 & b, const Vec2 & c, const Vec2 & d)
{   
	Vec2 r		= b - a;
	Vec2 s		= d - c;
	Vec2 cma	= c - a;
	float rxs	= r.cross(s);
	float t		= cma.cross(s) / rxs;
	float u		= cma.cross(r) / rxs;

	if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
		return { true, Vec2(a.x + t * r.x, a.y + t * r.y) };
	}
	else {
		return { false, Vec2(0,0) };
	}
}

bool Physics::EntityIntersect(const Vec2 & a, const Vec2 & b, std::shared_ptr<Entity> e)
{
	auto position	= e->getComponent<CTransform>()->pos;
	auto halfSize	= e->getComponent<CBoundingBox>()->halfSize;
	std::vector<Vec2> points;

	points.push_back(Vec2(position.x - halfSize.x, position.y + halfSize.y));
	points.push_back(position + halfSize);
	points.push_back(position - halfSize);
	points.push_back(Vec2(position.x + halfSize.x, position.y - halfSize.y));

	for (int i = 0; i < 4; i++) {
		if (LineIntersect(a, b, points[i], points[(i + 1) % 4]).result) {
			return true;
		}
	}


    return false;
}
