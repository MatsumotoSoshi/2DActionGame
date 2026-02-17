#include "GameScene.h"
using namespace KamataEngine;

void GameScene::Initialize() { 
	 camera_.farZ = 5000.0f;

	// カメラ初期化
	camera_.Initialize();
	PrimitiveDrawer::GetInstance()->SetCamera(&camera_);

	// 3Dモデル生成
	modelBlock_ = Model::CreateFromOBJ("block");
	modelSkydome_ = Model::CreateFromOBJ("SkyDome");

	skydome_.Initialize(modelSkydome_, &camera_);
	player_.Initialize();

	// デバッグカメラ生成
	debugCamera_ = new DebugCamera(1280, 720);

	// 要素数
	const uint32_t kNumBlockVirtical = 10;
	const uint32_t kNumBlockHorizontal = 20;

	// ブロックサイズ
	const float kBlockWidth = 1.0f;
	const float kBlockHeight = 1.0f;

	 worldTransformBlocks_.resize(kNumBlockVirtical);
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	}

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {

			if ((i + j) % 2 == 0) {
				worldTransformBlocks_[i][j] = nullptr;
				continue;
			}

			worldTransformBlocks_[i][j] = new WorldTransform();
			worldTransformBlocks_[i][j]->Initialize();
			worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
			worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * i;
		}
	}

}

void GameScene::Update() { 
	
     for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			 if (!worldTransformBlock)
				 continue;
			 worldMatrix_.Update(*worldTransformBlock);
		}
	 }

	 skydome_.Update();
	 player_.Update();

	 #ifdef _DEBUG
	 // デバッグカメラの切り替え
	 if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		 isDebugCameraActive_ = true;
	 }
	 #endif

	 if (isDebugCameraActive_) {
		 // デバッグカメラ更新
		 debugCamera_->Update();
		 // デバッグカメラのビュー行列
		 camera_.matView = debugCamera_->GetCamera().matView; 
		 // デバッグカメラのプロジェクション行列
		 camera_.matProjection = debugCamera_->GetCamera().matProjection;
		 // ビュープロジェクション行列の転送
		 camera_.TransferMatrix();
	 } else {
		 // ビュープロジェクション行列の更新と転送
		 camera_.UpdateMatrix();
	 }
	
}

void GameScene::Draw() {
	// 3D描画前処理
	Model::PreDraw();

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	player_.Draw(camera_);
	skydome_.Draw();

	// 3D描画後処理
	Model::PostDraw();
}

GameScene::~GameScene() { 
	delete modelBlock_;
	delete debugCamera_;
	delete modelSkydome_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
}
