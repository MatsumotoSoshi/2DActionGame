#pragma once
#include "KamataEngine.h"
#include <vector>
#include "Skydome.h"
#include "Player.h"
#include "WorldMatrix.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "Enemy.h"
#include "Collision.h"
#include "DeathParticles.h"
#include "Fade.h"
#include "HitEffect.h"
#include "ShieldEnemy.h"

class GameScene {
public:

	/// <summary>
	/// 
	/// </summary>
	void Initialize();

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

	void GeneralBlocks();

	// すべての当たり判定を行う
	void CheckAllCollisions();

	// ゲームのフェーズ
	enum class Phase {
		kFadeIn,    // フェードイン
		kPlay,      // ゲームプレイ
		kDeath,     // デス演出
		kFadeOut,   // フェードアウト
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

	// エフェクトを生成
	void CreateHitEffect(const KamataEngine::WorldTransform& worldTransform);

private:
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Camera camera_;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	Skydome skydome_;
	Player player_;
	WorldMatrix worldMatrix_;
	MapChipField* mapChipField_;
	CameraController cameraController_;
	std::list<Enemy*> enemies_;
	DeathParticles* deathParticles_ = nullptr;
	Fade* fade_ = nullptr;
	std::list<HitEffect*> hitEffects_;
	std::list<ShieldEnemy*> shieldEnemies_;

	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;
	KamataEngine::Model* modelDeathParticles_ = nullptr;
	KamataEngine::Model* modelAttack_ = nullptr;
	KamataEngine::Model* modelHitEffect_ = nullptr;
	KamataEngine::Model* modelShieldEnemy_ = nullptr;
};
