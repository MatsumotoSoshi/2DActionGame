#define NOMINMAX
#include "TitleScene.h"
#include "StageManager.h"
#include <algorithm>
#include <numbers>
#include <utility>
using namespace KamataEngine;


void TitleScene::Initialize() {

	// 3Dモデル生成
	modelFont_ = Model::CreateFromOBJ("titleFont");
	modelPlayer_ = Model::CreateFromOBJ("player");

	// ワールド変換の初期化
	// フォントの初期化
	worldTransformFont_.Initialize();
	worldTransformFont_.translation_ = {0.0f, 10.0f, 0.0f};

	// プレイヤーの初期化
	worldTransformPlayer_.Initialize();
	worldTransformPlayer_.translation_ = {-24.0f, 0.0f, -35.0f};

	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// カメラの初期化
	camera_.Initialize();
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);


	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeDuration_);
}



void TitleScene::Update() {
	worldTransformPlayer_.translation_.x += 0.12f;

	if (worldTransformPlayer_.translation_.x > 24.0f) {
		worldTransformPlayer_.translation_.x = -24.0f;
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
    PrimitiveDrawer::GetInstance()->Reset();

    Model::PreDraw();

    modelFont_->Draw(worldTransformFont_, camera_);
    modelPlayer_->Draw(worldTransformPlayer_, camera_);

    Model::PostDraw();

    fade_->Draw();
}

TitleScene::~TitleScene() { 
	delete modelFont_; 
	delete modelPlayer_;
	delete fade_;
}
