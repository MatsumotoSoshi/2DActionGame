#pragma once
#include "KamataEngine.h"
#include <vector>
#include "Skydome.h"
#include "Player.h"
#include "WorldMatrix.h"

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

private:
	KamataEngine::Model* modelBlock_ = nullptr;
	 KamataEngine::Camera camera_;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	bool isDebugCameraActive_ = false;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	Skydome skydome_;
	Player player_;
	WorldMatrix worldMatrix_;

	KamataEngine::Model* modelSkydome_ = nullptr;
};
