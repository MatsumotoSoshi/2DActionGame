#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "WorldMatrix.h"
#include "MapChipField.h"
#include "Trajectory.h"
#include <vector>
#include <cstdint>
#include "Skydome.h"

class StageManager;

class ClearScene {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const std::vector<KamataEngine::Vector3>& path, StageManager* stageManager, uint32_t wallHits, float totalDistance);

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

	enum class ReturnTarget { 
		kTitle, 
		kStageSelect
	};

	ReturnTarget GetReturnTarget() const { return returnTarget_; }

private:
	// 3Dモデル
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

	// 選択中の項目
	int selectedMenuIndex_ = 0;

	// 決定した戻り先
	ReturnTarget returnTarget_ = ReturnTarget::kStageSelect;

	bool isSelected_ = false;

	void DrawClearScore();

	int clearScore_ = 0;

	enum ClearRank {
		A,
		B,
		C,
	};

	ClearRank clearRank_ = ClearRank::C;
	
	// 評価
	KamataEngine::Sprite* spriteRank_ = nullptr;
	KamataEngine::Sprite* spriteRankComment_ = nullptr;

	// タイトルへ：通常・選択中
	KamataEngine::Sprite* spriteReturnTitle_ = nullptr;
	KamataEngine::Sprite* spriteReturnTitleSelected_ = nullptr;

	// ステージ選択へ：通常・選択中
	KamataEngine::Sprite* spriteReturnStageSelect_ = nullptr;
	KamataEngine::Sprite* spriteReturnStageSelectSelected_ = nullptr;

	KamataEngine::Model* modelSkydome_ = nullptr;
	Skydome skydome_;
};
