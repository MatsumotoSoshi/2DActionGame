#pragma once
#include "KamataEngine.h"
#include <array>
#include "WorldMatrix.h"
#include <numbers>

class Player;
class GameScene;

class DeathParticles 
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);

	// パーティクルの数
	static inline const uint32_t kNumParticles = 8;

	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;

	// 存続時間
	static inline const float kDuration = 1.5f; 
	// 移動の速さ
	static inline const float kSpeed = 0.07f;
	// 分割した一個分の角度
	static inline const float kAngleUnit = (std::numbers::pi_v<float> * 2.0f) / static_cast<float>(kNumParticles);

	// 終了フラグ
	bool isFinished_ = false;
	// 経過時間カウント
	float counter_ = 0.0f;

	// 色変換オブジェクト
	KamataEngine::ObjectColor objectColor_;
	// 色数値
	KamataEngine::Vector4 color_;

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	WorldMatrix worldMatrix_;
};
