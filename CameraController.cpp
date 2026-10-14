#define NOMINMAX
#include "CameraController.h"
#include <algorithm>
using namespace KamataEngine;

Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
	Vector3 result;
	result.x = a.x + (b.x - a.x) * t;
	result.y = a.y + (b.y - a.y) * t;
	result.z = a.z + (b.z - a.z) * t;
	return result;
}

void CameraController::Initialize() { 
	camera_.Initialize();
}

void CameraController::Update() {
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// プレイヤーの速度参照
	Vector3 targetVelocity = target_->GetVelocity();
	// 追従対象とオフセットからカメラの座標を計算
	targetCoordinates_.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;
	targetCoordinates_.y = targetWorldTransform.translation_.y + targetOffset_.y;
	targetCoordinates_.z = targetWorldTransform.translation_.z + targetOffset_.z;


	// 座標補間によりゆったり追従
	camera_.translation_ = Lerp(camera_.translation_, targetCoordinates_, kInterpolationRate); 

	// 移動範囲制限 
	float halfWidth = 12.0f;
	float halfHeight = 6.0f;

	// clamp用の最小・最大
	float minX = movableArea_.left + halfWidth;
	float maxX = movableArea_.right - halfWidth;

	float minY = movableArea_.bottom + halfHeight;
	float maxY = movableArea_.top - halfHeight;

	camera_.translation_.x = std::max(camera_.translation_.x, targetWorldTransform.translation_.x + margin_.left);

	camera_.translation_.x = std::min(camera_.translation_.x, targetWorldTransform.translation_.x + margin_.right);

	camera_.translation_.y = std::max(camera_.translation_.y, targetWorldTransform.translation_.y + margin_.bottom);

	camera_.translation_.y = std::min(camera_.translation_.y, targetWorldTransform.translation_.y + margin_.top);

	// マップが画面より小さい場合の安全処理
	if (minX <= maxX) {
		camera_.translation_.x = std::clamp(camera_.translation_.x, minX, maxX);
	}

	if (minY <= maxY) {
		camera_.translation_.y = std::clamp(camera_.translation_.y, minY, maxY);
	}

	// 敵死亡時のカメラ揺れを反映
	if (target_->IsKill()) {
		cameraShakeKill_ = true;
	}
	

	if (cameraShakeKill_) {
		// 揺れの強さ
		float shakeStrength = 0.8f;

		// 揺れの周期
		float shakeFrequency = 30.0f;

		// 揺れの時間経過
		static float shakeTime = 0.0f;

		shakeTime += 1.0f / 60.0f; // フレームレートに応じて調整

		cameraShakeOffset_.x = shakeStrength * sinf(shakeFrequency * shakeTime);
		cameraShakeOffset_.y = shakeStrength * cosf(shakeFrequency * shakeTime);

		camera_.translation_.x += cameraShakeOffset_.x;
		camera_.translation_.y += cameraShakeOffset_.y;

		if (shakeTime >= 0.2f) { // 揺れ時間が経過したら揺れを止める
			cameraShakeKill_ = false;
			cameraShakeOffset_ = {0, 0, 0};
			shakeTime = 0.0f;
		}
	}

	
	// 行列を更新する
	camera_.UpdateMatrix();
}

void CameraController::Reset() { 
	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの座標を計算
	camera_.translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
	camera_.translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;
	camera_.translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;
}