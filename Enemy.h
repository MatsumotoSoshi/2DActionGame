#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"
#include "AABB.h"

class Player;

/// <summary>
/// 敵
/// </summary>
class Enemy {
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

	// 衝突反応
	void onCollision(const Player* player);

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;
	WorldMatrix worldMatrix_;
};
