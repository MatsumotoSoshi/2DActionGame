#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"
#include "AABB.h"

class MapChipField;
class Enemy;

class Player {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 移動
	/// </summary>
	void Move();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);


	KamataEngine::Vector3 velocity_ = {};
	static inline const float kAcceleration = 0.05f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kLimitRunSpeed = 0.8f;

	enum class LRDirection { kRight, kLeft };

	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;

	// 旋回時間
	static inline const float kTimeTurn = 0.3f;

	// 接地状態フラグ
	bool onGround_ = true;

	// 重力加速度
	static inline const float kGravityAcceleration_ = 0.05f;
	// 最大落下速度
	static inline const float kLimitFallSpeed_ = 2.0f;
	// ジャンプ初速
	static inline const float kJumpAcceleration_ = 0.65f;

	bool landing_ = false;

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	//キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	struct CollisionMapInfo {
		bool hitCeiling_ = false;    // 天井衝突フラグ
		bool hitGround_ = false;     // 着地フラグ
		bool hitWall_ = false;       // 壁衝突フラグ
		KamataEngine::Vector3 move_;               // 移動量
	};

	// マップとの衝突判定
	void CollisionMapTop(CollisionMapInfo& collisionMapInfo);
	void CollisionMapBottom(CollisionMapInfo& collisionMapInfo);
	void CollisionMapRight(CollisionMapInfo& collisionMapInfo);
	void CollisionMapLeft(CollisionMapInfo& collisionMapInfo);


	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,      // 左上

		kNumCorner    // 要素数
	};

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	static inline const float kWidthHalf = kWidth / 2.0f;
	static inline const float kHeightHalf = kHeight / 2.0f;
	static inline const float kBlank = 0.001f;

	// 判定結果を反映して移動させる
	void MoveResult(const CollisionMapInfo& collisionMapInfo);

	// // 天井に接触している場合の処理
	void CollisionCeiling(const CollisionMapInfo& info);

	// 接地状態の切り替え処理
	void GroundTransition(const CollisionMapInfo& info); 

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.1f;

	static inline const float epsilon = 0.001f;

	// 壁に接触している場合の処理 
	void CollisionWall(const CollisionMapInfo& info);

	// 壁接触時の速度減衰率

	static inline const float kAttenuationWall = 0.9f;

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	AABB GetAABB();

	// 衝突反応
	void onCollision(const Enemy* enemy);

	// デスフラグ
	bool isDead_ = false;

	// デスフラグのゲッター
	bool IsDead() const { return isDead_; }
	
	// 通常行動更新
	void BehaviorRootUpdate();

	// 攻撃行動更新
	void BehaviorAttackUpdate(CollisionMapInfo& info);

	enum class Behavior { 
		kRoot,        // 通常状態
		kAttack,      // 攻撃中
		kUnknown      // 不明
	};

	// ふるまい
	Behavior behavior_ = Behavior::kRoot;

	// ふるまいリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 通常行動初期化
	void BehaviorRootInitialize();

	// 攻撃行動初期化
	void BehaviorAttackInitialize();

	// 攻撃ギミックの経過時間カウンター
	uint32_t attackParameter_ = 0;

	// 攻撃時間
	static inline const uint32_t kTimeCharge = 5;
	static inline const uint32_t kTimeRush = 10;
	static inline const uint32_t kTimeRear = 5;

	enum class AttackPhase {
		kCharge ,      // 溜め
		kRush,         // 突進
		kRear,         // 余韻
	};

	AttackPhase attackPhase_ = AttackPhase::kCharge;

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	WorldMatrix worldMatrix_;
	MapChipField* mapChipField_ = nullptr;
};
