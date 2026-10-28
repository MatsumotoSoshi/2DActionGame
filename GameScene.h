#pragma once
#include "KamataEngine.h"
#include <vector>
#include "Skydome.h"
#include "Player.h"
#include "WorldMatrix.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "WalkEnemy.h"
#include "Collision.h"
#include "DeathParticles.h"
#include "Fade.h"
#include "HitEffect.h"
#include "ShieldEnemy.h"
#include "GuardEffect.h"
#include "Trajectory.h"

class StageManager;

class GameScene {
public:

	/// <summary>
	/// 
	/// </summary>
	void Initialize(StageManager* stageDataManager);

	/// <summary>
	/// 
	/// </summary>
	void Update();

	/// <summary>
	/// 
	/// </summary>
	void Draw();

	/// <summary>
	/// 
	/// </summary>
	~GameScene();

	void GenerateFieldObjects();
	void OpenGoalGate();
	void RegisterEnemyDefeated();
	const Trajectory& GetTrajectory() const { return trajectory_; }

	// すべての当たり判定を行う
	void CheckAllCollisions();

	// ゲームのフェーズ
	enum class Phase {
		kFadeIn,   // フェードイン
		kTutorial, // チュートリアル
		kPlay,     // ゲームプレイ
		kDeath,    // デス演出
		kFadeOut,  // フェードアウト
		kTitle,    // タイトルシーン
		kClear,    // クリアシーン
	};

	// ゲームの現在フェーズ
	Phase phase_;

	// カメラ更新関数
	void CameraUpdate();

	void ChangePhase();

	// 終了フラグ
	bool isFinished_ = false;
	bool isCleared_ = false;

	// デスフラグのゲッター
	bool IsFinished() const { return isFinished_; }
	bool IsCleared() const { return isCleared_; }

	// フェード時間
	float fadeDuration_ = 2.0f;

	// ヒットエフェクトを生成
	void CreateHitEffect(const KamataEngine::WorldTransform& worldTransform);

	// ヒットストップを開始
	void StartHitStop(const KamataEngine::Vector3& focusPosition);

	// ガードエフェクトを生成
	void CreateGuardEffect(const KamataEngine::WorldTransform& worldTransform);

	// リロード要求フラグ
	bool reloadRequested_ = false;

	// リロード要求フラグのgetter
	bool IsReloadRequested() const { return reloadRequested_; }

	
	// =====ヒットストップ=====
	// ヒットストップの経過時間カウンター
	uint32_t hitStopTimer_ = 0;
	bool isHitStoped_ = false;
	// ヒットストップ時間
	static inline const uint32_t kHitStopDuration = 5;

	uint32_t GetWallHitCount() const { return wallHitCount_; }

	float GetTotalMoveDistance() const { return totalMoveDistance_; }

	void RecordClearStats();

	float totalMoveDistance_ = 0.0f;
	KamataEngine::Vector3 previousClearPosition_{};

	// 通常ブロックとゴール前の壁を描画
	void DrawMapBlocks();

	
	void DrawDashSpace();

private:
	void DrawFieldEditor();
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Camera camera_;

	// ゴール前の壁専用モデル
	KamataEngine::Model* modelGoalWall_ = nullptr;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;
	KamataEngine::WorldTransform goalWorldTransform_;
	KamataEngine::ObjectColor goalObjectColor_;
	bool hasGoal_ = false;
	bool goalGateOpen_ = false;
	std::vector<MapChipField::IndexSet> goalGateBlocks_;

	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	Skydome skydome_;
	Player* player_ = nullptr;
	WorldMatrix worldMatrix_;
	MapChipField* mapChipField_;
	CameraController cameraController_;
	std::list<WalkEnemy*> enemies_;
	DeathParticles* deathParticles_ = nullptr;
	Fade* fade_ = nullptr;
	std::list<HitEffect*> hitEffects_;
	std::list<ShieldEnemy*> shieldEnemies_;
	std::list<GuardEffect*> guardEffects_;
	StageManager* stageDataManager_ = nullptr; 
	// ステージマネージャ参照用のポインタ
	StageManager* stageManager_ = nullptr;
	Trajectory trajectory_;
	int score_ = 0;
	uint32_t wallHitCount_ = 0;
	bool isFieldEditorOpen_ = false;
	int32_t editorStageIndex_ = 0;
	int32_t selectedFieldX_ = 0;
	int32_t selectedFieldY_ = 0;
	int32_t selectedTileIndex_ = 0;
	const char* fieldEditorMessage_ = "F1: open field editor";

	// プレイヤー頭上のSPACE表示
	KamataEngine::Sprite* spriteDashSpace_ = nullptr;


	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;
	KamataEngine::Model* modelDeathParticles_ = nullptr;
	KamataEngine::Model* modelBubble_ = nullptr;
	KamataEngine::Model* modelHitEffect_ = nullptr;
	KamataEngine::Model* modelShieldEnemy_ = nullptr;
	KamataEngine::Model* modelGuardEffect_ = nullptr;
};
