#pragma once
#include "KamataEngine.h"

inline KamataEngine::Vector3 operator+(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

inline KamataEngine::Vector3& operator+=(KamataEngine::Vector3& a, const KamataEngine::Vector3& b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}
