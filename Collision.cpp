#include "Collision.h"

// AABB同士の交差判定
bool Collision::IsCollision(const AABB& a, const AABB& b) {

	// X軸
	if (a.max.x < b.min.x || a.min.x > b.max.x)
		return false;

	// Y軸
	if (a.max.y < b.min.y || a.min.y > b.max.y)
		return false;

	// Z軸（いらなければ消してOK）
	if (a.max.z < b.min.z || a.min.z > b.max.z)
		return false;

	return true;
}