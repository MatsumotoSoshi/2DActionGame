#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "WorldMatrix.h"
#include "MapChipField.h"
#include "Trajectory.h"
#include <vector>

class StageManager;

class ClearScene {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const std::vector<KamataEngine::Vector3>& path, StageManager* stageManager);

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
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;
	KamataEngine::Model* modelShieldEnemy_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	std::vector<KamataEngine::WorldTransform*> stageBlocks_;
	std::vector<KamataEngine::WorldTransform*> normalTargets_;
	std::vector<KamataEngine::WorldTransform*> shieldTargets_;
	KamataEngine::WorldTransform goalTransform_;
	KamataEngine::ObjectColor goalColor_;
	Trajectory trajectory_;

	// ワールド変換
	KamataEngine::WorldTransform worldTransformFont_;
	// カメラ
	KamataEngine::Camera camera_;

	WorldMatrix worldMatrix_;
	Fade* fade_ = nullptr;
};
