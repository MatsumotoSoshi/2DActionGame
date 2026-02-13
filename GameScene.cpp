#include "GameScene.h"
using namespace KamataEngine;

void GameScene::Initialize() {
	textureHandle_ = TextureManager::Load("./Resources/uvChecker.png");

	// カメラ初期化
	camera_.Initialize();
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

	// デバッグカメラ
	debugCamera_ = new DebugCamera(1270.0f, 720.0f);

	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetCamera(&debugCamera_->GetCamera());

	// Player生成
	player_ = new Player();
	player_->Initialize(textureHandle_);
}

void GameScene::Update() {
	debugCamera_->Update();
	player_->Update();
}

void GameScene::Draw() {
	// 3D描画前処理
	Model::PreDraw();

	player_->Draw(debugCamera_->GetCamera());

	// 3D描画後処理
	Model::PostDraw();
}

GameScene::~GameScene() {
	delete player_;
	delete debugCamera_;
}
