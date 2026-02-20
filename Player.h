#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"

class Player {
public:
	/// <summary>
	///
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 
	/// </summary>
	void Update();

	/// <summary>
	/// 
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);

	KamataEngine::Vector3 velocity_ = {};
	static inline const float kAcceleration = 0.1f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kLimitRunSpeed = 1.0f;

	enum class LRDirection { 
		kRight,
		kLeft
	};

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
	static inline const float kJumpAcceleration_ = 0.8f;

	bool landing_ = false;

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	WorldMatrix worldMatrix_;
};
