#include "ClearScene.h"
#include "StageManager.h"
#include <numbers>
using namespace KamataEngine;

void ClearScene::Initialize(const std::vector<Vector3>& path, StageManager* stageManager) {
	// 3Dモデル生成
	modelFont_ = Model::CreateFromOBJ("titleFont");
	modelBlock_ = Model::CreateFromOBJ("block");
	modelEnemy_ = Model::CreateFromOBJ("enemy");
	modelShieldEnemy_ = Model::CreateFromOBJ("shieldEnemy");
	trajectory_.SetPoints(path);

	// クリア画面には、開いたゴールまで含めたステージ全体を再構成する。
	mapChipField_ = new MapChipField();
	const std::string stageFileName = "Resources/fields/" + stageManager->GetCurrentStageData().name + ".csv";
	mapChipField_->LoadMapChipCsv(stageFileName);
	for (uint32_t y = 0; y < mapChipField_->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < mapChipField_->GetNumBlockHorizontal(); ++x) {
			const MapChipType type = mapChipField_->GetMapChipTypeByIndex(x, y);
			if (type == MapChipType::kBlock) {
				WorldTransform* block = new WorldTransform();
				block->Initialize();
				block->translation_ = mapChipField_->GetMapChipPositionByIndex(x, y);
				worldMatrix_.Update(*block);
				stageBlocks_.push_back(block);
			} else if (type == MapChipType::kEnemy) {
				WorldTransform* target = new WorldTransform();
				target->Initialize();
				target->translation_ = mapChipField_->GetMapChipPositionByIndex(x, y);
				target->scale_ = {0.55f, 0.55f, 0.55f};
				worldMatrix_.Update(*target);
				if (mapChipField_->GetMapChipSubIDByIndex(x, y) == 1) {
					shieldTargets_.push_back(target);
				} else {
					normalTargets_.push_back(target);
				}
			} else if (type == MapChipType::kGoal) {
				goalTransform_.Initialize();
				goalTransform_.translation_ = mapChipField_->GetMapChipPositionByIndex(x, y);
				goalTransform_.scale_ = {0.55f, 0.55f, 0.55f};
				worldMatrix_.Update(goalTransform_);
			}
		}
	}
	goalColor_.Initialize();
	goalColor_.SetColor({0.25f, 0.9f, 1.0f, 1.0f});

	// ワールド変換の初期化
	// フォントの初期化
	worldTransformFont_.Initialize();
	worldTransformFont_.translation_ = {49.5f, 27.0f, 0.0f};

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 5000.0f;
	camera_.translation_ = {49.5f, 9.5f, -65.0f};
	camera_.UpdateMatrix();
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

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

	// ステージ全体を描画
	for (WorldTransform* block : stageBlocks_) {
		modelBlock_->Draw(*block, camera_);
	}

	for (WorldTransform* target : normalTargets_) {
		modelEnemy_->Draw(*target, camera_);
	}

	for (WorldTransform* target : shieldTargets_) {
		modelShieldEnemy_->Draw(*target, camera_);
	}

	modelBlock_->Draw(goalTransform_, camera_, &goalColor_);

	// タイトルを描画
	modelFont_->Draw(worldTransformFont_, camera_);

	// 3D描画後処理
	Model::PostDraw();
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNormal, Model::DepthTestMode::kReadOnly);
	trajectory_.Draw();
	Model::PostDraw();

	fade_->Draw();
}

ClearScene::~ClearScene() {
	delete modelFont_;
	delete modelBlock_;
	delete modelEnemy_;
	delete modelShieldEnemy_;
	delete mapChipField_;
	delete fade_;
	for (WorldTransform* block : stageBlocks_) {
		delete block;
	}

	for (WorldTransform* target : normalTargets_) {
		delete target;
	}

	for (WorldTransform* target : shieldTargets_) {
		delete target;
	}
}
