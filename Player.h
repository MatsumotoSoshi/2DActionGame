#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"
#include "AABB.h"
#include "Direction.h"
#include <array>

class MapChipField;
class WalkEnemy;
class ShieldEnemy;

class Player {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* bubbleModel, const KamataEngine::Vector3& position);

	/// <summary>
	/// 移動
	/// </summary>
	void Move(float rotationSpeed);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);
	void DrawAfterimages(const KamataEngine::Camera& camera);
	void DrawTrajectory() const;


	KamataEngine::Vector3 velocity_ = {};
	// ダッシュ前の方向決めの回転速度
	static inline const float kAimRotationSpeed = 3.0f;
	// ダッシュ中の回転速度
	static inline const float kDashRotationSpeed = 7.0f;
	static inline const float kDashInitialSpeed = 0.005f;
	static inline const float kDashAcceleration = 0.0018f;
	static inline const float kGearAccelerationBonus = 0.0008f;
	static inline const float kDashMaxSpeed = 0.15f;
	static inline const uint32_t kMaxGearLevel = 5;
	static inline const float kAimPreviewDistance = 15.0f;
	float aimAngle_ = 0.0f;

	LRDirection lrDirection_ = LRDirection::kRight;

	LRDirection GetDirection() const { return lrDirection_; }

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }
	KamataEngine::Vector3 GetTipPosition() const;

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }
	float GetDashSpeed() const { return currentDashSpeed_; }
	bool ConsumeWallHit();

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	//キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;


	struct CollisionMapInfo {
		bool hitTop_ = false;        // 上側ブロックとの衝突
		bool hitBottom_ = false;     // 下側ブロックとの衝突
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

	// 壁判定だけ、上下左右に0.04ずつ広げる
	static constexpr float kWallCollisionMargin = 0.04f;

	static inline const float kWallWidthHalf = kWidthHalf + kWallCollisionMargin;

	static inline const float kWallHeightHalf = kHeightHalf + kWallCollisionMargin;

	// 判定結果を反映して移動させる
	void MoveResult(const CollisionMapInfo& collisionMapInfo);

	static inline const float epsilon = 0.001f;

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	AABB GetAABB();

	// 衝突反応
	void OnCollision(const WalkEnemy* enemy);
	void OnCollision(const ShieldEnemy* enemy);
	void OnEnemyTipCollision();

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
		kKnockback,   // ノックバック
		kStuck,       // 壁に刺さって硬直
		kUnknown      // 不明
	};

	Behavior GetBehavior() const { return behavior_; }

	// ふるまい
	Behavior behavior_ = Behavior::kRoot;

	// ふるまいリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 通常行動初期化
	void BehaviorRootInitialize();

	// 攻撃行動初期化
	void BehaviorAttackInitialize();

	// 攻撃時間
	static inline const uint32_t kWallStuckDuration = 30;

	bool IsAttack() const { return behavior_ == Behavior::kAttack; }
	bool IsDashing() const { return behavior_ == Behavior::kAttack && behaviorRequest_ != Behavior::kStuck; }
	void GearUp() { if (gearLevel_ < kMaxGearLevel) { ++gearLevel_; } }

	// ノックバックリクエストフラグ
	bool isKnockBackRequested_ = false;
	void RequestKnockBack() { isKnockBackRequested_ = true; }

	// ノックバック初期化
	void BehaviorKnockBackInitialize();
	// ノックバック更新
	void BehaviorKnockBackUpdate();
	void BehaviorStuckInitialize();
	void BehaviorStuckUpdate();
	uint32_t stuckTimer_ = 0;

	// 敵討伐フラグ
	bool isKilled_ = false;
	// 敵討伐ゲッター
	bool IsKill() const { return isKilled_; }

	
	// ヒットストップフラグ
	bool isHitStoped_ = false;
	bool IsHitStop() const { return isHitStoped_; }

	void UpdateAfterimages();
	void UpdateDashBubbles();

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* bubbleModel_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	WorldMatrix worldMatrix_;
	MapChipField* mapChipField_ = nullptr;
	float currentDashSpeed_ = 0.0f;
	uint32_t gearLevel_ = 0;
	bool wallHitThisFrame_ = false;

	struct Afterimage {
		KamataEngine::WorldTransform worldTransform_;
		KamataEngine::ObjectColor objectColor_;
		KamataEngine::Vector3 origin_{};
		KamataEngine::Vector3 trailDirection_{};
		uint32_t age_ = 0;
		bool active_ = false;
	};

	static inline const uint32_t kAfterimageCount_ = 4;       // 残像数
	static inline const uint32_t kAfterimageInterval_ = 2;    // 残像生成の間隔
	static inline const uint32_t kAfterimageLifetime_ = 10;   // 残像生存時間
	std::array<Afterimage, kAfterimageCount_> afterimages_;
	uint32_t afterimageFrameCounter_ = 0;
	uint32_t nextAfterimageIndex_ = 0;

	struct DashBubble {
		KamataEngine::WorldTransform worldTransform_;
		KamataEngine::ObjectColor objectColor_;
		KamataEngine::Vector3 origin_{};
		float size_ = 0.1f;
		float phase_ = 0.0f;
		uint32_t age_ = 0;
		bool active_ = false;
	};

	static inline const uint32_t kDashBubbleCount_ = 16;
	static inline const uint32_t kDashBubbleInterval_ = 2;
	static inline const uint32_t kDashBubbleLifetime_ = 18;
	std::array<DashBubble, kDashBubbleCount_> dashBubbles_;
	uint32_t dashBubbleFrameCounter_ = 0;
	uint32_t nextDashBubbleIndex_ = 0;

};
