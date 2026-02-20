#define NOMINMAX
#include "Player.h"
#include <numbers>
#include <algorithm>
using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	velocity_ = {0, 0, 0}; 

}

void Player::Update() {
	// 移動入力
	if (onGround_) {

		// 左右移動
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			KamataEngine::Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration;
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					// 旋回開始時の角度を記録
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {

				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					// 旋回開始時の角度を記録
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					// 旋回タイマーに時間を設定する
					turnTimer_ = kTimeTurn;
				}
			}

			velocity_.x += acceleration.x;

			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		}

		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			// ジャンプ初速
			velocity_.y = kJumpAcceleration_;
			onGround_ = false;
		}

    // 空中
	} else {
		// 落下速度
		velocity_.y -= kGravityAcceleration_;
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed_);
	}
		
	// プレイヤーの位置を更新
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;

	// 地面との当たり判定
	if (velocity_.y < 0) {
		// Y座標が地面以下になったら着地
		if (worldTransform_.translation_.y <= 1.0f && velocity_.y <= 0.0f) {
			landing_ = true;
		}
	}

	// 着地処理
	if (landing_) {
		// めり込み排斥
		worldTransform_.translation_.y = 1.0f;
		// 摩擦で横方向速度が減衰する
		velocity_.x *= (1.0f - kAttenuation);
		// 下方向速度をリセット
		velocity_.y = 0.0f;
		// 着地状態に移行
		onGround_ = true;
		landing_ = false;
	}

	// 旋回制御
	if (turnTimer_ > 0.0f) {

		turnTimer_ -= 1.0f / 60.0f;

		if (turnTimer_ < 0.0f) {
			turnTimer_ = 0.0f;
		}

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> / 2.0f * 3.0f};
		// 状態に応じた角度を取得
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		float t = 1.0f - (turnTimer_ / kTimeTurn);
		// イージング関数（EaseInOut）
		float easeInOut = t * t * (3.0f - 2.0f * t);

		// 自キャラの角度を認定する
		worldTransform_.rotation_.y = turnFirstRotationY_ + (destinationRotationY - turnFirstRotationY_) * easeInOut;
	}

	// ワールド行列を更新
	worldMatrix_.Update(worldTransform_);
} 

void Player::Draw(const Camera& camera) { 
	 model_->Draw(worldTransform_, camera);
}
