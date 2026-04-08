#pragma once
#include "AABB.h" 

class Collision 
{
public:
	// AABB同士の交差判定
	static bool IsCollision(const AABB& a, const AABB& b);
};
