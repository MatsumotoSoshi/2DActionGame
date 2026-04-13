#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"
#include "Fade.h"

class TitleScene 
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

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

    /// <summary>
    /// 
    /// </summary>
	~TitleScene();

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
	KamataEngine::Model* modelPlayer_ = nullptr;
	// ワールド変換
	KamataEngine::WorldTransform worldTransformFont_;
	KamataEngine::WorldTransform worldTransformPlayer_;
	// カメラ
	KamataEngine::Camera camera_;
	
	WorldMatrix worldMatrix_;
	Fade* fade_ = nullptr;
};
