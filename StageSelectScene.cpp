
#define NOMINMAX

#include "StageSelectScene.h"
#include <algorithm>
#include <cassert>

using namespace KamataEngine;

void StageSelectScene::Initialize(StageManager* stageManager) {
	assert(stageManager != nullptr);
	stageManager_ = stageManager;

	const int32_t stageCount = stageManager_->GetStageCount();

	selectedStageIndex_ = stageManager_->GetCurrentStageIndex();

	if (stageCount > 0) {
		selectedStageIndex_ = std::clamp(selectedStageIndex_, 0, stageCount - 1);
	} else {
		selectedStageIndex_ = 0;
	}

	selectorFrameX_ = (selectedStageIndex_ - 2) * kCardSpacing;

	// モデル生成
	modelStage1_ = Model::CreateFromOBJ("block");
	modelStage2_ = Model::CreateFromOBJ("block");
	modelStage3_ = Model::CreateFromOBJ("block");
	modelStage4_ = Model::CreateFromOBJ("block");
	modelStage5_ = Model::CreateFromOBJ("block");

	// ワールド変換初期化
	worldTransformStage1_.Initialize();
	worldTransformStage2_.Initialize();
	worldTransformStage3_.Initialize();
	worldTransformStage4_.Initialize();
	worldTransformStage5_.Initialize();

	// モデル配置
	worldTransformStage1_.translation_ = {-20.0f, 0.0f, 0.0f};
	worldTransformStage2_.translation_ = {-10.0f, 0.0f, 0.0f};
	worldTransformStage3_.translation_ = {0.0f, 0.0f, 0.0f};
	worldTransformStage4_.translation_ = {10.0f, 0.0f, 0.0f};
	worldTransformStage5_.translation_ = {20.0f, 0.0f, 0.0f};

	worldMatrix1_.Update(worldTransformStage1_);
	worldMatrix2_.Update(worldTransformStage2_);
	worldMatrix3_.Update(worldTransformStage3_);
	worldMatrix4_.Update(worldTransformStage4_);
	worldMatrix5_.Update(worldTransformStage5_);


	camera_.Initialize();
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeDuration_);

	phase_ = Phase::kFadeIn;
	isFinished_ = false;
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
		if (Input::GetInstance()->TriggerKey(DIK_A)) {
			selectedStageIndex_ = std::max(0, selectedStageIndex_ - 1);
		}

		if (Input::GetInstance()->TriggerKey(DIK_D)) {
			selectedStageIndex_ = std::min(stageManager_->GetStageCount() - 1, selectedStageIndex_ + 1);
		}

		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
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

	worldMatrix1_.Update(worldTransformStage1_);
	worldMatrix2_.Update(worldTransformStage2_);
	worldMatrix3_.Update(worldTransformStage3_);
	worldMatrix4_.Update(worldTransformStage4_);
	worldMatrix5_.Update(worldTransformStage5_);

}

void StageSelectScene::DrawStageSelector() {
	// ステージモデル描画
	if (modelStage1_) {
		modelStage1_->Draw(worldTransformStage1_, camera_);
	}

	if (modelStage2_) {
		modelStage2_->Draw(worldTransformStage2_, camera_);
	}

	if (modelStage3_) {
		modelStage3_->Draw(worldTransformStage3_, camera_);
	}

	if (modelStage4_) {
		modelStage4_->Draw(worldTransformStage4_, camera_);
	}

	if (modelStage5_) {
		modelStage5_->Draw(worldTransformStage5_, camera_);
	}

	// 選択枠を滑らかに移動
	const float targetX = (selectedStageIndex_ - 2) * kCardSpacing;
	selectorFrameX_ += (targetX - selectorFrameX_) * 0.22f;

	// 選択枠の頂点
	constexpr float kFrameHalfWidth = 5.0f;
	constexpr float kFrameHalfHeight = 3.5f;
	constexpr float kFrameZ = -0.1f;

	const Vector4 selectorColor = {1.0f, 0.95f, 0.45f, 1.0f};

	const Vector3 corners[] = {
	    {selectorFrameX_ - kFrameHalfWidth, kCardCenterY - kFrameHalfHeight, kFrameZ},
	    {selectorFrameX_ + kFrameHalfWidth, kCardCenterY - kFrameHalfHeight, kFrameZ},
	    {selectorFrameX_ + kFrameHalfWidth, kCardCenterY + kFrameHalfHeight, kFrameZ},
	    {selectorFrameX_ - kFrameHalfWidth, kCardCenterY + kFrameHalfHeight, kFrameZ}
    };

	// corners と selectorColor を使った選択枠の線描画は、
	// 現在の PrimitiveDrawer のAPIに合わせて実装する。
}

void StageSelectScene::Draw() {
	PrimitiveDrawer::GetInstance()->Reset();

	Model::PreDraw();

	DrawStageSelector();

	Model::PostDraw();

	if (fade_ != nullptr) {
		fade_->Draw();
	}
}

StageSelectScene::~StageSelectScene() {
	delete modelStage1_;
	delete modelStage2_;
	delete modelStage3_;
	delete modelStage4_;
	delete modelStage5_;

	delete fade_;
}
