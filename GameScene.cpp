#include "GameScene.h"
using namespace KamataEngine;

void GameScene::GeneralBlocks() { 
	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数を設定（縦方向のブロック数）
	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
	    // １列の要素数を設定（横方向のブロック数）
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::Initialize() { 
	 camera_.farZ = 5000.0f;

	// カメラ初期化
	camera_.Initialize();

	 // 3Dモデル生成
    modelBlock_ = Model::CreateFromOBJ("block");
    modelSkydome_ = Model::CreateFromOBJ("SkyDome");
    modelPlayer_ = Model::CreateFromOBJ("Player");
	modelEnemy_ = Model::CreateFromOBJ("enemy");

    // デバッグカメラ生成
    debugCamera_ = new DebugCamera(1280, 720);

    mapChipField_ = new MapChipField;
    mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

    // プレイヤー初期位置
    Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 0);
    playerPosition.y = 1.0f;

    player_.Initialize(modelPlayer_, nullptr, playerPosition);

	player_.SetMapChipField(mapChipField_);

	// 敵
	Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(5, 0);
	enemyPosition.y = 1.0f;

	enemy_.Initialize(modelEnemy_, nullptr, enemyPosition);

    // カメラコントローラ初期化
    cameraController_.Initialize();
    cameraController_.SetTarget(&player_);
    cameraController_.Reset();

	// 移動範囲の指定
	CameraController::Rect area;

	area.left = 0.0f;
	area.right = 2000.0f; 
	area.bottom = 0.0f;
	area.top = 1000.0f;

	cameraController_.SetMovableArea(area);

    // PrimitiveDrawerにカメラを渡す
    PrimitiveDrawer::GetInstance()->SetCamera(
        &cameraController_.GetCamera());

    skydome_.Initialize(modelSkydome_,
        &cameraController_.GetCamera());

    GeneralBlocks();
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
	 enemy_.Update();
	 cameraController_.Update();

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
		 cameraController_.GetCamera().matView = debugCamera_->GetCamera().matView; 
		 // デバッグカメラのプロジェクション行列
		 cameraController_.GetCamera().matProjection = debugCamera_->GetCamera().matProjection;
		 // ビュープロジェクション行列の転送
		 cameraController_.GetCamera().TransferMatrix();
	 } else {
		 // ビュープロジェクション行列の更新と転送
		 cameraController_.GetCamera().UpdateMatrix();
	 }
	
}

void GameScene::Draw() {
	// 3D描画前処理
	Model::PreDraw();

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			modelBlock_->Draw(*worldTransformBlock, cameraController_.GetCamera());
		}
	}

	player_.Draw(cameraController_.GetCamera());
	skydome_.Draw();
	enemy_.Draw(cameraController_.GetCamera());

	// 3D描画後処理
	Model::PostDraw();
}

GameScene::~GameScene() { 
	delete modelBlock_;
	delete debugCamera_;
	delete modelSkydome_;
	delete mapChipField_;
	

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
}
