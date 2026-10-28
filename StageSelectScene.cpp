#define NOMINMAX
#include "StageSelectScene.h"
#include <algorithm>
#include <cassert>
#include "GameAudio.h"
using namespace KamataEngine;

void StageSelectScene::Initialize(StageManager* stageManager) {
	assert(stageManager != nullptr);

	// 再初期化前に以前のスプライトを解放
	for (Sprite*& sprite : stageSprites_) {
		delete sprite;
		sprite = nullptr;
	}

	for (Sprite*& sprite : selectedStageSprites_) {
		delete sprite;
		sprite = nullptr;
	}

	delete fade_;
	fade_ = nullptr;

	stageManager_ = stageManager;
	selectedStageIndex_ = 0;
	selectableStageCount_ = 0;

	phase_ = Phase::kFadeIn;
	isFinished_ = false;

	if (stageManager_ == nullptr) {
		return;
	}

	selectableStageCount_ = std::clamp(stageManager_->GetStageCount(), 0, static_cast<int32_t>(kSpriteCount));

	if (selectableStageCount_ > 0) {
		selectedStageIndex_ = std::clamp(stageManager_->GetCurrentStageIndex(), 0, selectableStageCount_ - 1);
	}

	const std::array<const char*, kSpriteCount> texturePaths = {"stageSelect/stage1.png", "stageSelect/stage2.png", "stageSelect/stage3.png", "stageSelect/stage4.png", "stageSelect/stage5.png"};

	const std::array<const char*, kSpriteCount> selectedTexturePaths = {
	    "stageSelect/stage1Selected.png", "stageSelect/stage2Selected.png", "stageSelect/stage3Selected.png", "stageSelect/stage4Selected.png", "stageSelect/stage5Selected.png"};

	const float firstCenterX = kScreenCenterX - static_cast<float>(selectableStageCount_ > 0 ? selectableStageCount_ - 1 : 0) * kCardSpacing * 0.5f;

	for (int32_t i = 0; i < selectableStageCount_; ++i) {
		const std::size_t index = static_cast<std::size_t>(i);

		const float centerX = firstCenterX + static_cast<float>(i) * kCardSpacing;

		const Vector2 position = {centerX - kCardWidth * 0.5f, kCardY};

		// 通常画像
		stageSprites_[index] = Sprite::Create(TextureManager::Load(texturePaths[index]), position);

		stageSprites_[index]->SetSize({kCardWidth, kCardHeight});

		// 選択中の画像も同じ位置・サイズで用意
		selectedStageSprites_[index] = Sprite::Create(TextureManager::Load(selectedTexturePaths[index]), position);

		selectedStageSprites_[index]->SetSize({kCardWidth, kCardHeight});
	}

	// 背景用のカメラ
	camera_.Initialize();
	camera_.farZ = 5000.0f;
	camera_.UpdateMatrix();

	modelSkydome_ = Model::CreateFromOBJ("SkyDome");

	skydome_.Initialize(modelSkydome_, &camera_);
	skydome_.Update();

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeDuration_);

	// 再初期化時は以前の操作UIを解放
	delete spriteKeyA_;
	spriteKeyA_ = nullptr;

	delete spriteKeyD_;
	spriteKeyD_ = nullptr;

	delete spriteKeySpace_;
	spriteKeySpace_ = nullptr;

	// Aキー
	spriteKeyA_ = Sprite::Create(TextureManager::Load("ui/keys/keyA.png"), {440.0f, 580.0f});

	spriteKeyA_->SetSize({64.0f, 64.0f});

	// Dキー
	spriteKeyD_ = Sprite::Create(TextureManager::Load("ui/keys/keyD.png"), {520.0f, 580.0f});

	spriteKeyD_->SetSize({64.0f, 64.0f});

	// SPACEキー
	spriteKeySpace_ = Sprite::Create(TextureManager::Load("ui/keys/keySPACE.png"), {680.0f, 580.0f});

	spriteKeySpace_->SetSize({160.0f, 64.0f});

	GameAudio::GetInstance().PlayBgm(GameAudio::Bgm::kStageSelect);
}

void StageSelectScene::Update() {
	if (stageManager_ == nullptr || fade_ == nullptr) {
		return;
	}

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();

		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;

	case Phase::kMain:
		if (selectableStageCount_ <= 0) {
			break;
		}

		// Aキー：左へ
		if (Input::GetInstance()->TriggerKey(DIK_A)) {
			selectedStageIndex_ = std::max(0, selectedStageIndex_ - 1);
		}

		// Dキー：右へ
		if (Input::GetInstance()->TriggerKey(DIK_D)) {
			selectedStageIndex_ = std::min(selectableStageCount_ - 1, selectedStageIndex_ + 1);
		}

		// Spaceキー：決定
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			GameAudio::GetInstance().PlayConfirm();

			stageManager_->SetCurrentStageIndex(selectedStageIndex_);

			fade_->Start(Fade::Status::FadeOut, fadeDuration_);
			phase_ = Phase::kFadeOut;
		}

		break;

	case Phase::kFadeOut:
		fade_->Update();

		if (fade_->IsFinished()) {
			isFinished_ = true;
		}
		break;
	}

	skydome_.Update();
}

void StageSelectScene::DrawStageSelector() {
	for (int32_t i = 0; i < selectableStageCount_; ++i) {
		const std::size_t index = static_cast<std::size_t>(i);

		Sprite* sprite = nullptr;

		if (i == selectedStageIndex_) {
			sprite = selectedStageSprites_[index];
		} else {
			sprite = stageSprites_[index];
		}

		if (sprite != nullptr) {
			sprite->Draw();
		}
	}
}

void StageSelectScene::Draw() {
	PrimitiveDrawer::GetInstance()->Reset();

	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kReadOnly);

	skydome_.Draw();

	Model::PostDraw();

	Sprite::PreDraw();

	// ステージ選択画像
	DrawStageSelector();

	// 操作UI
	if (spriteKeyA_ != nullptr) {
		spriteKeyA_->Draw();
	}

	if (spriteKeyD_ != nullptr) {
		spriteKeyD_->Draw();
	}

	if (spriteKeySpace_ != nullptr) {
		spriteKeySpace_->Draw();
	}

	Sprite::PostDraw();

	// UIも含めてフェード
	if (fade_ != nullptr) {
		fade_->Draw();
	}
}

StageSelectScene::~StageSelectScene() {
	for (Sprite*& sprite : stageSprites_) {
		delete sprite;
		sprite = nullptr;
	}

	for (Sprite*& sprite : selectedStageSprites_) {
		delete sprite;
		sprite = nullptr;
	}

	delete spriteKeyA_;
	delete spriteKeyD_;
	delete spriteKeySpace_;

	delete fade_;
	fade_ = nullptr;
}