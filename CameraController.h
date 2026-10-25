#pragma once
#include "KamataEngine.h" 
#include "Player.h" 


class CameraController 
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void SetTarget(Player* target) { target_ = target; };
	void SetHitStopFocus(const KamataEngine::Vector3& position, bool enabled) {
		hitStopFocusPosition_ = position;
		isHitStopFocusEnabled_ = enabled;
	}
	void SetDashShake(bool enabled) { isDashShakeEnabled_ = enabled; }
	void Reset();

	KamataEngine::Camera& GetCamera() { return camera_; }

	struct Rect {
		float left = 0.0f;      // 左端
		float right = 1.0f;     // 右端
		float bottom = 0.0f;    // 下端
		float top = 1.0f;       // 上端
	};

	void SetMovableArea(Rect area) { movableArea_ = area; }

	static inline const float kInterpolationRate = 0.3f;

	static inline const float kVelocityBias = 5.0f;

	static inline const Rect margin_ = { -8.0f, 8.0f, -4.0f, 4.0f };

private:
	KamataEngine::Camera camera_;

	Player* target_ = nullptr;
	KamataEngine::Vector3 targetOffset_ = {0, 0, -15.0f};

	// プレイヤー攻撃時のカメラシェイク
	KamataEngine::Vector3 cameraShakeOffset_ = {0, 0, 0};

	// カメラシェイクフラグ
	bool cameraShakeKill_ = false;
	bool isDashShakeEnabled_ = false;
	float dashShakeTime_ = 0.0f;

	Rect movableArea_ = {0, 100, 0, 100};
	// 目標座標
	KamataEngine::Vector3 targetCoordinates_;
	KamataEngine::Vector3 hitStopFocusPosition_;
	bool isHitStopFocusEnabled_ = false;

};


