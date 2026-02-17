#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"

class Player {
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
	void Draw(const KamataEngine::Camera& camera);

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	WorldMatrix worldMatrix_;
};
