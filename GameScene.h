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

	// すべての当たり判定を行う
	void CheckAllCollisions();

	// ゲームのフェーズ
	enum class Phase {
		kFadeIn,    // フェードイン
		kPlay,      // ゲームプレイ
		kDeath,     // デス演出
		kFadeOut,   // フェードアウト
		kTitle,     // タイトルシーン
		kClear,     // クリアシーン
	};

	// ゲームの現在フェーズ
	Phase phase_;

	// カメラ更新関数
	void CameraUpdate();

	void ChangePhase();

	// 終了フラグ
	bool isFinished_ = false;

	// デスフラグのゲッター
	bool IsFinished() const { return isFinished_; }

	// フェード時間
	float fadeDuration_ = 2.0f;

	// ヒットエフェクトを生成
	void CreateHitEffect(const KamataEngine::WorldTransform& worldTransform);

	// ヒットストップを開始
	void StartHitStop();

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
	static inline const uint32_t kHitStopDuration = 15;


private:
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Camera camera_;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

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


	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;
	KamataEngine::Model* modelDeathParticles_ = nullptr;
	KamataEngine::Model* modelAttack_ = nullptr;
	KamataEngine::Model* modelHitEffect_ = nullptr;
	KamataEngine::Model* modelShieldEnemy_ = nullptr;
	KamataEngine::Model* modelGuardEffect_ = nullptr;
};
