#include "Easing.h"

float Easing::EaseIn(float t, float start, float end) {
	float easedT = t * t;
	return start + (end - start) * easedT;
}

float Easing::EaseOut(float t, float start, float end) {
	float easedT = 1.0f - (1.0f - t) * (1.0f - t);
	return start + (end - start) * easedT;
}