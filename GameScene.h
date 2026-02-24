#pragma once
#include "KamataEngine.h"
#include <vector>
#include "Skydome.h"
#include "Player.h"
#include "WorldMatrix.h"
#include "MapChipField.h"
#include "CameraController.h"

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


	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelPlayer_ = nullptr;
};
