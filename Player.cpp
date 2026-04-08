#define NOMINMAX
#include "Player.h"
#include <numbers>
#include <algorithm>
#include "MapChipField.h"
#include "Operator.h"
using namespace KamataEngine;

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

Vector3 Player::CornerPosition(const Vector3& center, Corner corner ){
	Vector3 offSetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kRightBottom
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, //  kLeftBottom
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, //  kRightTop
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}, //  kLeftTop
	};

	return center + offSetTable[static_cast<uint32_t>(corner)];
}

Vector3 Player::GetWorldPosition() {
	Vector3 worldPos;

	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Player::GetAABB() { 
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	aabb.min = {worldPos.x - kWidthHalf, worldPos.y - kHeightHalf, worldPos.z - kWidthHalf};
	aabb.max = {worldPos.x + kWidthHalf, worldPos.y + kHeightHalf, worldPos.z + kWidthHalf};

	return aabb;
}

void Player::onCollision(const Enemy* enemy) { 
	(void)enemy; 
	// ジャンプ開始
	velocity_ += Vector3(0, kJumpAcceleration_, 0);
}

void Player::CollisionMapTop(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.move_.y <= 0) {
		return;
    }

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move_, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// 移動後の自キャラ上端座標
		Vector3 playerTop = CornerPosition(worldTransform_.translation_ + info.move_, kRightTop);
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(playerTop);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			float yMove = (rect.bottom - worldTransform_.translation_.y) - (kHeightHalf + kBlank);
			info.move_.y = std::max(0.0f, yMove);

			// 天井に当たったことを記録する
			info.hitCeiling_ = true;
		}
	}
}

void Player::CollisionMapBottom(CollisionMapInfo& info) { 
	// 下降あり？
	if (info.move_.y >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move_, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真下の当たり判定を行う
	bool hit = false;
	// 左下点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// 移動後の自キャラ下端座標
		Vector3 playerBottom = CornerPosition(worldTransform_.translation_ + info.move_, kRightBottom);
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(playerBottom);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow; 
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			float yMove = (rect.top - worldTransform_.translation_.y) + (kHeightHalf + kBlank);
			info.move_.y = std::min(0.0f, yMove);

			// 地面に当たったことを記録する
			info.hitGround_ = true;
		}
	}
}

void Player::CollisionMapRight(CollisionMapInfo & info) {
	// 右移動あり？
	if (info.move_.x <= 0) {
			return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move_, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真右の当たり判定を行う
	bool hit = false;
	// 右上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// 移動後の自キャラ右端座標
		Vector3 playerRight = CornerPosition(worldTransform_.translation_ + info.move_, kRightTop);
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(playerRight);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
		if (indexSetNow.xIndex != indexSet.xIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			float xMove = (rect.left - worldTransform_.translation_.x) - (kWidthHalf + kBlank);
			info.move_.x = std::min(0.0f, xMove);

			// 壁に当たったことを記録する
			info.hitWall_ = true;
		}
	}
}

void Player::CollisionMapLeft(CollisionMapInfo& info) {
	// 左移動あり？
	if (info.move_.x >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionNew;
	for (uint32_t i = 0; i < positionNew.size(); ++i) {
		positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move_, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真左の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
		
	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// 移動後の自キャラ左端座標
		Vector3 playerLeft = CornerPosition(worldTransform_.translation_ + info.move_, kLeftTop);
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(playerLeft);
		// 現在座標が壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
		if (indexSetNow.xIndex != indexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			float xMove = (rect.right - worldTransform_.translation_.x) + (kWidthHalf + kBlank);
			info.move_.x = std::max(0.0f, xMove);
			// 壁に当たったことを記録する
			info.hitWall_ = true;
		}
	}
}

void Player::MoveResult(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translation_ += info.move_;
}

void Player::CollisionCeiling(const CollisionMapInfo& info) {
	// 天井に当たった？
	if (info.hitCeiling_) {
		velocity_.y = 0.0f;
	}
}

void Player::GroundTransition(const CollisionMapInfo& info) {
	// 自キャラが接地状態？
	if (onGround_) {
		// 接地状態の処理

		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			// 移動後の4つの角の座標
			std::array<Vector3, kNumCorner> positionNew;

			for (uint32_t i = 0; i < positionNew.size(); ++i) {
				positionNew[i] = CornerPosition(worldTransform_.translation_ + info.move_, static_cast<Corner>(i));
			}

			MapChipType mapChipType;
			// 真下の当たり判定を行う
			bool hit = false;
			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom] + Vector3(0, -epsilon, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右下点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom] + Vector3(0, -epsilon, 0));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 落下開始
			if (!hit) {
				// 空中状態に切り替える
				onGround_ = false;
			}
		}

	} else {
		// 空中状態の処理
		// 着地フラグ
		if (info.hitGround_) {

			// 着地状態に切り替える（落下を止める）
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// Y爽度を0にする
			velocity_.y = 0.0f;
		}
	}
}

void Player::CollisionWall(const CollisionMapInfo& info) {
	// 壁による速度減衰率
	if (info.hitWall_) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::Update() {
	
	Move();	

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.move_ = velocity_;

	// マップ衝突チェック
	CollisionMapTop(collisionMapInfo);
	CollisionMapBottom(collisionMapInfo);
	CollisionMapRight(collisionMapInfo);
	CollisionMapLeft(collisionMapInfo);
	
	// 判定結果を反映して移動させる
	MoveResult(collisionMapInfo);

	// 天井に接触している場合の処理
	CollisionCeiling(collisionMapInfo);

	// 接地状態の切り替え処理
	GroundTransition(collisionMapInfo);

	// 壁に接触している場合の処理
	CollisionWall(collisionMapInfo);

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
