#define NOMINMAX
#include "Player.h"
#include <numbers>
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include "MapChipField.h"
#include "ShieldEnemy.h"
#include "Operator.h"
#include "GameAudio.h"
using namespace KamataEngine;

void Player::Initialize(Model* model, Model* bubbleModel, const Vector3& position) {
	model_ = model;
	
	bubbleModel_ = bubbleModel;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// 目を画面手前に保ち、頭を予測線の方向へ向ける
	worldTransform_.rotation_ = {0.0f, 0.0f, aimAngle_ - std::numbers::pi_v<float> / 2.0f};
	// Draw may be called before the first Update (for example during fade-in).
	// Transfer a valid world matrix as soon as the transform is initialized.
	worldMatrix_.Update(worldTransform_);

	for (Afterimage& afterimage : afterimages_) {
		afterimage.worldTransform_.Initialize();
		afterimage.objectColor_.Initialize();
	}

	for (DashBubble& bubble : dashBubbles_) {
		bubble.worldTransform_.Initialize();
		bubble.objectColor_.Initialize();
	}

	velocity_ = {0, 0, 0}; 

}

void Player::Update() {
	isKilled_ = false;

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;

	if (isKnockBackRequested_) {
		behaviorRequest_ = Behavior::kKnockback;
		// フラグをリセット
		isKnockBackRequested_ = false;
	}

	if (behaviorRequest_ != Behavior::kUnknown) {
		// ふるまいを変更する
		behavior_ = behaviorRequest_;
		// 各振る舞いごとの初期化を行う
		switch (behavior_) {
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();

			break;

		case Behavior::kAttack:
			BehaviorAttackInitialize();

			break;

		case Behavior::kKnockback:
			BehaviorKnockBackInitialize();

			break;

		case Behavior::kStuck:
			BehaviorStuckInitialize();
			break;
		}

		// ふるまいリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
		// 通常行動
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;

		// 攻撃行動
	case Behavior::kAttack:
		BehaviorAttackUpdate(collisionMapInfo);
		break;

	case Behavior::kKnockback:
		BehaviorKnockBackUpdate();
		break;

	case Behavior::kStuck:
		BehaviorStuckUpdate();
		break;
	}

	UpdateAfterimages();
	UpdateDashBubbles();
}

void Player::Draw(const Camera& camera) {
	if (model_ == nullptr) {
		return;
	}

	model_->Draw(worldTransform_, camera);
}

void Player::DrawTrajectory() const {
	if (!mapChipField_ || behavior_ != Behavior::kRoot) {
		return;
	}

	const Vector3 start = worldTransform_.translation_;
	const float directionX = std::cos(aimAngle_);
	const float directionY = std::sin(aimAngle_);
	Vector3 end = start;
	constexpr float sampleStep = 0.1f;

	for (float distance = sampleStep; distance <= kAimPreviewDistance; distance += sampleStep) {
		const Vector3 sample = {start.x + directionX * distance, start.y + directionY * distance, start.z};
		bool hitBlock = false;
		for (float xSign : {-1.0f, 1.0f}) {
			for (float ySign : {-1.0f, 1.0f}) {
				const Vector3 corner = {sample.x + xSign * kWallWidthHalf, sample.y + ySign * kWallHeightHalf, sample.z};
				const MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(corner);
				if (IsSolidMapChipType(mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex))) {
					hitBlock = true;
					break;
				}
			}
			if (hitBlock) {
				break;
			}
		}

		if (hitBlock) {
			break;
		}
		end = sample;
	}

	PrimitiveDrawer::GetInstance()->DrawLine3d(start, end, {0.2f, 0.8f, 1.0f, 0.9f});
}

void Player::Move(float rotationSpeed) {
	const bool turnLeft = Input::GetInstance()->PushKey(DIK_A);
	const bool turnRight = Input::GetInstance()->PushKey(DIK_D);

	if (turnLeft != turnRight) {
		const float turnDirection = turnLeft ? 1.0f : -1.0f;
		aimAngle_ += turnDirection * rotationSpeed / 60.0f;
	}

	// 回転を進行方向に反映
	worldTransform_.rotation_ = {0.0f, 0.0f, aimAngle_ - std::numbers::pi_v<float> / 2.0f};

	lrDirection_ = std::cos(aimAngle_) >= 0.0f ? LRDirection::kRight : LRDirection::kLeft;

	velocity_ = {};
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	const Vector3 offsetTable[kNumCorner] = {
	    {+kWallWidthHalf, -kWallHeightHalf, 0.0f},
        {-kWallWidthHalf, -kWallHeightHalf, 0.0f},
        {+kWallWidthHalf, +kWallHeightHalf, 0.0f},
        {-kWallWidthHalf, +kWallHeightHalf, 0.0f}
    };

	return center + offsetTable[static_cast<uint32_t>(corner)];
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

Vector3 Player::GetTipPosition() const {
	return {worldTransform_.translation_.x + std::cos(aimAngle_) * 0.62f,
	        worldTransform_.translation_.y + std::sin(aimAngle_) * 0.62f,
	        worldTransform_.translation_.z};
}

void Player::OnCollision(const WalkEnemy* enemy) { 
	if (IsAttack() && enemy != nullptr) {
		isKilled_ = true;
		GearUp();
	}
}

void Player::OnCollision(const ShieldEnemy* enemy) {
	if (IsAttack() && enemy != nullptr) {
		if (lrDirection_ == enemy->GetDirection()) {
			isKilled_ = true;
			GearUp();
		}
	}
}

void Player::OnEnemyTipCollision() {
	isDead_ = true;
	velocity_ = {0.0f, 0.0f, 0.0f};
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
	if (IsSolidMapChipType(mapChipType) && !IsSolidMapChipType(mapChipTypeNext)) {
		hit = true;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (IsSolidMapChipType(mapChipType) && !IsSolidMapChipType(mapChipTypeNext)) {
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
			float yMove = (rect.bottom - worldTransform_.translation_.y) - (kWallHeightHalf + kBlank);
			info.move_.y = std::max(0.0f, yMove);

			// 天井に当たったことを記録する
			info.hitTop_ = true;
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
	if (IsSolidMapChipType(mapChipType) && !IsSolidMapChipType(mapChipTypeNext)) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (IsSolidMapChipType(mapChipType) && !IsSolidMapChipType(mapChipTypeNext)) {
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
			float yMove = (rect.top - worldTransform_.translation_.y) + (kWallHeightHalf + kBlank);
			info.move_.y = std::min(0.0f, yMove);

			// 地面に当たったことを記録する
			info.hitBottom_ = true;
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
	if (IsSolidMapChipType(mapChipType) && !IsSolidMapChipType(mapChipTypeNext)) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (IsSolidMapChipType(mapChipType) && !IsSolidMapChipType(mapChipTypeNext)) {
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
			float xMove = (rect.left - worldTransform_.translation_.x) - (kWallWidthHalf + kBlank);
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
	if (IsSolidMapChipType(mapChipType) && !IsSolidMapChipType(mapChipTypeNext)) {
		hit = true;
	}
		
	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (IsSolidMapChipType(mapChipType) && !IsSolidMapChipType(mapChipTypeNext)) {
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
			float xMove = (rect.right - worldTransform_.translation_.x) + (kWallWidthHalf + kBlank);
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

void Player::BehaviorRootInitialize() { 
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_ = {0.0f, 0.0f, aimAngle_ - std::numbers::pi_v<float> / 2.0f};
}

void Player::BehaviorAttackInitialize() {
	// ダッシュ状態に入った瞬間だけ鳴らす
	GameAudio::GetInstance().PlayDash();

	currentDashSpeed_ = kDashInitialSpeed;

	const Vector3 direction = {std::cos(aimAngle_), std::sin(aimAngle_), 0.0f};

	velocity_ = {direction.x * currentDashSpeed_, direction.y * currentDashSpeed_, 0.0f};
}

void Player::BehaviorKnockBackInitialize() {
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	velocity_ = {-std::cos(aimAngle_) * 0.5f, -std::sin(aimAngle_) * 0.5f, 0.0f};
}

void Player::BehaviorRootUpdate() {
	if (!isDead_) {
		Move(kAimRotationSpeed);
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			behaviorRequest_ = Behavior::kAttack;
		}
	}

	worldMatrix_.Update(worldTransform_);
}

void Player::BehaviorAttackUpdate(CollisionMapInfo& info) {
	wallHitThisFrame_ = false;
	Move(kDashRotationSpeed);
	const Vector3 direction = {std::cos(aimAngle_), std::sin(aimAngle_), 0.0f};
	const float acceleration = kDashAcceleration + gearLevel_ * kGearAccelerationBonus;
	currentDashSpeed_ = std::min(currentDashSpeed_ + acceleration, kDashMaxSpeed);
	velocity_ = {direction.x * currentDashSpeed_, direction.y * currentDashSpeed_, 0.0f};
	worldTransform_.scale_ = {1.0f, 1.15f, 1.0f};

	info.move_ = velocity_;

	// マップ衝突チェック
	CollisionMapTop(info);
	CollisionMapBottom(info);
	CollisionMapRight(info);
	CollisionMapLeft(info);

	// 判定結果を反映して移動させる
	MoveResult(info);

	if (info.hitTop_ || info.hitBottom_ || info.hitWall_) {
		wallHitThisFrame_ = true;
		velocity_ = {};
		currentDashSpeed_ = 0.0f;
		behaviorRequest_ = Behavior::kStuck;
	}

	// ワールド行列を更新
	worldMatrix_.Update(worldTransform_);
}

bool Player::ConsumeWallHit() {
	const bool hit = wallHitThisFrame_;
	wallHitThisFrame_ = false;
	return hit;
}

void Player::BehaviorKnockBackUpdate() {
	// 地面状態や重力に依存せず、空中で減衰する。
	velocity_.x *= 0.9f;
	velocity_.y *= 0.9f;

	// 衝突判定処理
	CollisionMapInfo info;
	info.move_ = velocity_;

	CollisionMapTop(info);
	CollisionMapBottom(info);
	CollisionMapRight(info);
	CollisionMapLeft(info);

	MoveResult(info);
	if (info.hitTop_) {
		velocity_.y = std::min(velocity_.y, 0.0f);
	}

	if (info.hitBottom_) {
		velocity_.y = std::max(velocity_.y, 0.0f);
	}

	if (info.hitWall_) {
		velocity_.x = 0.0f;
	}

	// 止まったら終了
	if (std::abs(velocity_.x) < 0.05f && std::abs(velocity_.y) < 0.05f) {
		behaviorRequest_ = Behavior::kRoot;
	}

	worldMatrix_.Update(worldTransform_);
}

void Player::BehaviorStuckInitialize() {
	stuckTimer_ = 0;
	velocity_ = {};

	worldTransform_.rotation_ = {0.0f, 0.0f, aimAngle_ - std::numbers::pi_v<float> / 2.0f};

	worldMatrix_.Update(worldTransform_);
}

void Player::BehaviorStuckUpdate() {
	++stuckTimer_;

	const float wobble = std::sin(static_cast<float>(stuckTimer_) * 0.35f) * 0.04f;

	// 画面内で小さく揺らす。目は手前を向いたまま。
	worldTransform_.rotation_ = {0.0f, 0.0f, aimAngle_ - std::numbers::pi_v<float> / 2.0f + wobble};

	worldMatrix_.Update(worldTransform_);

	if (stuckTimer_ >= kWallStuckDuration) {
		behaviorRequest_ = Behavior::kRoot;
	}
}


void Player::UpdateAfterimages() {
	// 残像の経過を進める
	for (Afterimage& afterimage : afterimages_) {
		if (!afterimage.active_) {
			continue;
		}

		++afterimage.age_;
		if (afterimage.age_ >= kAfterimageLifetime_) {
			afterimage.active_ = false;
			continue;
		}

		afterimage.worldTransform_.translation_ = afterimage.origin_;
		afterimage.worldTransform_.translation_.x += afterimage.trailDirection_.x * 0.04f * static_cast<float>(afterimage.age_);
		afterimage.worldTransform_.translation_.y += afterimage.trailDirection_.y * 0.04f * static_cast<float>(afterimage.age_);
		worldMatrix_.Update(afterimage.worldTransform_);
	}

	// フレームカウント初期化
	if (!IsAttack()) {
		afterimageFrameCounter_ = 0;
		return;
	}

	++afterimageFrameCounter_;
	if (afterimageFrameCounter_ < kAfterimageInterval_) {
		return;
	}

	afterimageFrameCounter_ = 0;

	Afterimage& afterimage = afterimages_[nextAfterimageIndex_];
	nextAfterimageIndex_ = (nextAfterimageIndex_ + 1) % kAfterimageCount_;
	afterimage.worldTransform_.scale_ = worldTransform_.scale_;
	afterimage.worldTransform_.rotation_ = worldTransform_.rotation_;
	afterimage.origin_ = worldTransform_.translation_;
	afterimage.trailDirection_ = {-std::cos(aimAngle_), -std::sin(aimAngle_), 0.0f};
	afterimage.age_ = 0;
	afterimage.active_ = true;

	worldMatrix_.Update(afterimage.worldTransform_);
}

void Player::DrawAfterimages(const Camera& camera) {
	// 残像を描画
	for (Afterimage& afterimage : afterimages_) {
		if (!afterimage.active_) {
			continue;
		}

		float fade = 1.0f - static_cast<float>(afterimage.age_) / kAfterimageLifetime_;
		afterimage.objectColor_.SetColor({0.25f, 0.6f, 1.0f, 0.55f * fade});
		model_->Draw(afterimage.worldTransform_, camera, &afterimage.objectColor_);
	}

	if (bubbleModel_ == nullptr) {
		return;
	}

	for (DashBubble& bubble : dashBubbles_) {
		if (!bubble.active_) {
			continue;
		}

		const float fade = 1.0f - static_cast<float>(bubble.age_) / kDashBubbleLifetime_;
		bubble.objectColor_.SetColor({0.68f, 0.92f, 1.0f, 0.72f * fade});
		bubbleModel_->Draw(bubble.worldTransform_, camera, &bubble.objectColor_);
	}
}

void Player::UpdateDashBubbles() {
	for (DashBubble& bubble : dashBubbles_) {
		if (!bubble.active_) {
			continue;
		}

		++bubble.age_;

		if (bubble.age_ >= kDashBubbleLifetime_) {
			bubble.active_ = false;
			continue;
		}

		const float age = static_cast<float>(bubble.age_);
		bubble.worldTransform_.translation_ = bubble.origin_;
		bubble.worldTransform_.translation_.y += age * 0.008f + std::sin(age * 0.35f + bubble.phase_) * 0.025f;
		const float size = bubble.size_ * (1.0f + age * 0.025f);
		bubble.worldTransform_.scale_ = {size, size, size};
		worldMatrix_.Update(bubble.worldTransform_);
	}

	if (!IsAttack()) {
		dashBubbleFrameCounter_ = 0;
		return;
	}

	if (++dashBubbleFrameCounter_ < kDashBubbleInterval_) {
		return;
	}

	dashBubbleFrameCounter_ = 0;

	const Vector3 direction = {std::cos(aimAngle_), std::sin(aimAngle_), 0.0f};
	const Vector3 perpendicular = {-direction.y, direction.x, 0.0f};
	DashBubble& bubble = dashBubbles_[nextDashBubbleIndex_];
	nextDashBubbleIndex_ = (nextDashBubbleIndex_ + 1) % kDashBubbleCount_;
	const float lateralOffset = (static_cast<float>(nextDashBubbleIndex_ % 3) - 1.0f) * 0.12f;
	bubble.origin_ = {
	    worldTransform_.translation_.x - direction.x * 0.55f + perpendicular.x * lateralOffset,
	    worldTransform_.translation_.y - direction.y * 0.55f + perpendicular.y * lateralOffset,
	    worldTransform_.translation_.z};
	bubble.size_ = 0.07f + static_cast<float>(nextDashBubbleIndex_ % 3) * 0.025f;
	bubble.phase_ = static_cast<float>(nextDashBubbleIndex_) * 0.83f;
	bubble.age_ = 0;
	bubble.active_ = true;
	bubble.worldTransform_.translation_ = bubble.origin_;
	bubble.worldTransform_.scale_ = {bubble.size_, bubble.size_, bubble.size_};
	worldMatrix_.Update(bubble.worldTransform_);
}

