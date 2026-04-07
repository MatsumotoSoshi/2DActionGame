#include "WorldMatrix.h"
using namespace KamataEngine;

Matrix4x4 WorldMatrix::MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation) {

	// 各軸のsin/cos
	float cx = cosf(rotation.x);
	float sx = sinf(rotation.x);
	float cy = cosf(rotation.y);
	float sy = sinf(rotation.y);
	float cz = cosf(rotation.z);
	float sz = sinf(rotation.z);

	Matrix4x4 matrix = {
	    scale.x * (cy * cz + sy * sx * sz),
	    scale.x * (cx * sz),
	    scale.x * (cy * sx * sz - sy * cz),
	    0.0f,
	    scale.y * (sy * sx * cz - cy * sz),
	    scale.y * (cx * cz),
	    scale.y * (sy * sz + cy * sx * cz),
	    0.0f,
	    scale.z * (sy * cx),
	    scale.z * (-sx),
	    scale.z * (cy * cx),
	    0.0f,
	    translation.x,
	    translation.y,
	    translation.z,
	    1.0f};

	return matrix;
}

void WorldMatrix::Update(WorldTransform& worldTransform) {

	Vector3 scale = {1.0f, 1.0f, 1.0f};

	worldTransform.matWorld_ = MakeAffineMatrix(scale, worldTransform.rotation_, worldTransform.translation_);

	worldTransform.TransferMatrix();
}