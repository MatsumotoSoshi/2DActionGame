#include "GameScene.h"
#include "StageManager.h"
#include <algorithm>
#include <cmath>
#include <string>
#ifdef USE_IMGUI
#include <imgui.h>
#endif
using namespace KamataEngine;

namespace {
constexpr float kTipCollisionRadius = 0.12f;

bool TipTouchesAABB(const Vector3& tip, const AABB& target) {
	return tip.x + kTipCollisionRadius >= target.min.x && tip.x - kTipCollisionRadius <= target.max.x &&
	       tip.y + kTipCollisionRadius >= target.min.y && tip.y - kTipCollisionRadius <= target.max.y &&
	       tip.z + kTipCollisionRadius >= target.min.z && tip.z - kTipCollisionRadius <= target.max.z;
}

}

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
			case MapChipType::kBlock:
			case MapChipType::kGoalWall: {
				WorldTransform* worldTransform = new WorldTransform;
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				if (mapChipType == MapChipType::kGoalWall) {
					goalGateBlocks_.push_back({j, i});
				}
				break;
			}
			case MapChipType::kPlayer: {
				assert(player_ == nullptr && "自キャラを二重に配置使用としています");
				// 自キャラを配置

				player_ = new Player();
				Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
				
				player_->Initialize(modelPlayer_, modelBubble_, &cameraController_.GetCamera(), playerPosition);

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
					newEnemy->SetMapChipField(mapChipField_);
					enemies_.push_back(newEnemy);
					break;
				}
				case 1: {
					ShieldEnemy* newEnemy = new ShieldEnemy();
					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(j, i);
					newEnemy->Initialize(modelShieldEnemy_, &camera_, enemyPosition);
					newEnemy->SetGameScene(this);
					newEnemy->SetMapChipField(mapChipField_);
					shieldEnemies_.push_back(newEnemy);
					break;
				}
				default:
					break;
				}
			}
				break;
			case MapChipType::kGoal: {
				hasGoal_ = true;
				goalWorldTransform_.Initialize();
				goalWorldTransform_.translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				goalWorldTransform_.scale_ = {0.55f, 0.55f, 0.55f};
				worldMatrix_.Update(goalWorldTransform_);
				break;
			}
			default:
				break;
			}
		}
	}
}

void GameScene::OpenGoalGate() {
	if (goalGateOpen_ || !hasGoal_ || !enemies_.empty() || !shieldEnemies_.empty()) {
		return;
	}

	for (const MapChipField::IndexSet& gate : goalGateBlocks_) {
		mapChipField_->SetMapChipTypeByIndex(gate.xIndex, gate.yIndex, MapChipType::kBlank);
		WorldTransform*& gateTransform = worldTransformBlocks_[gate.yIndex][gate.xIndex];
		delete gateTransform;
		gateTransform = nullptr;
	}

	goalGateOpen_ = true;
}

void GameScene::CreateHitEffect(const KamataEngine::WorldTransform& worldTransform) {
	HitEffect* newHitEffect = HitEffect::Create(worldTransform);
	hitEffects_.push_back(newHitEffect);
}

void GameScene::CreateGuardEffect(const KamataEngine::WorldTransform& worldTransform) {
	GuardEffect* newHitEffect = GuardEffect::Create(worldTransform);
	guardEffects_.push_back(newHitEffect);
}

void GameScene::Initialize(StageManager* stageDataManager_) { 
	 phase_ = Phase::kFadeIn;

	 camera_.farZ = 5000.0f;

	 // 引数をメンバ変数に記録する
	 stageManager_ = stageDataManager_;
	editorStageIndex_ = stageManager_->GetCurrentStageIndex();

	// カメラ初期化
	camera_.Initialize();
	goalObjectColor_.Initialize();
	goalObjectColor_.SetColor({0.25f, 0.9f, 1.0f, 1.0f});

	// 3Dモデル生成
    modelBlock_ = Model::CreateFromOBJ("block");
    modelSkydome_ = Model::CreateFromOBJ("SkyDome");
    modelPlayer_ = Model::CreateFromOBJ("Player");
	modelEnemy_ = Model::CreateFromOBJ("enemy");
	modelDeathParticles_ = Model::CreateFromOBJ("deathParticle");
	modelBubble_ = Model::CreateSphere(8, 8);
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
    
	// 現在のステージを変更
	const StageManager::StageData& stageData = stageManager_->GetCurrentStageData();
	// ステージファイルパスの生成
	std::string stageFileName = "Resources/fields/" + stageData.name + ".csv";
	// ステージファイルの読み込み
	mapChipField_->LoadMapChipCsv(stageFileName);

    // カメラコントローラ初期化
    cameraController_.Initialize();
	GenerateFieldObjects();
	for (WalkEnemy* enemy : enemies_) {
		enemy->SetTarget(player_);
	}

	for (ShieldEnemy* enemy : shieldEnemies_) {
		enemy->SetTarget(player_);
	}

	trajectory_.Initialize();
	if (player_ != nullptr) {
		trajectory_.RecordPoint(player_->GetWorldTransform().translation_);
	}

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
	if (player_ == nullptr || player_->IsDead()) {
		return;
	}

	const AABB playerBody = player_->GetAABB();
	const Vector3 playerTip = player_->GetTipPosition();

	for (WalkEnemy* enemy : enemies_) {
		if (enemy->IsCollisionDisabled()) {
			continue;
		}
		if (player_->IsAttack() && TipTouchesAABB(playerTip, enemy->GetAABB())) {
			player_->OnCollision(enemy);
			enemy->onCollision(player_);
		}
		if (!player_->IsDead() && enemy->IsDashing() && TipTouchesAABB(enemy->GetTipPosition(), playerBody)) {
			player_->OnEnemyTipCollision();
		}
	}

	for (ShieldEnemy* enemy : shieldEnemies_) {
		if (enemy->IsCollisionDisabled()) {
			continue;
		}
		if (!enemy->IsInvincible(player_->GetDashSpeed()) && player_->IsAttack() && TipTouchesAABB(playerTip, enemy->GetAABB())) {
			player_->OnCollision(enemy);
			enemy->onCollision(player_);
		}
		if (!player_->IsDead() && enemy->IsDashing() && TipTouchesAABB(enemy->GetTipPosition(), playerBody)) {
			player_->OnEnemyTipCollision();
		}
	}
}

void GameScene::CameraUpdate() { 
	#ifdef _DEBUG
	// デバッグカメラの切り替え
	if (Input::GetInstance()->TriggerKey(DIK_C)) {
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
	}
}

void GameScene::Update() { 
	if (isHitStoped_) {
		++hitStopTimer_;
		cameraController_.Update();
		CameraUpdate();
		if (hitStopTimer_ < kHitStopDuration) {
			return;
		}

		isHitStoped_ = false;
		hitStopTimer_ = 0;
		cameraController_.SetHitStopFocus({}, false);
	}

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

	if (phase_ == Phase::kPlay) {
		OpenGoalGate();
	}

	guardEffects_.remove_if([](GuardEffect* effect) {
		if (effect->IsDead()) {
			delete effect;
			return true;
		}
		return false;
	});

	// プレイヤーが突進している間だけ、弱いカメラシェイクを有効にする。
	cameraController_.SetDashShake(player_ != nullptr && player_->IsDashing());

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
		if (player_->ConsumeWallHit()) {
			++wallHitCount_;
			score_ = max(0, score_ - 100);
		}

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
		trajectory_.RecordPoint(player_->GetWorldTransform().translation_);

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

		if (hasGoal_ && goalGateOpen_) {
			const Vector3 playerPosition = player_->GetWorldPosition();
			const Vector3 goalPosition = goalWorldTransform_.translation_;
			// ゴールとプレイヤーの矩形が重なったら到達とする。
			constexpr float kGoalHalfWidth = 0.5f;
			constexpr float kGoalHalfHeight = 0.5f;
			if (std::abs(playerPosition.x - goalPosition.x) <= Player::kWidthHalf + kGoalHalfWidth &&
			    std::abs(playerPosition.y - goalPosition.y) <= Player::kHeightHalf + kGoalHalfHeight) {
				isCleared_ = true;
				trajectory_.RecordPoint(playerPosition);
				phase_ = Phase::kFadeOut;
				fade_->Start(Fade::Status::FadeOut, fadeDuration_);
			}
		}

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

	DrawFieldEditor();

}

void GameScene::DrawFieldEditor() {
#ifdef USE_IMGUI
	if (Input::GetInstance()->TriggerKey(DIK_F1)) {
		isFieldEditorOpen_ = !isFieldEditorOpen_;
	}
	if (!isFieldEditorOpen_ || mapChipField_ == nullptr || stageManager_ == nullptr) {
		return;
	}

	if (!ImGui::Begin("Field Editor (F1)", &isFieldEditorOpen_)) {
		ImGui::End();
		return;
	}

	const int32_t stageCount = stageManager_->GetStageCount();
	if (stageCount > 0) {
		const std::string& previewName = stageManager_->GetStageData(editorStageIndex_).name;
		if (ImGui::BeginCombo("Stage", previewName.c_str())) {
			for (int32_t i = 0; i < stageCount; ++i) {
				const std::string& name = stageManager_->GetStageData(i).name;
				const bool selected = editorStageIndex_ == i;
				if (ImGui::Selectable(name.c_str(), selected)) {
					editorStageIndex_ = i;
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Load selected stage")) {
			stageManager_->SetCurrentStageIndex(editorStageIndex_);
			reloadRequested_ = true;
			fieldEditorMessage_ = "Loading selected stage...";
		}
	}

	struct TileOption {
		const char* name;
		MapChipType type;
		uint8_t subID;
	};

	static constexpr TileOption tileOptions[] = {
	    {"Empty", MapChipType::kBlank, 0}, {"Block", MapChipType::kBlock, 0}, {"Goal wall", MapChipType::kGoalWall, 0},
	    {"Player", MapChipType::kPlayer, 0}, {"Target", MapChipType::kEnemy, 0}, {"Shield target", MapChipType::kEnemy, 1},
	    {"Goal", MapChipType::kGoal, 0},
	};

	constexpr int32_t kTileOptionCount = 7;
	if (ImGui::BeginCombo("Paint tile", tileOptions[selectedTileIndex_].name)) {
		for (int32_t i = 0; i < kTileOptionCount; ++i) {
			const bool selected = selectedTileIndex_ == i;
			if (ImGui::Selectable(tileOptions[i].name, selected)) {
				selectedTileIndex_ = i;
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Text("Click a cell to select it. Paint selection: row %d, column %d", selectedFieldY_, selectedFieldX_);
	constexpr float kCellSize = 6.0f;
	const ImVec2 canvasSize(kCellSize * mapChipField_->GetNumBlockHorizontal(), kCellSize * mapChipField_->GetNumBlockVirtical());
	ImGui::InvisibleButton("Field grid", canvasSize);
	const ImVec2 canvasMin = ImGui::GetItemRectMin();
	const bool canvasHovered = ImGui::IsItemHovered();
	if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		const ImVec2 mouse = ImGui::GetIO().MousePos;
		const int32_t x = static_cast<int32_t>((mouse.x - canvasMin.x) / kCellSize);
		const int32_t y = static_cast<int32_t>((mouse.y - canvasMin.y) / kCellSize);
		if (x >= 0 && x < static_cast<int32_t>(mapChipField_->GetNumBlockHorizontal()) &&
		    y >= 0 && y < static_cast<int32_t>(mapChipField_->GetNumBlockVirtical())) {
			selectedFieldX_ = x;
			selectedFieldY_ = y;
		}
	}
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	for (uint32_t y = 0; y < mapChipField_->GetNumBlockVirtical(); ++y) {
		for (uint32_t x = 0; x < mapChipField_->GetNumBlockHorizontal(); ++x) {
			const MapChipType type = mapChipField_->GetMapChipTypeByIndex(x, y);
			ImU32 color = IM_COL32(32, 38, 48, 255);
			switch (type) {
			case MapChipType::kBlock: color = IM_COL32(125, 135, 150, 255); break;
			case MapChipType::kGoalWall: color = IM_COL32(255, 145, 45, 255); break;
			case MapChipType::kPlayer: color = IM_COL32(65, 190, 255, 255); break;
			case MapChipType::kEnemy:
				color = mapChipField_->GetMapChipSubIDByIndex(x, y) == 1 ? IM_COL32(210, 105, 255, 255) : IM_COL32(255, 75, 85, 255);
				break;
			case MapChipType::kGoal: color = IM_COL32(80, 240, 160, 255); break;
			case MapChipType::kBlank: default: break;
			}
			const ImVec2 cellMin(canvasMin.x + x * kCellSize, canvasMin.y + y * kCellSize);
			const ImVec2 cellMax(cellMin.x + kCellSize, cellMin.y + kCellSize);
			drawList->AddRectFilled(cellMin, cellMax, color);
			if (x == static_cast<uint32_t>(selectedFieldX_) && y == static_cast<uint32_t>(selectedFieldY_)) {
				drawList->AddRect(cellMin, cellMax, IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f);
			} else {
				drawList->AddRect(cellMin, cellMax, IM_COL32(20, 24, 30, 255));
			}
		}
	}

	if (ImGui::Button("Paint selected cell")) {
		const TileOption& option = tileOptions[selectedTileIndex_];
		mapChipField_->SetMapChip(selectedFieldX_, selectedFieldY_, option.type, option.subID);
		fieldEditorMessage_ = "Edited in memory. Save and reload to apply to gameplay.";
	}
	ImGui::SameLine();
	if (ImGui::Button("Save and reload")) {
		uint32_t playerCount = 0;
		uint32_t goalCount = 0;
		for (uint32_t y = 0; y < mapChipField_->GetNumBlockVirtical(); ++y) {
			for (uint32_t x = 0; x < mapChipField_->GetNumBlockHorizontal(); ++x) {
				const MapChipType type = mapChipField_->GetMapChipTypeByIndex(x, y);
				playerCount += type == MapChipType::kPlayer ? 1u : 0u;
				goalCount += type == MapChipType::kGoal ? 1u : 0u;
			}
		}
		if (playerCount != 1 || goalCount != 1) {
			fieldEditorMessage_ = "Field needs exactly one player and one goal before saving.";
			ImGui::TextWrapped("%s", fieldEditorMessage_);
			ImGui::End();
			return;
		}
		const std::string path = "Resources/fields/" + stageManager_->GetCurrentStageData().name + ".csv";
		// ゴール解放中に一時的に消した壁は、保存時には元へ戻して残す。
		if (goalGateOpen_) {
			for (const MapChipField::IndexSet& gate : goalGateBlocks_) {
				mapChipField_->SetMapChip(gate.xIndex, gate.yIndex, MapChipType::kGoalWall);
			}
		}
		const bool saved = mapChipField_->SaveMapChipCsv(path);
		if (goalGateOpen_) {
			for (const MapChipField::IndexSet& gate : goalGateBlocks_) {
				mapChipField_->SetMapChip(gate.xIndex, gate.yIndex, MapChipType::kBlank);
			}
		}
		if (saved) {
			reloadRequested_ = true;
			fieldEditorMessage_ = "Saved. Reloading field...";
		} else {
			fieldEditorMessage_ = "Save failed: could not open field CSV.";
		}
	}
	ImGui::TextWrapped("%s", fieldEditorMessage_);
	ImGui::End();
#endif
}

void GameScene::StartHitStop(const Vector3& focusPosition) {
	isHitStoped_ = true;
	hitStopTimer_ = 0;
	cameraController_.SetHitStopFocus(focusPosition, true);
}

void GameScene::Draw() {
	// 前フレームの予測線を破棄してから、今フレーム分を描画する。
	PrimitiveDrawer::GetInstance()->Reset();
	// 不透明なモデルはブレンドせず、深度を書き込みながら描画する
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNone, Model::DepthTestMode::kOn);

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
	}

	if (hasGoal_) {
		modelBlock_->Draw(goalWorldTransform_, cameraController_.GetCamera(), &goalObjectColor_);
	}

	// 不透明モデルの描画を終了し、半透明エフェクト用のパスに切り替える
	Model::PostDraw();
	if (phase_ == Phase::kFadeIn || phase_ == Phase::kPlay) {
		player_->DrawTrajectory();
	}
	Model::PreDraw(Model::CullingMode::kBack, Model::BlendMode::kNormal, Model::DepthTestMode::kReadOnly);

	if (phase_ == Phase::kPlay) {
		player_->DrawAfterimages(cameraController_.GetCamera());
		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Draw(cameraController_.GetCamera());
		}
		for (GuardEffect* guardEffect : guardEffects_) {
			guardEffect->Draw(cameraController_.GetCamera());
		}
	} else if (phase_ == Phase::kDeath && deathParticles_) {
		deathParticles_->Draw(cameraController_.GetCamera());
	}

	Model::PostDraw();

	// フェードの描画
	fade_->Draw();
}

GameScene::~GameScene() { 
	delete modelBlock_;
	delete modelBubble_;
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
