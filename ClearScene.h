#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "WorldMatrix.h"
class ClearScene {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	///
	/// </summary>
	~ClearScene();

	// 終了フラグ
	bool isFinished_ = false;

	// デスフラグのゲッター
	bool IsFinished() const { return isFinished_; }

	// フェード時間
	float fadeDuration_ = 2.0f;

	enum class Phase {
		kFadeIn, // フェードイン
		kMain,   // メイン
		kFadeOut // フェードアウト
	};

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

private:
	// 3Dモデル
	KamataEngine::Model* modelFont_ = nullptr;

	// ワールド変換
	KamataEngine::WorldTransform worldTransformFont_;
	// カメラ
	KamataEngine::Camera camera_;

	WorldMatrix worldMatrix_;
	Fade* fade_ = nullptr;
};
