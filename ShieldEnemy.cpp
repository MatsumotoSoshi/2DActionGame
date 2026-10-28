#define NOMINMAX
#include "ShieldEnemy.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include "Easing.h"
#include "MapChipField.h"
#include "Operator.h"

using namespace KamataEngine;

namespace {
constexpr float kPi = std::numbers::pi_v<float>;
}

void ShieldEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	aimAngle_ = kPi;
	worldTransform_.rotation_ = {0.0f, 0.0f, aimAngle_ - kPi / 2.0f};
	lrDirection_ = LRDirection::kLeft;
	velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
	worldMatrix_.Update(worldTransform_);
	walkTimmer_ = 0.0f;
}

Vector3 ShieldEnemy::GetWorldPosition() {
	return worldTransform_.translation_;
}

AABB ShieldEnemy::GetAABB() {
	const Vector3 position = GetWorldPosition();

	// 各方向に0.05ずつ判定を広げる
	constexpr float kHitPadding = 0.05f;
	const float halfWidth = kWidthHalf + kHitPadding;
	const float halfHeight = kHeightHalf + kHitPadding;

	AABB aabb;
	aabb.min = {position.x - halfWidth, position.y - halfHeight, position.z - kWidthHalf};
	aabb.max = {position.x + halfWidth, position.y + halfHeight, position.z + kWidthHalf};

	return aabb;
}

Vector3 ShieldEnemy::GetTipPosition() const {
	return {worldTransform_.translation_.x + std::cos(aimAngle_) * kTipOffset,
	        worldTransform_.translation_.y + std::sin(aimAngle_) * kTipOffset,
	        worldTransform_.translation_.z};
}

bool ShieldEnemy::IsDashing() const {
	return behavior_ == Behavior::kDash && behaviorRequest_ == Behavior::kDash;
}

bool ShieldEnemy::IsBlockedAt(const Vector3& position) const {
	if (mapChipField_ == nullptr) {
		return false;
	}

	for (float xSign : {-1.0f, 1.0f}) {
		for (float ySign : {-1.0f, 1.0f}) {
			const Vector3 corner = {position.x + xSign * kWidthHalf, position.y + ySign * kHeightHalf, position.z};
			const MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(corner);
			if (IsSolidMapChipType(mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex))) {
				return true;
			}
		}
	}

	return false;
}

void ShieldEnemy::onCollision(Player* player) {
	if (player == nullptr || IsInvincible(player->GetDashSpeed()) || behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath || !player->IsAttack()) {
		return;
	}

	if (player->GetDirection() != lrDirection_) {
		gameScene_->CreateGuardEffect(worldTransform_);
		player->RequestKnockBack();
		behaviorRequest_ = Behavior::kStop;
		return;
	}

	behaviorRequest_ = Behavior::kDeath;
	const Vector3 effectPos = (worldTransform_.translation_ + player->GetWorldTransform().translation_) / 2.0f;
	WorldTransform effectTransform;
	effectTransform.translation_ = effectPos;
	gameScene_->StartHitStop(effectPos);
	gameScene_->CreateHitEffect(effectTransform);
	gameScene_->RegisterEnemyDefeated();
}

void ShieldEnemy::Update() {
	if (behavior_ != behaviorRequest_) {
		behavior_ = behaviorRequest_;
		switch (behavior_) {
		case Behavior::kDash:
			canCharge_ = false;

			if (target_ != nullptr) {
				const Vector3 toTarget = {target_->GetWorldTransform().translation_.x - worldTransform_.translation_.x,
				                          target_->GetWorldTransform().translation_.y - worldTransform_.translation_.y,
			                          target_->GetWorldTransform().translation_.z - worldTransform_.translation_.z};
				aimAngle_ = std::atan2(toTarget.y, toTarget.x);
			}

			worldTransform_.rotation_.z = aimAngle_ - kPi / 2.0f;
			break;
		case Behavior::kStuck:
			stuckTimer_ = 0;
			velocity_ = {};
			break;
		case Behavior::kSearch:
			searchTimer_ = 0;
			velocity_ = {};
			break;
		case Behavior::kDeath:
			animationTimer_ = 0;
			break;
		case Behavior::kStop:
			animationTimer_ = 0;
			canCharge_ = false;
			break;
		case Behavior::kWalk:
		default:
			break;
		}
	}

	// シールド敵も静止した標的にし、撃破演出だけを更新する。
	if (behavior_ != Behavior::kDeath) {
		velocity_ = {};
		worldMatrix_.Update(worldTransform_);
		return;
	}

	if (target_ != nullptr) {
		const Vector3 toTarget = {target_->GetWorldTransform().translation_.x - worldTransform_.translation_.x,
		                          target_->GetWorldTransform().translation_.y - worldTransform_.translation_.y,
		                          0.0f};
		if (std::hypot(toTarget.x, toTarget.y) >= kDetectionResetRange) {
			canCharge_ = true;
		}
	}

	switch (behavior_) {
	case Behavior::kWalk: {
		if (target_ != nullptr && canCharge_) {
			const Vector3 toTarget = {target_->GetWorldTransform().translation_.x - worldTransform_.translation_.x,
			                          target_->GetWorldTransform().translation_.y - worldTransform_.translation_.y,
			                          0.0f};
			if (std::hypot(toTarget.x, toTarget.y) <= kDetectionRange) {
				behaviorRequest_ = Behavior::kDash;
				break;
			}
		}

		const Vector3 candidate = {worldTransform_.translation_.x + velocity_.x, worldTransform_.translation_.y, worldTransform_.translation_.z};
		if (IsBlockedAt(candidate)) {
			velocity_.x = -velocity_.x;
		} else {
			worldTransform_.translation_ = candidate;
		}

		aimAngle_ = velocity_.x >= 0.0f ? 0.0f : kPi;
		lrDirection_ = aimAngle_ == 0.0f ? LRDirection::kRight : LRDirection::kLeft;
		walkTimmer_ += 1.0f / 60.0f;
		const float bob = std::sin(walkTimmer_ * kPi * 2.0f / kWalkMotionCycle);
		worldTransform_.rotation_.x = (kWalkMotionAngleStart + kWalkMotionAngleEnd * (bob + 1.0f) / 2.0f) * kPi / 180.0f;
		worldTransform_.rotation_.z = aimAngle_ - kPi / 2.0f;
		break;
	}
	case Behavior::kDash: {
		if (target_ != nullptr) {
			const Vector3 toTarget = {target_->GetWorldTransform().translation_.x - worldTransform_.translation_.x,
			                          target_->GetWorldTransform().translation_.y - worldTransform_.translation_.y,
			                          0.0f};
			const float targetAngle = std::atan2(toTarget.y, toTarget.x);
			const float angleDifference = std::remainder(targetAngle - aimAngle_, 2.0f * kPi);
			aimAngle_ += std::clamp(angleDifference, -kHomingTurnRate, kHomingTurnRate);
		}

		lrDirection_ = std::cos(aimAngle_) >= 0.0f ? LRDirection::kRight : LRDirection::kLeft;
		velocity_ = {std::cos(aimAngle_) * kDashSpeed, std::sin(aimAngle_) * kDashSpeed, 0.0f};
		// 高速移動時にブロックを飛び越えないよう、移動経路を細かく分けて調べる。
		const float distance = std::hypot(velocity_.x, velocity_.y);
		const int stepCount = std::max(1, static_cast<int>(std::ceil(distance / 0.1f)));
		const Vector3 step = {velocity_.x / stepCount, velocity_.y / stepCount, 0.0f};
		bool hitWall = false;
		for (int i = 0; i < stepCount; ++i) {
			const Vector3 candidate = worldTransform_.translation_ + step;
			if (IsBlockedAt(candidate)) {
				hitWall = true;
				break;
			}

			worldTransform_.translation_ = candidate;
		}

		if (hitWall) {
			velocity_ = {};
			behaviorRequest_ = Behavior::kStuck;
		}

		worldTransform_.rotation_.x = 0.0f;
		worldTransform_.rotation_.z += kSpinSpeed;
		break;
	}
	case Behavior::kStuck:
		++stuckTimer_;
		worldTransform_.rotation_.x = std::sin(static_cast<float>(stuckTimer_) * 0.35f) * 0.04f;
		worldTransform_.rotation_.z = aimAngle_ - kPi / 2.0f;
		if (stuckTimer_ >= kWallStuckDuration) {
			canCharge_ = true;
			behaviorRequest_ = Behavior::kSearch;
		}
		break;
	case Behavior::kSearch: {
		++searchTimer_;
		if (target_ != nullptr) {
			const Vector3 toTarget = {target_->GetWorldTransform().translation_.x - worldTransform_.translation_.x,
			                          target_->GetWorldTransform().translation_.y - worldTransform_.translation_.y,
			                          0.0f};
			aimAngle_ = std::atan2(toTarget.y, toTarget.x);
			lrDirection_ = std::cos(aimAngle_) >= 0.0f ? LRDirection::kRight : LRDirection::kLeft;
			if (canCharge_ && std::hypot(toTarget.x, toTarget.y) <= kDetectionRange) {
				behaviorRequest_ = Behavior::kDash;
			}
		}

		worldTransform_.rotation_.z = aimAngle_ - kPi / 2.0f;
		if (searchTimer_ >= kSearchDuration && behaviorRequest_ == behavior_) {
			velocity_ = {std::cos(aimAngle_) >= 0.0f ? kWalkSpeed : -kWalkSpeed, 0.0f, 0.0f};
			behaviorRequest_ = Behavior::kWalk;
		}

		break;
	}
	case Behavior::kStop: {
		++animationTimer_;
		const float t = std::clamp(static_cast<float>(animationTimer_) / 30.0f, 0.0f, 1.0f);
		const float angle = 20.0f * kPi / 180.0f;
		const float recoil = t < 0.5f ? Easing::EaseOut(t * 2.0f, 0.0f, angle) : Easing::EaseIn((t - 0.5f) * 2.0f, angle, 0.0f);
		worldTransform_.rotation_.x = -recoil;
		if (animationTimer_ > 30) {
			behaviorRequest_ = Behavior::kWalk;
			canCharge_ = true;
			velocity_ = {-kWalkSpeed, 0.0f, 0.0f};
			worldTransform_.rotation_.x = 0.0f;
		}

		break;
	}
	case Behavior::kDeath: {
		++animationTimer_;
		const float t = std::clamp(static_cast<float>(animationTimer_) / 45.0f, 0.0f, 1.0f);
		worldTransform_.rotation_.z = aimAngle_ - kPi / 2.0f + Easing::EaseOut(t, 0.0f, 2.0f * kPi);
		worldTransform_.rotation_.x = Easing::EaseOut(t, 0.0f, -kPi / 2.0f);
		const float scale = Easing::EaseIn(t, 1.0f, 0.4f);
		worldTransform_.scale_ = {scale, scale, scale};
		if (animationTimer_ > 45) {
			isDead_ = true;
		}

		break;
	}
	}
	worldMatrix_.Update(worldTransform_);
}

void ShieldEnemy::Draw(const Camera& camera) {
	if (model_ != nullptr) {
		model_->Draw(worldTransform_, camera);
	}
}
