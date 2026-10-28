#define NOMINMAX
#include "CameraController.h"
#include "Operator.h"
#include <algorithm>
#include <cmath>
using namespace KamataEngine;

Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
	Vector3 result;
	result.x = a.x + (b.x - a.x) * t;
	result.y = a.y + (b.y - a.y) * t;
	result.z = a.z + (b.z - a.z) * t;
	return result;
}

void CameraController::Initialize() { camera_.Initialize(); }

void CameraController::Update() {
	if (target_ == nullptr) {
		camera_.UpdateMatrix();
		return;
	}

	// 通常時はプレイヤーを追従し、ヒットストップ中はプレイヤーと敵の中間へ寄せる
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	if (isHitStopFocusEnabled_) {
		targetCoordinates_ = hitStopFocusPosition_ + Vector3(0.0f, targetOffset_.y + 3.0f, targetOffset_.z + 3.0f);
	} else {
		Vector3 targetVelocity = target_->GetVelocity();
		targetCoordinates_.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;
		targetCoordinates_.y = targetWorldTransform.translation_.y + targetOffset_.y;
		targetCoordinates_.z = targetWorldTransform.translation_.z + targetOffset_.z;
	}

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
	if (!isHitStopFocusEnabled_) {

		if (minX <= maxX) {
			camera_.translation_.x = std::clamp(camera_.translation_.x, minX, maxX);
		} else {
			camera_.translation_.x = (movableArea_.left + movableArea_.right) * 0.5f;
		}

		if (minY <= maxY) {
			camera_.translation_.y = std::clamp(camera_.translation_.y, minY, maxY);
		} else {
			camera_.translation_.y = (movableArea_.bottom + movableArea_.top) * 0.5f;
		}
	}

	// 敵死亡時のカメラシェイクを反映
	if (target_->IsKill()) {
		cameraShakeKill_ = true;
	}

	// ヒットストップ時にはシェイクを行わない
	if (isHitStopFocusEnabled_) {
		camera_.UpdateMatrix();
		return;

	} else {

		// 敵討伐時にカメラをシェイク
		if (cameraShakeKill_) {
			// シェイクの強さ
			float shakeStrength = 0.5f;

			// シェイクの周期
			float shakeFrequency = 5.0f;

			// シェイクの時間経過
			static float shakeTime = 0.0f;

			shakeTime += 1.0f / 60.0f; // フレームレートに応じて調整

			cameraShakeOffset_.x = shakeStrength * sinf(shakeFrequency * shakeTime);
			cameraShakeOffset_.y = shakeStrength * cosf(shakeFrequency * shakeTime);

			camera_.translation_.x += cameraShakeOffset_.x;
			camera_.translation_.y += cameraShakeOffset_.y;

			if (shakeTime >= 0.2f) { // シェイク時間が経過したらシェイクを止める
				cameraShakeKill_ = false;
				cameraShakeOffset_ = {0, 0, 0};
				shakeTime = 0.0f;
			}
		}
	}

	// 突進中は小さな高頻度の揺れを加え、速度感を出す。
	if (isDashShakeEnabled_) {
		dashShakeTime_ += 1.0f / 60.0f;
		camera_.translation_.x += 0.018f * std::sin(dashShakeTime_ * 75.0f);
		camera_.translation_.y += 0.014f * std::cos(dashShakeTime_ * 61.0f);
	} else {
		dashShakeTime_ = 0.0f;
	}

	// 行列を更新する
	camera_.UpdateMatrix();
}

void CameraController::Reset() {
	if (target_ == nullptr) {
		camera_.UpdateMatrix();
		return;
	}

	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの座標を計算
	camera_.translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
	camera_.translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;
	camera_.translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;
	Update();
}
