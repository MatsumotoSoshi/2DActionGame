#include "ClearScene.h"
#include "StageManager.h"
#include <numbers>
#include "GameAudio.h"
using namespace KamataEngine;

void ClearScene::Initialize(const std::vector<Vector3>& path, StageManager* stageManager, uint32_t wallHits, float totalDistance) {

	phase_ = Phase::kFadeIn;
	isFinished_ = false;

	// 壁への激突と移動距離の減点を少し増やす
	constexpr double kBaseScore = 10000.0;
	constexpr double kWallHitPenalty = 400.0;
	constexpr double kDistancePenalty = 20.0;

	const double distance = totalDistance > 0.0f ? static_cast<double>(totalDistance) : 0.0;

	const double result = kBaseScore - static_cast<double>(wallHits) * kWallHitPenalty - distance * kDistancePenalty;

	// 内部スコアは最低0点
	clearScore_ = result > 0.0 ? static_cast<int>(result) : 0;

	// 評価
	if (clearScore_ >= 8000) {
		clearRank_ = ClearRank::A;
	} else if (clearScore_ >= 5000) {
		clearRank_ = ClearRank::B;
	} else {
		clearRank_ = ClearRank::C;
	}

	uint32_t rankTexture = 0;

	switch (clearRank_) {
	case ClearRank::A:
		rankTexture = TextureManager::Load("ui/ranks/rankA.png");
		break;

	case ClearRank::B:
		rankTexture = TextureManager::Load("ui/ranks/rankB.png");
		break;

	case ClearRank::C:
		rankTexture = TextureManager::Load("ui/ranks/rankC.png");
		break;
	}

	// 評価
	spriteRank_ = Sprite::Create(rankTexture, {1100.0f, 230.0f});

	spriteRank_->SetSize({160.0f, 160.0f});

	// 評価コメント
	uint32_t commentTexture = 0;

	switch (clearRank_) {
	case ClearRank::A:
		commentTexture = TextureManager::Load("ui/clear/rankAComment.png");
		break;

	case ClearRank::B:
		commentTexture = TextureManager::Load("ui/clear/rankBComment.png");
		break;

	case ClearRank::C:
		commentTexture = TextureManager::Load("ui/clear/rankCComment.png");
		break;
	}

	// ステージ表示の下、選択項目の上に配置
	spriteRankComment_ = Sprite::Create(commentTexture, {450.0f, 245.0f});

	spriteRankComment_->SetSize({720.0f, 128.0f});

	// 左：タイトルへ（通常）
	spriteReturnTitle_ = Sprite::Create(TextureManager::Load("ui/clear/returnTitle.png"), {240.0f, 600.0f});

	// 左：タイトルへ（選択中）
	spriteReturnTitleSelected_ = Sprite::Create(TextureManager::Load("ui/clear/returnTitleSelected.png"), {240.0f, 600.0f});

	// 右：ステージ選択へ（通常）
	spriteReturnStageSelect_ = Sprite::Create(TextureManager::Load("ui/clear/returnStageSelect.png"), {720.0f, 600.0f});

	// 右：ステージ選択へ（選択中）
	spriteReturnStageSelectSelected_ = Sprite::Create(TextureManager::Load("ui/clear/returnStageSelectSelected.png"), {720.0f, 600.0f});

	const Vector2 menuSize = {320.0f, 320.0f * 112.0f / 480.0f};

	spriteReturnTitle_->SetSize(menuSize);
	spriteReturnTitleSelected_->SetSize(menuSize);
	spriteReturnStageSelect_->SetSize(menuSize);
	spriteReturnStageSelectSelected_->SetSize(menuSize);

	// 初期選択は左のタイトル
	selectedMenuIndex_ = 0;
	returnTarget_ = ReturnTarget::kTitle;
	isSelected_ = false;

	// 3Dモデル生成
	modelBlock_ = Model::CreateFromOBJ("block");
	modelEnemy_ = Model::CreateFromOBJ("target");
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

	// ワールド変換の初期化
	// フォントの初期化
	worldTransformFont_.Initialize();
	worldTransformFont_.translation_ = {0.0f, 10.0f, 0.0f};


	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 5000.0f;
	camera_.translation_ = {49.5f, 9.5f, -65.0f};
	camera_.UpdateMatrix();
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

	// 天球を表示できる描画距離
	modelSkydome_ = Model::CreateFromOBJ("SkyDome");

	skydome_.Initialize(modelSkydome_, &camera_);

	// 最初の描画前にも行列を作成
	skydome_.Update();

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeDuration_);

	GameAudio::GetInstance().PlayBgm(GameAudio::Bgm::kClear);
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
		// Aキーで左の項目を選択
		if (Input::GetInstance()->TriggerKey(DIK_A)) {
			selectedMenuIndex_ = 0;
		}

		// Dキーで右の項目を選択
		if (Input::GetInstance()->TriggerKey(DIK_D)) {
			selectedMenuIndex_ = 1;
		}

		// Spaceキーで決定
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			GameAudio::GetInstance().PlayConfirm();

			if (selectedMenuIndex_ == 0) {
				returnTarget_ = ReturnTarget::kTitle;
				isSelected_ = true;
			} else {
				returnTarget_ = ReturnTarget::kStageSelect;
				isSelected_ = false;
			}

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

void ClearScene::Draw() {
	PrimitiveDrawer::GetInstance()->Reset();

	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kReadOnly);

	skydome_.Draw();

	Model::PostDraw();

	// ステージの3D描画
	Model::PreDraw();

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

	Model::PostDraw();

	// 移動軌跡
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNormal, Model::DepthTestMode::kReadOnly);

	trajectory_.Draw();

	Model::PostDraw();

	// PNG描画
	Sprite::PreDraw();

	DrawClearScore();

	if (spriteRankComment_ != nullptr) {
		spriteRankComment_->Draw();
	}

	// 左の項目
	Sprite* titleSprite = selectedMenuIndex_ == 0 ? spriteReturnTitleSelected_ : spriteReturnTitle_;

	if (titleSprite != nullptr) {
		titleSprite->Draw();
	}

	// 右の項目
	Sprite* stageSelectSprite = selectedMenuIndex_ == 1 ? spriteReturnStageSelectSelected_ : spriteReturnStageSelect_;

	if (stageSelectSprite != nullptr) {
		stageSelectSprite->Draw();
	}

	Sprite::PostDraw();

	// UIを含めてフェード
	if (fade_ != nullptr) {
		fade_->Draw();
	}
}

ClearScene::~ClearScene() {
	// Sprite
	delete spriteRank_;
	delete spriteRankComment_;

	delete spriteReturnTitle_;
	delete spriteReturnTitleSelected_;

	delete spriteReturnStageSelect_;
	delete spriteReturnStageSelectSelected_;

	// Model
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

void ClearScene::DrawClearScore() {
	if (spriteRank_ != nullptr) {
		spriteRank_->Draw();
	}
}
