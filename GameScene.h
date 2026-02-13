#pragma once
#include "KamataEngine.h"
#include "Player.h"

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
	uint32_t textureHandle_ = 0;

	// カメラ
	KamataEngine::Camera camera_;
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	Player* player_ = nullptr;
};
