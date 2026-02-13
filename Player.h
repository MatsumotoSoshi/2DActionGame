#pragma once
#include "KamataEngine.h"

class Player {
public:
	/// <summary>
	/// 
	/// </summary>
	void Initialize(uint32_t textureHandle);

	/// <summary>
	/// 
	/// </summary>
	void Update();

	/// <summary>
	/// 
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	uint32_t textureHandle_ = 0;
};
