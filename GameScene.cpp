#include "GameScene.h"
using namespace KamataEngine;

void GameScene::GenerateFieldObjects() {
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

			MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(j, i);

			switch (mapChipType) {
			case MapChipType::kBlock: {
				WorldTransform* worldTransform = new WorldTransform;
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				break;
			}
			case MapChipType::kPlayer: {
				assert(player_ == nullptr && "自キャラを二重に配置使用としています");
				// 自キャラを配置

				player_ = new Player();
				Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				
				player_->Initialize(modelPlayer_, modelAttack_, &cameraController_.GetCamera(), playerPosition);

				player_->SetMapChipField(mapChipField_);
				break;
			}
			case MapChipType::kEnemy: {
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				switch (subID) {
				case 0: {
					WalkEnemy* newEnemy = new WalkEnemy();
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
					newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);
					newEnemy->SetGameScene(this);
					enemies_.push_back(newEnemy);
					break;
				}
				case 1: {
					ShieldEnemy* newEnemy = new ShieldEnemy();
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
					newEnemy->Initialize(modelShieldEnemy_, &camera_, enemyPosition);
					newEnemy->SetGameScene(this);
					shieldEnemies_.push_back(newEnemy);
					break;
				}
				default:
					break;
				}
			}
			default:
				break;
			}
		}
	}
}

void GameScene::CreateHitEffect(const KamataEngine::WorldTransform& worldTransform) {
	HitEffect* newHitEffect = HitEffect::Create(worldTransform);
	hitEffects_.push_back(newHitEffect);
}

void GameScene::CreateGuardEffect(const KamataEngine::WorldTransform& worldTransform) {
	GuardEffect* newHitEffect = GuardEffect::Create(worldTransform);
	guardEffects_.push_back(newHitEffect);
}

void GameScene::Initialize() { 
	phase_ = Phase::kFadeIn;

	 camera_.farZ = 5000.0f;

	// カメラ初期化
	camera_.Initialize();

	 // 3Dモデル生成
    modelBlock_ = Model::CreateFromOBJ("block");
    modelSkydome_ = Model::CreateFromOBJ("SkyDome");
    modelPlayer_ = Model::CreateFromOBJ("Player");
	modelEnemy_ = Model::CreateFromOBJ("enemy");
	modelDeathParticles_ = Model::CreateFromOBJ("deathParticle");
	modelAttack_ = Model::CreateFromOBJ("hit_effect");
	modelHitEffect_ = Model::CreateFromOBJ("particle");
	modelShieldEnemy_ = Model::CreateFromOBJ("shieldEnemy");
	HitEffect::SetModel(modelHitEffect_);
	HitEffect::SetCamera(&camera_);
	modelGuardEffect_ = Model::CreateFromOBJ("ring");
	GuardEffect::SetModel(modelGuardEffect_);
	GuardEffect::SetCamera(&camera_);

    // デバッグカメラ生成
    debugCamera_ = new DebugCamera(1280, 720);

    mapChipField_ = new MapChipField;
    mapChipField_->LoadMapChipCsv("Resources/blocks.csv");


    // カメラコントローラ初期化
    cameraController_.Initialize();
	GenerateFieldObjects();

    cameraController_.SetTarget(player_);
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

	// フェードの初期化
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeDuration_);

}

void GameScene::CheckAllCollisions() {
	#pragma region 自キャラと敵キャラの当たり判定
	// 自キャラと敵キャラの当たり判定
	{
		// 判定対象1と2の座標
		AABB aabb1, aabb2;
	
		// 自キャラの座標
		aabb1 = player_->GetAABB();

		// 自キャラと敵弾すべての当たり判定
		for (WalkEnemy *enemy : enemies_) {
			if (enemy->IsCollisionDisabled()) {
				continue;   //	コリジョン無効の敵はスキップ
			}

			// 敵弾の座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
			if (Collision::IsCollision(aabb1, aabb2)) {
				// 自キャラの衝突時コールバックを呼び出す
				player_->onCollision(enemy);

				// 敵弾の衝突時コールバックを呼び出す
				enemy->onCollision(player_);
			}
		}
	}

	// 自キャラとシールド敵キャラの当たり判定
	{
		// 判定対象1と2の座標
		AABB aabb1, aabb2;

		// 自キャラの座標
		aabb1 = player_->GetAABB();

		// 自キャラと敵弾すべての当たり判定
		for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
			if (shieldEnemy->IsCollisionDisabled()) {
				continue; //	コリジョン無効の敵はスキップ
			}

			// 敵弾の座標
			aabb2 = shieldEnemy->GetAABB();

			// AABB同士の交差判定
			if (Collision::IsCollision(aabb1, aabb2)) {
				// 自キャラの衝突時コールバックを呼び出す
				player_->onCollision(shieldEnemy);

				// 敵弾の衝突時コールバックを呼び出す
				shieldEnemy->onCollision(player_);
			}
		}
	}
    #pragma endregion	
}

void GameScene::CameraUpdate() { 
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

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		// ゲームプレイフェーズの処理
		if (player_->isDead_) {
			// 死亡演出フェーズに切り替え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			Vector3 deathParticlePosition = player_->GetWorldPosition();

			// 自キャラ座標にデスパーティクルを発生、初期化
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelDeathParticles_, &camera_, deathParticlePosition);
		}
		break;
	case Phase::kDeath:
		// デス演出フェーズの処理
		
		break;
	}
}

void GameScene::Update() { 
	ChangePhase();

	// 天球の更新
	skydome_.Update();

	// 敵の更新(複数)
	for (WalkEnemy* enemy : enemies_) {
		enemy->Update();
	}

	// デスフラグの立った敵を削除
	enemies_.remove_if([](WalkEnemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true; 
		}
		return false;
	});

	hitEffects_.remove_if([](HitEffect* effect) {
		if (effect->IsDead()) {
			delete effect;
			return true;
		}
		return false;
	});

	// シールド敵の更新(複数)
	for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
		shieldEnemy->Update();
	}

	// デスフラグの立った敵を削除
	shieldEnemies_.remove_if([](ShieldEnemy* shieldEnemy) {
		if (shieldEnemy->IsDead()) {
			delete shieldEnemy;
			return true;
		}
		return false;
	});

	guardEffects_.remove_if([](GuardEffect* effect) {
		if (effect->IsDead()) {
			delete effect;
			return true;
		}
		return false;
	});

	// カメラコントローラーの更新
	cameraController_.Update();

	// カメラの更新
	CameraUpdate();

	// ブロックの更新
    for (auto& line : worldTransformBlocks_) {
        for (auto* block : line) {
            if (block) worldMatrix_.Update(*block);
        }
    }

	switch (phase_) {
	case Phase::kFadeIn:

		// 自キャラの更新
		player_->Update();

		// フェードインの処理
		fade_->Update();

		if (fade_->IsFinished()) {
			phase_ = Phase::kPlay;
		}

		break;

	case Phase::kPlay:
		// ゲームフェーズの処理
		
		// 自キャラの更新
		player_->Update();

		// ヒットエフェクトの更新
		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		// ガードエフェクトの更新
		for (GuardEffect* guardEffect : guardEffects_) {
			guardEffect->Update();
		}

		// すべての当たり判定
		CheckAllCollisions();

		break;

	case Phase::kDeath:
		// デス演出フェーズの処理
		
		// デスパーティクルの更新
		if (deathParticles_) {
			deathParticles_->Update();
		}

		if (deathParticles_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, fadeDuration_);
			phase_ = Phase::kFadeOut;
		}

		break;

		case Phase::kFadeOut:

		// 自キャラの更新
		player_->Update();

		// フェードアウトの処理
		fade_->Update();

		if (fade_->IsFinished()) {
			isFinished_ = true; 
		}
		break;
	}

	if (Input::GetInstance()->TriggerKey(DIK_R)) {
		reloadRequested_ = true;
	}

}

void GameScene::Draw() {
	// 3D描画前処理
	Model::PreDraw();

	switch (phase_) {
	case Phase::kFadeIn:
		// フェードインフェーズの描画処理
		// 天球の描画
		skydome_.Draw();

		// 自キャラの描画
		player_->Draw(cameraController_.GetCamera());

		// 敵の描画(複数)
		for (WalkEnemy* enemy : enemies_) {
			enemy->Draw(cameraController_.GetCamera());
		}

		// シールド敵の描画(複数)
		for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
			shieldEnemy->Draw(cameraController_.GetCamera());
		}

		// ブロックの描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;
				modelBlock_->Draw(*worldTransformBlock, cameraController_.GetCamera());
			}
		}
		break;

	case Phase::kPlay:
		// ゲームフェーズの描画処理

		// 天球の描画
		skydome_.Draw();

		// 自キャラの描画
		player_->Draw(cameraController_.GetCamera());

		// 敵の描画(複数)
		for (WalkEnemy* enemy : enemies_) {
			enemy->Draw(cameraController_.GetCamera());
		}

		// シールド敵の描画(複数)
		for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
			shieldEnemy->Draw(cameraController_.GetCamera());
		}

		// ヒットエフェクトの描画
		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Draw(cameraController_.GetCamera());
		}

		// ガードエフェクトの描画
		for (GuardEffect* guardEffect : guardEffects_) {
			guardEffect->Draw(cameraController_.GetCamera());
		}

		// ブロックの描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;
				modelBlock_->Draw(*worldTransformBlock, cameraController_.GetCamera());
			}
		}

		break;

	case Phase::kDeath:
		// デス演出フェーズの描画処理

		if (deathParticles_ && deathParticles_->IsFinished()) {
			isFinished_ = true;
		}

		// 天球の描画
		skydome_.Draw();

		// デスパーティクルの描画
		if (deathParticles_) {
			deathParticles_->Draw(cameraController_.GetCamera());
		}

		// 敵の描画(複数)
		for (WalkEnemy* enemy : enemies_) {
			enemy->Draw(cameraController_.GetCamera());
		}

		// シールド敵の描画(複数)
		for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
			shieldEnemy->Draw(cameraController_.GetCamera());
		}

		// ブロックの描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;
				modelBlock_->Draw(*worldTransformBlock, cameraController_.GetCamera());
			}
		}

		break;

	case Phase::kFadeOut:
		// フェードアウトフェーズの描画処理
		// 天球の描画
		skydome_.Draw();

		// 敵の描画(複数)
		for (WalkEnemy* enemy : enemies_) {
			enemy->Draw(cameraController_.GetCamera());
		}

		// シールド敵の描画(複数)
		for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
			shieldEnemy->Draw(cameraController_.GetCamera());
		}

		// ブロックの描画
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;
				modelBlock_->Draw(*worldTransformBlock, cameraController_.GetCamera());
			}
		}

		break;
	}

	// 3D描画後処理
	Model::PostDraw();

	// フェードの描画
	fade_->Draw();
}

GameScene::~GameScene() { 
	delete modelBlock_;
	delete debugCamera_;
	delete modelSkydome_;
	delete mapChipField_;
	delete deathParticles_;
	delete modelShieldEnemy_;

	for (WalkEnemy* enemy : enemies_) {
		delete enemy;
	}

	for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
		delete shieldEnemy;
	}

	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
}
