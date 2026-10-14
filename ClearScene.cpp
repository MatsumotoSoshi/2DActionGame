#include "ClearScene.h"
#include <numbers>
using namespace KamataEngine;

void ClearScene::Initialize() {
	// 3Dモデル生成
	modelFont_ = Model::CreateFromOBJ("titleFont");

	// ワールド変換の初期化
	// フォントの初期化
	worldTransformFont_.Initialize();
	worldTransformFont_.translation_ = {0.0f, 3.0f, 0.0f};

	// カメラの初期化
	camera_.Initialize();

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeDuration_);
}

void ClearScene::Update() {
	worldMatrix_.Update(worldTransformFont_);

	worldTransformFont_.TransferMatrix();

	// フェーズごとの処理
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;

	case Phase::kMain:
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
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
}

void ClearScene::Draw() {
	// 3D描画前処理（GameSceneと同様）
	Model::PreDraw();

	// モデルを描画
	modelFont_->Draw(worldTransformFont_, camera_);

	// 3D描画後処理
	Model::PostDraw();

	fade_->Draw();
}

ClearScene::~ClearScene() {
	delete modelFont_;
	delete fade_;
}