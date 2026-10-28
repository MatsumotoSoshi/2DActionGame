#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "StageManager.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include "Skydome.h"

class StageSelectScene {
public:
	enum class Phase { kFadeIn, kMain, kFadeOut };

	void Initialize(StageManager* stageManager);
	void Update();
	void Draw();
	void DrawStageSelector();

	bool IsFinished() const { return isFinished_; }

	~StageSelectScene();

private:
	static constexpr std::size_t kSpriteCount = 5;

	StageManager* stageManager_ = nullptr;

	// 通常画像
	std::array<KamataEngine::Sprite*, kSpriteCount> stageSprites_{};

	// 選択中の画像
	std::array<KamataEngine::Sprite*, kSpriteCount> selectedStageSprites_{};

	int32_t selectedStageIndex_ = 0;
	int32_t selectableStageCount_ = 0;

	Fade* fade_ = nullptr;
	Phase phase_ = Phase::kFadeIn;
	bool isFinished_ = false;

	float fadeDuration_ = 2.0f;

	static constexpr float kScreenCenterX = 640.0f;
	static constexpr float kCardY = 270.0f;
	static constexpr float kCardWidth = 180.0f;
	static constexpr float kCardHeight = 180.0f;
	static constexpr float kCardSpacing = 220.0f;

	// 操作説明のキー画像
	KamataEngine::Sprite* spriteKeyA_ = nullptr;
	KamataEngine::Sprite* spriteKeyD_ = nullptr;
	KamataEngine::Sprite* spriteKeySpace_ = nullptr;

	KamataEngine::Model* modelSkydome_ = nullptr;
	Skydome skydome_;

	KamataEngine::Camera camera_;
};