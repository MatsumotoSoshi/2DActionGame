#include "TitleScene.h"
#include <numbers>
using namespace KamataEngine;

void TitleScene::Initialize() {
	// 3Dモデル生成
	modelFont_ = Model::CreateFromOBJ("titleFont");
	modelPlayer_ = Model::CreateFromOBJ("player");

	// ワールド変換の初期化
	// フォントの初期化
	worldTransformFont_.Initialize();
	worldTransformFont_.translation_ = {0.0f, 3.0f, 0.0f}; 

	// プレイヤーの初期化
	worldTransformPlayer_.Initialize();
	worldTransformPlayer_.translation_ = {-8.0f, -1.0f, -40.0f};

	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// カメラの初期化
	camera_.Initialize();

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeDuration_);
}

void TitleScene::Update() { 
	worldTransformPlayer_.translation_.x += 0.1f;

	if (worldTransformPlayer_.translation_.x > 10.0f) {
		worldTransformPlayer_.translation_.x = -10.0f;
	}

	worldMatrix_.Update(worldTransformFont_);
	worldMatrix_.Update(worldTransformPlayer_);

	worldTransformFont_.TransferMatrix();
	worldTransformPlayer_.TransferMatrix();

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

void TitleScene::Draw() { 
	// 3D描画前処理（GameSceneと同様）
	Model::PreDraw();

	// モデルを描画
	modelFont_->Draw(worldTransformFont_, camera_);
	modelPlayer_->Draw(worldTransformPlayer_, camera_);
	
	// 3D描画後処理
	Model::PostDraw();

	fade_->Draw();
}

TitleScene::~TitleScene() { 
	delete modelFont_; 
	delete modelPlayer_;
	delete fade_;
}