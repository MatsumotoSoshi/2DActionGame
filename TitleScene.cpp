#define NOMINMAX
#include "TitleScene.h"
#include "StageManager.h"
#include <algorithm>
#include <numbers>
#include <utility>
#include "GameAudio.h"
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

	worldTransformPlayer_.rotation_.x = -std::numbers::pi_v<float> / 2.0f;
	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransformPlayer_.rotation_.z = -std::numbers::pi_v<float>;

	// カメラの初期化
	camera_.Initialize();
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

	// 天球を表示できる描画距離
	camera_.farZ = 5000.0f;
	camera_.UpdateMatrix();

	modelSkydome_ = Model::CreateFromOBJ("SkyDome");

	skydome_.Initialize(modelSkydome_, &camera_);

	// 最初の描画前にも行列を作成
	skydome_.Update();

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeDuration_);

	// 下部中央にSPACEを表示
	delete spriteSpace_;
	spriteSpace_ = nullptr;

	spriteSpace_ = Sprite::Create(TextureManager::Load("ui/keys/keySPACE.png"), {560.0f, 600.0f});

	spriteSpace_->SetSize({160.0f, 64.0f});

	GameAudio::GetInstance().PlayBgm(GameAudio::Bgm::kTitle);
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
			GameAudio::GetInstance().PlayConfirm();

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

void TitleScene::Draw() {
	PrimitiveDrawer::GetInstance()->Reset();

	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kReadOnly);

	skydome_.Draw();

	Model::PostDraw();

	// 3Dモデル
	Model::PreDraw();

	modelFont_->Draw(worldTransformFont_, camera_);
	modelPlayer_->Draw(worldTransformPlayer_, camera_);

	Model::PostDraw();

	// 操作UI
	Sprite::PreDraw();

	if (spriteSpace_ != nullptr) {
		spriteSpace_->Draw();
	}

	Sprite::PostDraw();

	// UIも含めてフェード
	if (fade_ != nullptr) {
		fade_->Draw();
	}
}

TitleScene::~TitleScene() {
	delete spriteSpace_;
	delete modelFont_;
	delete modelPlayer_;
	delete fade_;
}
