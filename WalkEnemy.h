#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"
#include "AABB.h"
#include "player.h"
#include "GameScene.h"

class Player;
class GameScene;
class MapChipField;

/// <summary>
/// 敵
/// </summary>
class WalkEnemy {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.01f;
	static inline const float kDashSpeed = 0.22f;
	static inline const float kDetectionRange = 7.5f;
	static inline const float kDetectionResetRange = 9.5f;
	static inline const float kHomingTurnRate = 0.01f;
	static inline const uint32_t kWallStuckDuration = 120;
	static inline const uint32_t kSearchDuration = 180;
	static inline const float kTipOffset = 0.62f;

	// 速度
	KamataEngine::Vector3 velocity_;

	// 最初の角度
	static inline const float kWalkMotionAngleStart = -20.0f;

	// 最後の角度
	static inline const float kWalkMotionAngleEnd = 50.0f;

	// アニメーションの周期となる時間
	static inline const float kWalkMotionCycle = 1.0f;

	// 経過時間
	float walkTimmer_ = 0.0f;

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	static inline const float kWidthHalf = kWidth / 2.0f;
	static inline const float kHeightHalf = kHeight / 2.0f;

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	AABB GetAABB();
	KamataEngine::Vector3 GetTipPosition() const;
	bool IsDashing() const;

	// 衝突反応
	void onCollision(const Player* player);

	// デスフラグ
	bool isDead_ = false;

	// デスフラグのゲッター
	bool IsDead() const { return isDead_; }

	enum class Behavior {
		kWalk,         // 歩行
		kDash,         // 突進
		kStuck,        // 壁に刺さった状態
		kSearch,       // 刺さった後の索敵
	    kDeath,        // デス演出
	};

	// ふるまい
	Behavior behavior_ = Behavior::kWalk;

	// ふるまいリクエスト
	Behavior behaviorRequest_ = Behavior::kWalk;

	// アニメーションタイマー
	uint32_t animationTimer_ = 0;

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }
	void SetTarget(Player* target) { target_ = target; }
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }


	bool isCollisionDisabled_ = false;

	bool IsCollisionDisabled() const {
		return isCollisionDisabled_ || behavior_ == Behavior::kDeath || behaviorRequest_ == Behavior::kDeath;
	}


private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	WorldMatrix worldMatrix_;

	GameScene* gameScene_ = nullptr;
	Player* target_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	float aimAngle_ = 0.0f;
	uint32_t stuckTimer_ = 0;
	uint32_t searchTimer_ = 0;
	bool canCharge_ = true;
	bool IsBlockedAt(const KamataEngine::Vector3& position) const;
};
