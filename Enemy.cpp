#include "Enemy.h"
#include <numbers>
#include "Operator.h"
using namespace KamataEngine;

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) { 
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;

	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0, 0};

	walkTimmer_ = 0.0f;
}

Vector3 Enemy::GetWorldPosition() {
	Vector3 worldPos;

	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = {worldPos.x - kWidthHalf, worldPos.y - kHeightHalf, worldPos.z - kWidthHalf};
	aabb.max = {worldPos.x + kWidthHalf, worldPos.y + kHeightHalf, worldPos.z + kWidthHalf};

	return aabb;
}

void Enemy::onCollision(const Player* player) { 
	(void)player; 
}

void Enemy::Update() {
	// 移動
	worldTransform_.translation_ += velocity_;

	walkTimmer_ += 1.0f / 60.0f;

	// 回転アニメーション
	float param = std::sin(walkTimmer_ * std::numbers::pi_v<float> * 2.0f / kWalkMotionCycle);
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	worldTransform_.rotation_.x = degree * std::numbers::pi_v<float> / 180.0f;

	// ワールド行列を更新
	worldMatrix_.Update(worldTransform_);
}

void Enemy::Draw(const Camera& camera) {
	model_->Draw(worldTransform_, camera); 
}