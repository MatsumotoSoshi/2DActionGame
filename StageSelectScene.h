#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "MapChipField.h"
#include "StageManager.h"
#include <cstdint>
#include <vector>
#include "WorldMatrix.h"

class StageSelectScene {
public:
	// ステージプレビューの線
	struct PreviewLine {
		KamataEngine::Vector3 start;
		KamataEngine::Vector3 end;
		KamataEngine::Vector4 color;
	};

	// ステージプレビュー
	struct StagePreview {
		std::vector<PreviewLine> lines;
	};

	// シーンの状態
	enum class Phase { kFadeIn, kMain, kFadeOut };

	void Initialize(StageManager* stageManager);
	void Update();
	void Draw();

	bool IsFinished() const { return isFinished_; }

	~StageSelectScene();


	// ステージ選択画面を描画
	void DrawStageSelector();

private:
	StageManager* stageManager_ = nullptr;

	// ステージ選択
	int32_t selectedStageIndex_ = 0;
	float selectorFrameX_ = 0.0f;

	// ステージプレビュー
	std::vector<StagePreview> stagePreviews_;

	// ステージごとに別々のモデルを用意
	KamataEngine::Model* modelStage1_ = nullptr;
	KamataEngine::Model* modelStage2_ = nullptr;
	KamataEngine::Model* modelStage3_ = nullptr;
	KamataEngine::Model* modelStage4_ = nullptr;
	KamataEngine::Model* modelStage5_ = nullptr;

	// ステージごとのワールド変換
	KamataEngine::WorldTransform worldTransformStage1_;
	KamataEngine::WorldTransform worldTransformStage2_;
	KamataEngine::WorldTransform worldTransformStage3_;
	KamataEngine::WorldTransform worldTransformStage4_;
	KamataEngine::WorldTransform worldTransformStage5_;

	WorldMatrix worldMatrix1_;
	WorldMatrix worldMatrix2_;
	WorldMatrix worldMatrix3_;
	WorldMatrix worldMatrix4_;
	WorldMatrix worldMatrix5_;
	

	// カメラ
	KamataEngine::Camera camera_;

	// フェード
	Fade* fade_ = nullptr;
	Phase phase_ = Phase::kFadeIn;
	bool isFinished_ = false;

	// フェード時間
	float fadeDuration_ = 30.0f;

	// レイアウト
	static constexpr float kCardSpacing = 12.0f;
	static constexpr float kCardCenterY = 0.0f;
};
