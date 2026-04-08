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

	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelPlayer_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;
};
