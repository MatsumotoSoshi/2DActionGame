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
	for (int32_t i = 0; i < 1; ++i) {
		Enemy* newEnemy = new Enemy();

		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(5 + i, 0);
		enemyPosition.y = 1.0f;

		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);
		enemies_.push_back(newEnemy);
	}

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

void GameScene::CheckAllCollisions() {
	#pragma region 自キャラと敵キャラの当たり判定
	// 自キャラと敵キャラの当たり判定
	{
		// 判定対象1と2の座標
		AABB aabb1, aabb2;
	
		// 自キャラの座標
		aabb1 = player_.GetAABB();

		// 自キャラと敵弾すべての当たり判定
		for (Enemy *enemy : enemies_) {
			// 敵弾の座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
			if (Collision::IsCollision(aabb1, aabb2)) {
				// 自キャラの衝突時コールバックを呼び出す
				player_.onCollision(enemy);

				// 敵弾の衝突時コールバックを呼び出す
				enemy->onCollision(&player_);
			}
		}
	}
    #pragma endregion	
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
	 for (Enemy* enemy : enemies_) {
		 enemy->Update();
	 }
	 cameraController_.Update();

	 // すべての当たり判定を行う
	 CheckAllCollisions();

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
	for (Enemy* enemy : enemies_) {
		enemy->Draw(cameraController_.GetCamera());
	}

	// 3D描画後処理
	Model::PostDraw();
}

GameScene::~GameScene() { 
	delete modelBlock_;
	delete debugCamera_;
	delete modelSkydome_;
	delete mapChipField_;
	
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
}
