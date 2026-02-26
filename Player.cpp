#define NOMINMAX
#include "Player.h"
#include <numbers>
#include <algorithm>
#include "MapChipField.h"
#include <array>
using namespace KamataEngine;

Vector3 operator+(const Vector3& a, const Vector3& b) { 
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector3& operator+=(Vector3& a, const Vector3& b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0}
    };

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	velocity_ = {0, 0, 0}; 

}

void Player::Move() {
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

		} else {

			// 減速処理
			velocity_.x *= (1.0f - kAttenuation);

			// 微小値なら完全停止
			if (std::abs(velocity_.x) < 0.01f) {
				velocity_.x = 0.0f;
			}
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

}

void Player::CollisionMap(CollisionMapInfo& info) { 

	if (info.amountOfMovement.y <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionNow;

	for (uint32_t i = 0; i < positionNow.size(); ++i) {
		positionNow[i] = CornerPosition(worldTransform_.translation_ + info.amountOfMovement, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNow[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNow[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	//	ブロックにヒット？
	if (hit) {
		// 移動後の自キャラ上端座標
		Vector3 playerTop = CornerPosition(worldTransform_.translation_ + info.amountOfMovement, kRightTop);

		// めり込み先を排除する方向に移動量を設定
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(playerTop);

		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);

		// y移動量
		float yMove = rect.bottom - playerTop.y - kBlank;

		info.amountOfMovement.y = std::max(0.0f, yMove);

		// 天井衝突フラグ
		info.ceilingCollision = true;
	}
}

void Player::ResultMove(const CollisionMapInfo& info) { 
	worldTransform_.translation_ += info.amountOfMovement;
}

void Player::CollisionCeiling(const CollisionMapInfo& info) {
	// 天井に当たった？
	if (info.ceilingCollision) {
		DebugText::GetInstance()->ConsolePrintf("hit ceiling\n");
		velocity_.y = 0;
	}
}

void Player::Update() {
	// 移動入力
	Move();

	// 衝突情報を入力
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.amountOfMovement = velocity_;

	// マップ衝突チェック
	CollisionMap(collisionMapInfo);

	// 判定結果を反映して移動させる
	ResultMove(collisionMapInfo);

	// 天井に接触している場合の処理
	CollisionCeiling(collisionMapInfo);	

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
