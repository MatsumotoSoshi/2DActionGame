#include "WorldMatrix.h"
using namespace KamataEngine;

Matrix4x4 WorldMatrix::MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation) {
	float c = cosf(rotation.y);
	float s = sinf(rotation.y);

	Matrix4x4 matrix = {
		scale.x * c, 0.0f, -scale.x * s, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f, 
		scale.z * s, 0.0f, scale.z * c, 0.0f,
		translation.x, translation.y, translation.z, 1.0f
	};

	return matrix;
}

void WorldMatrix::Update(WorldTransform& worldTransform) {

	Vector3 scale = {1.0f, 1.0f, 1.0f};

	worldTransform.matWorld_ = MakeAffineMatrix(scale, worldTransform.rotation_, worldTransform.translation_);

	worldTransform.TransferMatrix();
}