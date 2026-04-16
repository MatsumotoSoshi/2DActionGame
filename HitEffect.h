#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"
#include <array>

/// <summary>
/// ヒット演出用エフェクト
/// </summary>
class HitEffect {
public:
	/// <summary>
	///
	/// </summary>
	void Initialize(const KamataEngine::WorldTransform& circleWorldTransform);

	/// <summary>
	///
	/// </summary>
	void Update(); 

	/// <summary>
	///
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);

	static void SetModel(KamataEngine::Model* model) { model_ = model; }
	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

	// インスタンス生成と初期化
	static HitEffect* Create(const KamataEngine::WorldTransform& circleWorldTransform);

	static inline const uint32_t number = 2u;

	// 楕円のワールドトランスフォーム
	std::array<KamataEngine::WorldTransform, number> ellipseWorldTransform_;

	enum class State {
		kSpread,    // スプレッド
		kFade,      // フェード
		kDeath      // デス状態
	};

	State state_ = State::kSpread;

	uint32_t counter_ = 0;

	// 色変換オブジェクト
	KamataEngine::ObjectColor objectColor_;
	// 色数値
	KamataEngine::Vector4 color_;

	// デスフラグの取得
	bool IsDead() const { return state_ == State::kDeath; }

	static inline const uint32_t kSpreadDuration = 20;
	static inline const uint32_t kFadeDuration = 20;

private:
	// モデル（借りてくる用）
	static KamataEngine::Model* model_;
    // カメラ（借りてくる用）
	static KamataEngine::Camera* camera_;

	// 円のワールドトランスフォーム
	KamataEngine::WorldTransform circleWorldTransform_;

	WorldMatrix worldMatrix_;
};
