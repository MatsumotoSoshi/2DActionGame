#define NOMINMAX
#include "ShieldEnemy.h"
#include <numbers>
#include "Operator.h"
#include "Easing.h"
#include <algorithm>
using namespace KamataEngine;

void ShieldEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) { 
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;

	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0, 0};

	walkTimmer_ = 0.0f;
}

Vector3 ShieldEnemy::GetWorldPosition() {
	Vector3 worldPos;

	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB ShieldEnemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = {worldPos.x - kWidthHalf, worldPos.y - kHeightHalf, worldPos.z - kWidthHalf};
	aabb.max = {worldPos.x + kWidthHalf, worldPos.y + kHeightHalf, worldPos.z + kWidthHalf};

	return aabb;
}

void ShieldEnemy::onCollision(Player* player) { 
	// 敵がやられているならなにもしない
	if (behavior_ == Behavior::kDeath) {
		return;
	}

	// プレイヤーが攻撃中なら敵が死ぬ
	if (player->IsAttack()) {
		// 前方であればガード成功
		if (player->GetDirection() != lrDirection_) {
			gameScene_->CreateGuardEffect(worldTransform_);

			// playerノックバック
			player->RequestKnockBack();

			// 敵は停止
			behaviorRequest_ = Behavior::kStop; 

			return; // 正面なのでガード
		}

		// 敵のふるまいをデス演出に変更
		gameScene_->StartHitStop();
		behaviorRequest_ = Behavior::kDeath;

		Vector3 effectPos = (this->worldTransform_.translation_ + player->GetWorldTransform().translation_) / 2.0f;

		// 一時的なWorldTransformを作って座標をセット
		WorldTransform tempTransform;
		tempTransform.translation_ = effectPos;

		// GameSceneの生成関数を呼ぶ
		gameScene_->CreateHitEffect(tempTransform);
	}

}

void ShieldEnemy::Update() {
	if (behavior_ != behaviorRequest_) {
		behavior_ = behaviorRequest_;

		// 初期化処理
		switch (behavior_) {
		case Behavior::kDeath:
			animationTimer_ = 0;
			break;

		case Behavior::kStop:
			animationTimer_ = 0;
			break;
		}
	}

	switch (behavior_) {
	case Behavior::kWalk: {

		// 移動
		worldTransform_.translation_ += velocity_;

		walkTimmer_ += 1.0f / 60.0f;

		// 回転アニメーション
		float param = std::sin(walkTimmer_ * std::numbers::pi_v<float> * 2.0f / kWalkMotionCycle);
		float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;

		float base = -std::numbers::pi_v<float> / 2.0f; // 左向き

		worldTransform_.rotation_.y = base + degree * std::numbers::pi_v<float> / 180.0f;

		// ワールド行列を更新
		worldMatrix_.Update(worldTransform_);

		break;
	}

	case Behavior::kDeath: {
		// アニメーションのタイマーを加算する
		animationTimer_++;

		float deathTime = 45.0f;

		// 進行度（0.0～1.0）
		float t = static_cast<float>(animationTimer_) / deathTime;
		t = std::clamp(t, 0.0f, 1.0f);

		// // Y軸周りの回転角をイージングで変化させる
		worldTransform_.rotation_.y = Easing::EaseOut(t, 0.0f, std::numbers::pi_v<float> * 2.0f);

		// X軸周りの回転角をイージングで変化させる
		worldTransform_.rotation_.x = Easing::EaseOut(t, 0.0f, -std::numbers::pi_v<float> / 2.0f);

		// スケールを小さくして消える
		float scale = Easing::EaseIn(t, 1.0f, 0.4f);
		worldTransform_.scale_ = {scale, scale, scale};

		// 行列更新
		worldMatrix_.Update(worldTransform_);

		if (animationTimer_ > deathTime) {
			isDead_ = true;
		}

		break;
	}
     
	case Behavior::kStop: {
		animationTimer_++;

		float t = static_cast<float>(animationTimer_) / 30.0f;
		t = std::clamp(t, 0.0f, 1.0f);

		// のけぞり
		float angle = 20.0f * std::numbers::pi_v<float> / 180.0f;

		float recoil;
		if (t < 0.5f) {
			// 倒れる
			recoil = Easing::EaseOut(t * 2.0f, 0.0f, angle);
		} else {
			// 戻る
			recoil = Easing::EaseIn((t - 0.5f) * 2.0f, angle, 0.0f);
		}

		// 進行方向と逆に傾ける
		if (lrDirection_ == LRDirection::kLeft) {
			worldTransform_.rotation_.x = -recoil;
		} else {
			worldTransform_.rotation_.x = -recoil;
		}

		if (animationTimer_ > 30) {
			behaviorRequest_ = Behavior::kWalk;
			velocity_ = {-kWalkSpeed, 0, 0};

			// 回転リセット
			worldTransform_.rotation_.x = 0.0f;
		}

		worldMatrix_.Update(worldTransform_);
		break;
	}
	}
}

void ShieldEnemy::Draw(const Camera& camera) {
	model_->Draw(worldTransform_, camera); 
}
